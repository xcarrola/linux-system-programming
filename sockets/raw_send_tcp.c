/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* raw_send_tcp.c

   Send a TCP segment over an IPv4 raw socket.

   To test this program, (1) use two other programs that establish a TCP/IP
   connection in order to exchange text (a pair of netcat instances, for
   example); (2) use wireshark(1) to observe the sequence numbers and
   acknowledgement numbers of packets that are being exchanged; (3) use this
   program to send a segment to one of the programs already running,
   specifying suitable flags, sequence numbers, and acknowledgement numbers,
   something like the following:

        sudo ./raw_send_tcp -w 512 -p -a 3102459363 localhost localhost \
                        40704 50000 959585323 xxx
*/

#define _GNU_SOURCE
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <assert.h>
#include "tlpi_hdr.h"

#define IPv4_HEADER_LEN 20      /* Length of IPv4 header (assumes
                                   no options are present) */
#define TCP_HEADER_LEN  20      /* Length of TCP header */

struct cmdLineOpts {
    char    *dstHost;
    char    *srcHost;
    char    *dstPort;
    char    *srcPort;
    char    *data;
    char    *seqnumStr;
    uint32_t ackSeqnum;
    uint16_t winsize;
    bool     pshFlag;
    bool     finFlag;
    bool     rstFlag;
    bool     synFlag;
};

/* Compute the Internet Checksum, as per RFC 1071 */

static uint16_t
checksum(uint16_t *addr, int count)
{
    uint32_t sum = 0;

    /* Sum up 2-byte values until none or only one byte left */

    while (count > 1) {
        sum += *addr++;
        count -= 2;
    }

    /* Add left-over byte, if any */

    if (count > 0)
        sum += *(uint8_t *) addr;

    /* Fold 32-bit sum to 16 bits */

    while (sum >> 16)
        sum = (sum & 0xffff) + (sum >> 16);

    /* Checksum is ones' complement of sum */

    return (uint16_t) ~sum;
}

/* Calculate the IPv4 TCP checksum. The checksum is calculated based on a
   "pseudoheader" as well as the content of the TCP segment itself. As
   described in RFC 793, the pseudoheader consists mainly of fields from
   the IP header:

          0      7 8     15 16    23 24    31
         +--------+--------+--------+--------+
         |          source address           |
         +--------+--------+--------+--------+
         |        destination address        |
         +--------+--------+--------+--------+
         |  zero  |protocol|   TCP length    |
         +--------+--------+--------+--------+
*/

static uint16_t
tcpChecksum(struct ip *iphdr, struct tcphdr *tcphdr, char *data)
{
    char buf[IP_MAXPACKET];
    struct pseudoheader {
        struct in_addr  srcAddress;
        struct in_addr  dstAddress;
        uint8_t         zero;
        uint8_t         protocol;
        uint16_t        tcpLength;
    };
    const int PSEUDOHEADER_SIZE = 12;

    /* Sanity check the size of our pseudoheader structure */

    assert(sizeof(struct pseudoheader) == PSEUDOHEADER_SIZE);

    struct pseudoheader *phdr = (struct pseudoheader *) buf;
    struct tcphdr *tcpSegment =
            (struct tcphdr *) (buf + sizeof(struct pseudoheader));

    /* Populate the pseudoheader */

    phdr->srcAddress = iphdr->ip_src;
    phdr->dstAddress = iphdr->ip_dst;
    phdr->zero = 0;
    phdr->protocol = iphdr->ip_p;
    phdr->tcpLength = htons(TCP_HEADER_LEN + strlen(data));

    /* Copy TCP segment into buffer */

    memcpy(tcpSegment, tcphdr, ntohs(phdr->tcpLength));

    /* The checksum calculation includes the checksum field itself. During
       that calculation, the checksum field must be zero. */

    tcpSegment->check = 0;

    int checksumLen = sizeof(struct pseudoheader) + ntohs(phdr->tcpLength);

    /* Zero fill to 16-bit multiple, as required by RFC 793 */

    if (checksumLen % 2) {
        buf[checksumLen] = 0;
        checksumLen++;
    }

    printf("Buffer length for checksum = %d\n", checksumLen);

    return checksum((uint16_t *) buf, checksumLen);
}

