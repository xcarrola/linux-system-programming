/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* is_shell_cl.c

   Send a shell command to server and display results sent back by server.

   Usage: is_shell_cl server-host server-port shell-command

   See also is_shell_sv.c.
*/
#define _GNU_SOURCE
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
#define BUF_SIZE 100
    if (argc != 4 || strcmp(argv[1], "--help") == 0)
        usageErr("%s host port shell-command\n", argv[0]);

    char *host = argv[1];
    char *port = argv[2];
    char *shellCmd = argv[3];

    /* Call getaddrinfo() to obtain a list of addresses that
       we can try connecting to */

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC;                /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    struct addrinfo *result;
    if (getaddrinfo(host, port, &hints, &result) != 0)
        errExit("getaddrinfo");

    /* Walk through returned list until we find an address structure
       that can be used to successfully connect a socket */

    struct addrinfo *rp;
    int cfd;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue;                           /* On error, try next address */

        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                              /* Success */

        /* Connect failed: close this socket and try next address */

        close(cfd);
    }

    freeaddrinfo(result);

    if (rp == NULL)
        fatal("Could not connect socket to any address");

    /* FIXME: Write the shell command to the server; read and display
       the server's response. Write strlen()+1 bytes to ensure that
       the terminating null byte is also transmitted. */

    exit(EXIT_SUCCESS);
}
