/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* timed_read_ptmr.c

   Demonstrate the use of a POSIX timer to place a timeout on a blocking
   system call (read(2) in this case).
*/
#include <time.h>
#include <signal.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 10

static void     /* SIGALRM handler: interrupts blocked system call */
handler(int sig)
{
}

int
main(int argc, char *argv[])
{
    /* Set up handler for SIGALRM */

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = handler;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
        errExit("sigaction");

    /* Initialize timer settings to be given to timer_settime() */

    struct itimerspec its;
    its.it_value.tv_sec = (argc > 1) ? atoi(argv[1]) : 10;
    its.it_value.tv_nsec = 0;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;

    /* FIXME: Create and arm timer */

    char buf[BUF_SIZE];
    ssize_t numRead = read(STDIN_FILENO, buf, BUF_SIZE);

    int savedErrno = errno;                     /* In case alarm() changes it */

    /* FIXME: Disarm timer */

    errno = savedErrno;

    /* Determine result of read() */

    if (numRead == -1) {
        if (errno == EINTR)
            printf("Read timed out\n");
        else
            errMsg("read");
    } else {
        printf("Successful read (%zd bytes): %.*s",
                numRead, (int) numRead, buf);
    }

    exit(EXIT_SUCCESS);
}
