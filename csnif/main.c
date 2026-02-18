
#include <headers.h>

void err(const char *msg);

int createRawSocket(int protocol) {
    int sock = socket(PF_PACKET, SOCK_RAW, htons(protocol));
    if (sock == -1) err("socket init failed");
    return sock;
}

int bindRawSocketToInterface(const char *dev, int sock, int protocol) {
    struct sockaddr_ll sll;
    struct ifreq ifr;

    /** clear buffers for sll && interface reqs */
    memset(&sll, 0 , sizeof(sll));
    memset(&ifr, 0, sizeof(ifr));

    /** set the name for which we want to retrieve
     * data packets from ... (eth0, en0 etc)
     */
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) == -1)
        err("error getting interface index");
    
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(protocol);

    int sBind = bind(sock, (struct sockaddr *) &sll, sizeof(sll));
    if (sBind == -1) err("failed to bind socket");
    
    return (1);
}

void PrintPacketHex(unsigned char * packet, int len) {
        unsigned char *p = packet;
        while(len--){
            printf("%.2x ", *p);
            p++;
        }
}

int main(int argc, char **argv) {
    int sock, n, pack_to_sniff;
    unsigned char buf[2048];
    struct sockaddr_ll packet_info;
    int packet_info_sz = sizeof(packet_info);

    sock = createRawSocket(ETH_P_IP);
    if (sock == -1) err("failed to init socket");

    int bind = bindRawSocketToInterface(argv[1], sock, ETH_P_IP);
    if (bind == -1) 
        err("socket binding failed \n");
    pack_to_sniff = argv[2];
    while (pack_to_sniff--) {
        n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&packet_info, &packet_info_sz);
        if (n == -1) err("data packets fetch failed \n");
        
        PrintPacketHex(buf, n);
    }
}


void err(const char *msg) {
    perror(msg);
    exit(-1);
}