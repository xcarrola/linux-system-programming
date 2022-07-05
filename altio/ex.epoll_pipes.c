/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* epoll_pipes.c

   Example of the use of epoll to monitor multiple file descriptors.

   Usage: epoll_pipes num-pipes [num-writes]
                                  def = 1

   Create 'num-pipes' pipes, and perform 'num-writes' writes to
   randomly selected pipes. Then use epoll to inspect the read ends
   of the pipes to see which pipes are readable.
*/
#include <fcntl.h>
#include <time.h>
#include <sys/epoll.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-pipes [num-writes]\n", argv[0]);

    int numPipes = atoi(argv[1]);
    int numWrites = (argc > 2) ? getInt(argv[2], GN_GT_0, "num-writes") : 1;

    /* Allocate an array for the pipe file descriptors */

    int (*pfds)[2];             /* File descriptors for all pipes */
    pfds = calloc(numPipes, sizeof(int [2]));
    if (pfds == NULL)
        errExit("calloc");

    /* FIXME: Create an epoll instance that will be used to monitor read end
       of all pipes */

    /* Create the number of pipes specified on command line */

    for (int j = 0; j < numPipes; j++) {
        if (pipe(pfds[j]) == -1)
            errExit("pipe %d", j);

        struct epoll_event ev;

        /* FIXME: Add read end of pipe to epoll "interest list", monitoring
           for "input possible" */

        /* FIXME: Make read end of pipe nonblocking, so it can be drained of
           data without blocking when all data has been consumed */

    }

    /* Perform specified number of writes to random pipes */

    printf("About to write bytes to random pipes\n");

    srandom((int) time(NULL));
    for (int j = 0; j < numWrites; j++) {
        int randPipe = random() % numPipes;
        /*
        printf("Writing to fd: %3d (read fd: %3d)\n",
                pfds[randPipe][1], pfds[randPipe][0]);
        */
        if (write(pfds[randPipe][1], "a", 1) == -1)
            errExit("write %d", pfds[randPipe][1]);
    }

    printf("About to enter epoll_wait() loop\n");

    int totalReady = 0;         /* Total number of file descriptors found
                                   to be ready by all epoll_wait() calls */
    size_t totalBytesRead = 0;  /* Total bytes read from pipes */

    const int MAX_EVENTS = 10;  /* Maximum number of events to return
                                   in a single epoll_wait() call */
    struct epoll_event evlist[MAX_EVENTS];

    /* FIXME: Loop, calling epoll_wait() repeatedly to discover some ready
       file descriptors. When a file descriptor indicates as ready,
       display the file descriptor number on stdout, and then drain
       all available input, so that the FD does not indicate as ready
       in future epoll_wait() calls.  While looping, calculate the total
       number of ready file descriptors and total number of bytes read. */

    printf("Number of ready file descriptors: %d\n", totalReady);
    printf("Total bytes read = %zu\n", totalBytesRead);

    exit(EXIT_SUCCESS);
}
