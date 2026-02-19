#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <linux/if_ether.h>


#define MAX_EVENTS      1024
#define MAX_BUFFER      4096
#define MAX_CONNSZ      65536


typedef struct connection 
{
    int fd;                             /* file descriptor to connect to */
    size_t write_offset;                /* for handling partial writes ops */
    size_t write_length;                /* total size of packet data written */
    char buffer[MAX_BUFFER];            /* buffer size & container */
    struct connection *next;            /* pointer to the next connection */
} connection_t;

static volatile int running = 1;
static connection_t connections[MAX_CONNSZ];
static connection_t *free_list = NULL;

void pool_init() 
{
    for (int x = 0; x < MAX_CONNSZ; x++)
        connections[x].next = &connections[ x + 1];
    connections[MAX_CONNSZ - 1].next = NULL;
    free_list = &connections[0];
}

connection_t *pool_alloc()
{
    if (!free_list)
        return NULL;

    connection_t *conn = free_list;
    free_list = free_list->next;
    memset(conn, 0, sizeof(connection_t));
    return conn;
}

void pool_free(connection_t *conn)
{
    conn->next = free_list;
    free_list = conn;
}

int set_nonblocking(int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(int argc, char **argv) {}