/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* timerfd_simple.c

   Demonstrate the use of the timerfd API, which creates timers whose
   expirations can be read via a file descriptor.

   The program creates a timerfd timer based on its command-line arguments,
   and then sits in an infinite loop where it (1) waits for the user to
   press the ENTER key and (2) reads the expiration count from the timerfd
   file descriptor, and displays that expiration count (along with the total
   number of expirations).
*/
#include <sys/timerfd.h>
#include <time.h>
#include <stdint.h>                     /* Definition of uint64_t */
#include <inttypes.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 2)
        usageErr("%s secs [nsecs [int-secs [int-nsecs]]]\n", argv[0]);

    struct itimerspec ts;

    ts.it_value.tv_sec = atoi(argv[1]);
    ts.it_value.tv_nsec = (argc > 2) ? atoi(argv[2]) : 0;
    ts.it_interval.tv_sec = (argc > 3) ? atoi(argv[3]) : 0;
    ts.it_interval.tv_nsec = (argc > 4) ? atoi(argv[4]) : 0;

    int fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd == -1)
        errExit("timerfd_create");

    if (timerfd_settime(fd, 0, &ts, NULL) == -1)
        errExit("timerfd_settime");

    /* Each time the user presses the enter key, read the number of timer
       expirations from the timerfd file descriptor, add it to the total
       expirations, and output the two values. */

    uint64_t numExp, totalExp;
    totalExp = 0;

    for (;;) {
        char ch;
        read(STDIN_FILENO, &ch, 1);

        ssize_t s = read(fd, &numExp, sizeof(uint64_t));
        if (s != sizeof(uint64_t))
            errExit("read");

        totalExp += numExp;

        printf("Expirations read: %" PRIu64 "; total: %" PRIu64 "\n",
                numExp, totalExp);
    }

    exit(EXIT_SUCCESS);
}
