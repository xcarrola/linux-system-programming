/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* poll_pipes_write.c

   Write a program that has the following command-line syntax:

        poll_pipes_write num-pipes num-writes [block-size]

   The program should create 'num-pipes' pipes, and make the write ends of
   each pipe nonblocking (set the O_NONBLOCK flag with fcntl(F_SETFL)).

   The program should then loop 'num-writes time', each time writing
   'block-size' (arbitrary) bytes (default: 100) to a randomly selected
   pipe. During the loop, the program should count the number of writes
   that failed because the pipe was full (write() failed with EAGAIN) and
   the number of partial writes (write() wrote fewer bytes than requested).
   After the above loop completes, the program should employ poll() to
   monitor all of the pipe write ends to see which pipes are still
   writable, and then report the following:

   * The number of times that write() failed with EAGAIN
   * The number of partial writes
   * A list of the pipes that are writable

   Vary the command-line arguments until you see instances of EAGAIN
   errors and partial writes.
*/
#define _GNU_SOURCE
#include <fcntl.h>
#include <time.h>
#include <poll.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s num-pipes [num-writes [block-size]]\n", argv[0]);

    /* Process command-line arguments */

    int numPipes = atoi(argv[1]);
    int numWrites = (argc > 2) ? atoi(argv[2]) : 1;
    size_t blockSize = (argc > 3) ? atoi(argv[3]) : 100;

    if (numPipes <= 0 || numWrites <= 0 || blockSize <= 0)
        fatal("Arguments must be positive numbers");

    /* Allocate the arrays that we use. The arrays are sized according
       to the number of pipes specified on command line */

    int (*pfds)[2];             /* File descriptors for all pipes */
    pfds = calloc(numPipes, sizeof(int [2]));
    if (pfds == NULL)
        errExit("calloc");

    struct pollfd *pollFd = calloc(numPipes, sizeof(struct pollfd));
    if (pollFd == NULL)
        errExit("calloc");

    /* Allocate buffer to be given given to write(); we do not
       care about the content of the bytes */

    char *buf = malloc(blockSize);
    if (buf == NULL)
        errExit("malloc");

    /* Create the number of pipes specified on command line, and
       make the write ends nonblocking */

    for (int j = 0; j < numPipes; j++) {
        if (pipe(pfds[j]) == -1)
            errExit("pipe %d", j);

        /* FIXME: Make write end of pipe nonblocking */

    }

    int eagainCnt = 0;
    int partialWriteCnt = 0;

    /* Perform specified number of writes to random pipes */

    srandom((int) time(NULL));
    for (int j = 0; j < numWrites; j++) {
        int randPipe = random() % numPipes;

        ssize_t nwritten = write(pfds[randPipe][1], buf, blockSize);

        /* FIXME: Check for (and count) EAGAIN errors and partial writes */

    }

    /* Build the file descriptor list to be supplied to poll(). This list
       is set to contain the file descriptors for the write ends of all of
       the pipes. */

    /* FIXME: Initialize 'pollFd' */

    /* FIXME: Call poll() and print number of ready file descriptors */

    /* FIXME: Inspect 'pollFd' list to display list of writable pipes */

    printf("EAGAIN count = %d\n", eagainCnt);
    printf("Partial write count = %d\n", partialWriteCnt);

    exit(EXIT_SUCCESS);
}
