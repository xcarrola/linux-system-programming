/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* us_seqnum_single_sv.c

   A simple UNIX stream socket server. Our service is to provide
   unique sequence numbers to clients.

   Usage: us_seqnum_single_sv server-path

   See also us_seqnum_single_cl.c.
*/
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>
#include "ignore_signal.h"
#include "tlpi_hdr.h"

#define BACKLOG 5

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s server-path\n", argv[0]);

    int seqNum = 0;
    char *serverPath = argv[1];

    int lfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (lfd == -1)
        errExit("socket");

    /* Construct server socket address, bind socket to it,
       and make this a listening socket */

    if (remove(serverPath) == -1 && errno != ENOENT)
        errExit("remove-%s", serverPath);

    struct sockaddr_un addr;
    if (strlen(serverPath) > sizeof(addr.sun_path) - 1)
        fatal("socket pathname is too long");

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, serverPath, sizeof(addr.sun_path) - 1);

    if (bind(lfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    if (listen(lfd, BACKLOG) == -1)
        errExit("listen");

    /* Ignore SIGPIPE signal, so we find out about
       broken connections via a failure from write() */

    if (ignoreSignal(SIGPIPE))
        errExit("ignoreSignal");

    for (;;) {     /* Handle clients iteratively */

        /* Accept a client connection */

        int cfd = accept(lfd, NULL, NULL);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        /* Send sequence number back */

        if (write(cfd, &seqNum, sizeof(int)) != sizeof(int))
            fprintf(stderr, "Error on write\n");

        seqNum++;               /* Update sequence number */

        if (close(cfd) == -1)   /* Close connection */
            errMsg("close");
    }
}