/* Return the IPv4 address for a given host */

static struct sockaddr_in
getIPv4Addr(char *host)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    struct addrinfo *res;
    int s = getaddrinfo(host, NULL, &hints, &res);
    if (s != 0)
        fatal("getaddrinfo() failed: %s", gai_strerror(s));

    struct sockaddr_in addr = *(struct sockaddr_in *) res->ai_addr;
    freeaddrinfo(res);

    return addr;
}

/* Populate the fields of the IP header pointed to by 'iphdr' */

static void
buildIpHeader(struct ip *iphdr, size_t ipDagramLen,
              char *srcHost, struct sockaddr_in dstIpAddr)
{
    /* 'ip_hl' is a 4-bit value that counts 32-bit words in header */

    iphdr->ip_hl = IPv4_HEADER_LEN / sizeof(uint32_t);

    iphdr->ip_v = 4;            /* IPv4 */
    iphdr->ip_tos = 0;          /* Type of service/DSCP+ECN */

    iphdr->ip_id = htons(0);    /* Identification; unused since
                                   there is no fragmentation */

    /* ip_off: three 1-bit flags + 13-bit fragment offset */

    int mbz = 0;                /* Must be zero */
    int dontFrag = 0;           /* Don't fragment */
    int moreFrags = 0;          /* More fragments */
    int fragOffset = 0;
    iphdr->ip_off = htons((mbz << 15) + (dontFrag << 14) +
                          (moreFrags << 13) + fragOffset);

    iphdr->ip_ttl = 255;        /* 8-bit time-to-live */
    iphdr->ip_p = IPPROTO_TCP;  /* Transport-layer protocol (TCP) */

    iphdr->ip_src = (getIPv4Addr(srcHost)).sin_addr;
    iphdr->ip_dst = dstIpAddr.sin_addr;

    /* Note: the kernel automatically fills in the 'ip_len' (total
       length) and 'ip_sum' (checksum) fields for us (see raw(7)) */

}

/* Populate the header fields and data of the TCP segment at the
   location pointed to by 'tcphdr' */

static void
buildTcpSegment(struct tcphdr *tcphdr, struct ip *iphdr,
                 struct cmdLineOpts *clopt)
{
    /* Place data immediately after TCP header */

    uint8_t *tcpData = (uint8_t *) tcphdr + TCP_HEADER_LEN;
    uint16_t tcpDataLen = strlen(clopt->data);
    memcpy(tcpData, clopt->data, tcpDataLen);

    /* Fill in the header fields */

    tcphdr->source = htons(atoi(clopt->srcPort));       /* Source port */
    tcphdr->dest = htons(atoi(clopt->dstPort));         /* Destination port */

    tcphdr->seq = htonl(atoi(clopt->seqnumStr));

    if (clopt->ackSeqnum == 0) {
        tcphdr->ack = 0;
        tcphdr->ack_seq = htonl(0);
    } else {
        tcphdr->ack = 1;
        tcphdr->ack_seq = htonl(clopt->ackSeqnum);
    }

    tcphdr->doff = 5;           /* 5 * 4 byte words */
    tcphdr->res1 = 0;
    tcphdr->res2 = 0;           /* ECE + CWR */
    tcphdr->urg = 0;
    tcphdr->psh = clopt->pshFlag;
    tcphdr->rst = clopt->rstFlag;
    tcphdr->syn = clopt->synFlag;
    tcphdr->fin = clopt->finFlag;

    tcphdr->window = htons(clopt->winsize);
    tcphdr->urg_ptr = htons(0);

    /* Calculate TCP checksum */

    tcphdr->check = tcpChecksum(iphdr, tcphdr, clopt->data);
}

