/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* t_getaddrinfo.c

   Call getaddrinfo() with arguments specified from the command line, and
   display the contents of the returned linked list of results.
*/
#define _GNU_SOURCE         /* To get NI_MAXHOST and NI_MAXSERV */
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include "tlpi_hdr.h"

static void
usageError(const char *progName, const char *msg)
{
    if (msg != NULL)
        printf("%s", msg);
    fprintf(stderr, "Usage: %s [options] [-n host] [-s service]\n",
            progName);
    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "\t-f {4|6}   Specify address family "
            "(AF_INET, AF_INET6)\n");
    fprintf(stderr, "\t-o flags   Specify 1 or more flags:\n");
    fprintf(stderr, "\t\t\ta - AI_ADDRCONFIG\n");
    fprintf(stderr, "\t\t\tA - AI_ALL\n");
    fprintf(stderr, "\t\t\tp - AI_PASSIVE\n");
    fprintf(stderr, "\t\t\th - AI_NUMERICHOST\n");
    fprintf(stderr, "\t\t\tc - AI_CANONNAME\n");
    fprintf(stderr, "\t\t\ts - AI_NUMERICSERV\n");
    fprintf(stderr, "\t\t\t4 - AI_V4MAPPED\n");
    fprintf(stderr, "\t-p proto   Protocol (#, or tcp/udp/sctp/dccp)\n");
    fprintf(stderr, "\t-t type    Socket type\n"
                    "             (#, or s|stream / d|dgram / r|raw / rdm / sp|seqpacket)\n");

    exit(EXIT_FAILURE);
}

/* Display a getaddrinfo() return value in human-readable form */

static void
displayGetaddrinfoStatus(int status)
{
    switch (status) {
    case 0:
        printf("getaddrinfo() succeeded\n");
        break;
    case EAI_FAMILY:
        printf("EAI_FAMILY: Address family not supported\n");
        break;
    case EAI_SOCKTYPE:
        printf("EAI_SOCKTYPE: Socket type not supported\n");
        break;
    case EAI_BADFLAGS:
        printf("AIF_BADFLAGS: Invalid flags\n");
        break;
    case EAI_NONAME:
        printf("EAI_NONAME: host/service unknown\n");
        break;
    case EAI_SERVICE:
        printf("EAI_SERVICE: Service unavailable for socket type\n");
        break;
    default:
        printf("Error code: %d %s\n", status, gai_strerror(status));
        break;
    }

}

static char *
familyToString(int family)
{
    return (family == AF_INET) ?  "AF_INET" :
           (family == AF_INET6) ? "AF_INET6" : "???";
}

static char *
protocolToString(int protocol)
{
    switch (protocol) {
    case IPPROTO_TCP:   return "IPPROTO_TCP";
    case IPPROTO_UDP:   return "IPPROTO_UDP";
#ifdef IPPROTO_DCCP
    case IPPROTO_DCCP:  return "IPPROTO_DCCP";
#endif
#ifdef IPPROTO_SCTP
    case IPPROTO_SCTP:  return "IPPROTO_SCTP";
#endif
    default:            return "???";
    }
}

static char *
socketTypeToString(int socktype)
{
    switch (socktype) {
    case SOCK_STREAM:           return "SOCK_STREAM";
    case SOCK_DGRAM:            return "SOCK_DGRAM";
    case SOCK_RAW:              return "SOCK_RAW";
    case SOCK_RDM:              return "SOCK_RDM";
    case SOCK_SEQPACKET:        return "SOCK_SEQPACKET";
    default:                    return "???";
    }
}

/* Display the contents of the linked list returned by getaddrinfo() */

static void
displayGetaddrinfoResults(struct addrinfo *res, bool showCanonname)
{
    struct addrinfo *ai = res;

    for (int j = 0; ai != NULL; ai = ai->ai_next, j++) {
        printf("%d:\n", j);

        printf("\tai_family = %s (%d)\n", familyToString(ai->ai_family),
                ai->ai_family);

        printf("\tai_socktype = %s (%d)\n",
                socketTypeToString(ai->ai_socktype),
                ai->ai_socktype);

        printf("\tai_protocol = %d (%s)\n", ai->ai_protocol,
                protocolToString(ai->ai_protocol));

        if (showCanonname)
            printf("\tai_canonname = %s\n",
                    (ai->ai_canonname != NULL) ?  ai->ai_canonname : "<NULL>");

        /* Display info from socket address pointed to by 'ai_addr' field */

        printf("\tai_addr:\n");
        printf("\t\tsa_family = %d (%s)\n", ai->ai_addr->sa_family,
                familyToString(ai->ai_addr->sa_family));

        /* Display socket address as presentation string plus port number */

        char buf[1024];
        if (ai->ai_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *) ai->ai_addr;

            if (inet_ntop(ai->ai_addr->sa_family, &(sa->sin_addr),
                        buf, sizeof(buf)) == NULL)
                errMsg("inet_ntop");
            else
                printf("\t\thost = %s, port = %u\n", buf, ntohs(sa->sin_port));

        } else if (ai->ai_family == AF_INET6) {
            struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *) ai->ai_addr;

            printf("\t\tflowinfo = %d\n", sa6->sin6_flowinfo);
            printf("\t\tscope_id = %d\n", sa6->sin6_scope_id);
            if (inet_ntop(ai->ai_addr->sa_family, &(sa6->sin6_addr),
                        buf, sizeof(buf)) == NULL)
                errMsg("inet_ntop");
            else
                printf("\t\thost = %s, port = %d\n", buf,
                        ntohs(sa6->sin6_port));
        }

        /* Display socket address as host and service name */

        char host[NI_MAXHOST], service[NI_MAXSERV];
        if (getnameinfo(ai->ai_addr, ai->ai_addrlen,
                    host, sizeof(host), service, sizeof(service), 0) != 0)
            printf("\t\tgetnameinfo failed\n");
        else
            printf("\t\tgetnameinfo: host = %s, service = %s\n",
                    host, service);
    }
}

