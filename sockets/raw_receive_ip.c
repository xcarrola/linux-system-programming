/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* raw_receive_ip.c

   Receive an IP datagram on a raw socket.
*/
#define _GNU_SOURCE
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "tlpi_hdr.h"

#define IPv4_HEADER_LEN 20      /* Length of IPv4 header (assumes
                                   no options are present) */
#define UDP_HEADER_LEN  8       /* Length of UDP header */

int
main(int argc, char *argv[])
{
    if (argc != 2)
        usageErr("%s protoStr\n", argv[0]);

    char *protoStr = argv[1];

    void *ipDatagram = malloc(IP_MAXPACKET);
    if (ipDatagram == NULL)
        errExit("malloc-ipDatagram");

    struct ip *iphdr = ipDatagram;

    /* Receive datagrams and display on stdout */

    int protocol = atoi(protoStr);
    int sockfd = socket(AF_INET, SOCK_RAW, protocol);
    if (sockfd == -1)
        errExit("socket");

    for (;;) {
        struct sockaddr_storage claddr;
        socklen_t len = sizeof(struct sockaddr_storage);
        ssize_t numRead = recvfrom(sockfd, ipDatagram, IP_MAXPACKET, 0,
                                   (struct sockaddr *) &claddr, &len);
        if (numRead == -1)
            errExit("recvfrom");

        char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
        if (getnameinfo((struct sockaddr *) &claddr,
                    len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
                    NI_NUMERICHOST | NI_NUMERICSERV) == 0)
        printf("Receive datagram; address reported by recvfrom(): "
                "host = %s, port = %s\n", hbuf, sbuf);
        printf("    %zd bytes\n", numRead);

        char addrStr[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &iphdr->ip_src, addrStr,
                    INET_ADDRSTRLEN) == NULL)
            errExit("inet_ntop");
        printf("    source = %s; ", addrStr);

        if (inet_ntop(AF_INET, &iphdr->ip_dst, addrStr,
                    INET_ADDRSTRLEN) == NULL)
            errExit("inet_ntop");
        printf("destination = %s\n", addrStr);
        printf("    protocol = %d; ", iphdr->ip_hl);
        printf("header length = %d\n", iphdr->ip_hl * 4);
        printf("        data = %.*s\n",
                (int) numRead - iphdr->ip_hl * 4,
                (char *) ipDatagram + iphdr->ip_hl * 4);
        printf("\n");
    }

    exit(EXIT_SUCCESS);
}
