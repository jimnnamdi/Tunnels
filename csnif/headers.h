
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>


#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>

#define MAX_BUFSIZ 65536