/* Parse command-line options, returning information via 'host', 'service'
   and 'hintsp' */

static void
parseCommandLine(int argc, char *argv[], char **host, char **service,
                struct addrinfo **hintsp)
{
    static struct addrinfo hints;
    *hintsp = NULL;
    *host = NULL;
    *service = NULL;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;

    int opt;
    while ((opt = getopt(argc, argv, "n:s:f:o:p:ht:")) != -1) {
        switch (opt) {
        case 'n':       /* Host */
            *host = strlen(optarg) > 0 ? optarg : NULL;
            break;

        case 's':       /* Service */
            *service = strlen(optarg) > 0 ? optarg : NULL;
            break;

        case 'f':       /* Socket family (domain) */
            *hintsp = &hints;
            hints.ai_family = (optarg[0] == '6') ? AF_INET6 : AF_INET;
            break;

        case 'o':       /* Flags for 'hints' argument */
            *hintsp = &hints;
            if (strchr(optarg, 'a') != NULL)
                hints.ai_flags |= AI_ADDRCONFIG;
            if (strchr(optarg, 'A') != NULL)
                hints.ai_flags |= AI_ALL;
            if (strchr(optarg, '4') != NULL)
                hints.ai_flags |= AI_V4MAPPED;
            if (strchr(optarg, 'p') != NULL)
                hints.ai_flags |= AI_PASSIVE;
            if (strchr(optarg, 'c') != NULL)
                hints.ai_flags |= AI_CANONNAME;
            if (strchr(optarg, 'h') != NULL)
                hints.ai_flags |= AI_NUMERICHOST;
            if (strchr(optarg, 's') != NULL)
                hints.ai_flags |= AI_NUMERICSERV;
            break;

        case 't':       /* Socket type */
            *hintsp = &hints;

            if (isdigit(optarg[0])) {
                hints.ai_socktype = atoi(optarg);
            } else {
                if (strcmp(optarg, "s") == 0 || strcmp(optarg, "stream") == 0)
                    hints.ai_socktype = SOCK_STREAM;
                else if (strcmp(optarg, "d") == 0 ||
                         strcmp(optarg, "dgram") == 0)
                    hints.ai_socktype = SOCK_DGRAM;
                else if (strcmp(optarg, "r") == 0 ||
                         strcmp(optarg, "raw") == 0)
                    hints.ai_socktype = SOCK_RAW;
                else if (strcmp(optarg, "sp") == 0 ||
                         strcmp(optarg, "seqpacket") == 0)
                    hints.ai_socktype = SOCK_SEQPACKET;
                else if (strcmp(optarg, "rdm") == 0)
                    hints.ai_socktype = SOCK_RDM;
                else
                    usageError(argv[0], "Bad socket type\n");
            }
            break;

        case'p':        /* Protocol */
            *hintsp = &hints;
            if (isdigit(optarg[0])) {
                hints.ai_protocol = atoi(optarg);
            } else {
                if (strcmp(optarg, "tcp") == 0)
                    hints.ai_protocol = IPPROTO_TCP;
                else if (strcmp(optarg, "udp") == 0)
                    hints.ai_protocol = IPPROTO_UDP;
#ifdef IPPROTO_SCTP
                else if (strcmp(optarg, "sctp") == 0)
                    hints.ai_protocol = IPPROTO_SCTP;
#endif
#ifdef IPPROTO_DCCP
                else if (strcmp(optarg, "dccp") == 0)
                    hints.ai_protocol = IPPROTO_DCCP;
#endif
                else
                    usageError(argv[0], "Bad protocol name\n");
            }
            break;

        case 'h':
        default:
            usageError(argv[0], NULL);
        }
    }

}

int
main(int argc, char *argv[])
{
    char *host = NULL;
    char *service = NULL;
    struct addrinfo *hintsp = NULL;

    parseCommandLine(argc, argv, &host, &service, &hintsp);

    struct addrinfo *res;
    int s = getaddrinfo(host, service, hintsp, &res);

    displayGetaddrinfoStatus(s);

    if (s != 0) {
        if (host == NULL && service == NULL) {
            printf("('host' and 'service' were both NULL)\n\n");
            usageError(argv[0], NULL);
        }
        exit(EXIT_FAILURE);
    }

    displayGetaddrinfoResults(res,
            (hintsp != NULL) && (hintsp->ai_flags & AI_CANONNAME));

    exit(EXIT_SUCCESS);
}