static void
usageError(char *pname)
{
    fprintf(stderr, "Usage: %s [options] <src-host> <dst-host> "
            "<src-port> <dst-port> <seq-num> <data>\n", pname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "    -a <num>   Acknowledgement number\n");
    fprintf(stderr, "    -f         FIN flag\n");
    fprintf(stderr, "    -p         PSH flag\n");
    fprintf(stderr, "    -r         RST flag\n");
    fprintf(stderr, "    -s         SYN flag\n");
    fprintf(stderr, "    -w <size>  Window size\n");
    exit(EXIT_FAILURE);
}

static void
parseCmdLineOpts(int argc, char *argv[], struct cmdLineOpts *clopt)
{
    int opt;

    clopt->pshFlag = false;
    clopt->finFlag = false;
    clopt->rstFlag = false;
    clopt->synFlag = false;
    clopt->winsize = 32768;
    clopt->ackSeqnum = 0;

    while ((opt = getopt(argc, argv, "a:fprsw:")) != -1) {
        switch (opt) {
        case 'a': clopt->ackSeqnum = strtoul(optarg, NULL, 0);  break;
        case 'f': clopt->finFlag = true;                        break;
        case 'p': clopt->pshFlag = true;                        break;
        case 'r': clopt->rstFlag = true;                        break;
        case 's': clopt->synFlag = true;                        break;
        case 'w': clopt->winsize = strtoul(optarg, NULL, 0);    break;
        default:  usageError(argv[0]);
        }
    }
    if (argc != optind + 6)
        usageError(argv[0]);

    clopt->srcHost =    argv[optind];
    clopt->dstHost =    argv[optind + 1];
    clopt->srcPort =    argv[optind + 2];
    clopt->dstPort =    argv[optind + 3];
    clopt->seqnumStr =  argv[optind + 4];
    clopt->data =       argv[optind + 5];
}

int
main(int argc, char *argv[])
{
    char dstIpStr[INET_ADDRSTRLEN];
    struct cmdLineOpts clopt;

    parseCmdLineOpts(argc, argv, &clopt);

    void *ipDatagram = malloc(IP_MAXPACKET);
    if (ipDatagram == NULL)
        errExit("malloc-ipDatagram");

    size_t ipDagramLen = IPv4_HEADER_LEN + TCP_HEADER_LEN + strlen(clopt.data);

    /* The IP header sits at the start of the datagram */

    struct ip *iphdr = ipDatagram;

    /* The TCP segment, starting with the header, sits
       immediately after the IP header */

    struct tcphdr *tcphdr = (struct tcphdr *)
            ((char *) ipDatagram + IPv4_HEADER_LEN);

    struct sockaddr_in dstIpAddr = getIPv4Addr(clopt.dstHost);

    if (inet_ntop(AF_INET, &dstIpAddr.sin_addr, dstIpStr,
                INET_ADDRSTRLEN) == NULL)
        errExit("inet_ntop");

    printf("Destination IP: %*s\n", INET_ADDRSTRLEN, dstIpStr);

    /********* IPv4 header **********/

    buildIpHeader(iphdr, ipDagramLen, clopt.srcHost, dstIpAddr);

    /********* TCP segment **********/

    buildTcpSegment(tcphdr, iphdr, &clopt);

    /* Create the raw socket that we'll use to send the IP datagram */

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd == -1)
        errExit("socket");

    /* Send the IP datagram */

    /* The datagram already contains the destination IP address.
       However, the kernel must prepare the data-link layer header
       (probably an Ethernet header), and needs to know where to send
       the raw datagram. The kernel determines this information from
       the IP address supplied to the sendto() call. */

    printf("Sending datagram (%zd bytes)\n", ipDagramLen);

    if (sendto(sockfd, ipDatagram, ipDagramLen, 0,
                (struct sockaddr *) &dstIpAddr,
                sizeof(struct sockaddr)) == -1)
        errExit("sendto");

    close(sockfd);
    free(ipDatagram);
    exit(EXIT_SUCCESS);
}
