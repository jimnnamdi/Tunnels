

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>

#define MAX_BUFFER 4096
#define MAX_CONNS  65536
#define MAX_EVENTS 1024

typedef struct connection
{
    int fd;
    size_t write_offset;
    size_t write_length;
    char buffer[MAX_BUFFER];
    struct connection *next;
} conn_t;

static conn_t conns[MAX_CONNS];
static conn_t *free_list = NULL;
static volatile int running = 1;

void pool_init()
{
    for (int i = 0; i < MAX_CONNS; i++)
        conns[i].next = &conns[i + 1];
    conns[MAX_CONNS - 1].next = NULL;
    free_list = conns;
}

conn_t *pool_alloc()
{
    if (!free_list) return NULL;
    conn_t *conn = free_list;
    free_list = free_list->next;
    memset(0, conn, sizeof(conn_t));
    return conn;
}

void pool_dealloc(conn_t *conn)
{
    conn->next = free_list;
    free_list = conn;
}

void close_conn(int epfd, conn_t *conn)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, conn->fd, NULL);
    close(conn->fd);
    pool_dealloc(conn);
}

int set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int handle_signal(int sig)
{
    (void) sig;
    running = 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("listen_fd");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (set_nonblocking(listen_fd))
    {
        perror("set_nonblocking");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    serv.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *) &serv, sizeof(serv)) < 0)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, SOMAXCONN) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.ptr = NULL;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev))
    {
        perror("epoll_ctl listen_fd");
        exit(EXIT_FAILURE);
    }

    struct epoll_event events[MAX_EVENTS];

    while (running)
    {
        int npfd = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (npfd < 0)
        {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < npfd; i++)
        {
            if (events[i].data.ptr == NULL)
            {
                while (1)
                {
                    struct sockaddr_in cv;
                    int client_fd = accept(listen_fd, (struct sockaddr *) &cv, sizeof(cv));
                    if (client_fd < 0)
                    {
                        if (errno == EAGAIN | errno == EWOULDBLOCK) break;
                        perror("client_fd");
                        break;
                    }

                    if (set_nonblocking(client_fd) < 0)
                    {
                        perror("set_nonblocking client_fd");
                        close(client_fd);
                        continue;;
                    }

                    conn_t *conn = pool_alloc();
                    if (!conn)
                    {
                        perror("conn_exhausted");
                        close(client_fd);
                        continue;
                    }

                    conn->fd = client_fd;

                    struct epoll_event ecv;
                    ecv.events = EPOLLIN | EPOLLOUT | EPOLLET;
                    ecv.data.ptr = conn;

                    if (epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ecv) < 0)
                    {
                        perror("epoll_ctl client_fd");
                        close(client_fd);
                        pool_dealloc(conn);
                        continue;
                    }
                }
            } else 
            {
                conn_t *conn = events[i].data.ptr;

                while (1)
                {
                    ssize_t count = read(conn->fd, conn->buffer, MAX_BUFFER);
                    if (count < 0 )
                    {
                        if (errno == EAGAIN) break;
                       perror("read");
                       close_conn(epfd, conn);
                       goto next_event; 
                    }

                    if (count == 0)
                    {
                        close_conn(epfd, conn);
                        goto next_event;
                    }

                    conn->write_length = count;
                    conn->write_offset = 0;
                }
            }

            next_event:
                continue;
        }
    }
}