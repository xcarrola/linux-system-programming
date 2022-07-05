/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* t_select_simple.c

   Example of the use of the select() system call to monitor multiple
   file descriptors for readability.

   Usage: ./t_select_simple timeout fd...

        'timeout' is a timeout in seconds, or '-' for infinite timeout
        (NULL timeout argument for select()).

   This is a simplified version of t_select.c.
*/
#include <sys/select.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s timeout fd...\n"
                 "\t'timeout' can be '-' for infinite timeout", argv[0]);

    /* Timeout for select() is specified in argv[1] */

    struct timeval timeout;
    struct timeval *pto;

    if (strcmp(argv[1], "-") == 0) {
        pto = NULL;                     /* Infinite timeout */
    } else {
        pto = &timeout;
        timeout.tv_sec = atoi(argv[1]);
        timeout.tv_usec = 0;            /* No microseconds */
    }

    /* Process remaining arguments to build 'readfds' file descriptor set */

    int nfds = 0;
    fd_set readfds;
    FD_ZERO(&readfds);

    for (int j = 2; j < argc; j++) {
        int fd = atoi(argv[j]);
        if (fd >= FD_SETSIZE)
            cmdLineErr("file descriptor exceeds limit (%d)\n", FD_SETSIZE);

        if (fd >= nfds)
            nfds = fd + 1;              /* Record maximum fd + 1 */
        FD_SET(fd, &readfds);
    }

    /* We've built the arguments; now call select() */

    int ready = select(nfds, &readfds, NULL, NULL, pto);
    if (ready == -1)
        errExit("select");

    /* Display results of select() */

    printf("ready = %d\n", ready);
    for (int fd = 0; fd < nfds; fd++)
        if (FD_ISSET(fd, &readfds))
            printf("%d: readable\n", fd);

    if (pto != NULL)
        printf("timeout after select(): %ld.%03ld\n",
               (long) timeout.tv_sec, (long) timeout.tv_usec / 1000);

    exit(EXIT_SUCCESS);
}
