/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* process_time_test.c

   Demonstrate the use of functions for retrieving process time.
*/
#include <sys/times.h>
#include <time.h>
#include "tlpi_hdr.h"

static void             /* Display 'msg' and process times */
displayProcessTimes(const char *msg)
{
    if (msg != NULL)
        printf("%s", msg);

    long clockTicks = sysconf(_SC_CLK_TCK);
    if (clockTicks == -1)
         errExit("sysconf");

    clock_t clockTime = clock();
    if (clockTime == -1)
        errExit("clock");

    printf("    clock(): %12ld;            %9.4f\n",
            (long) clockTime, (double) clockTime / CLOCKS_PER_SEC);

    struct tms t;
    if (times(&t) == -1)
        errExit("times");
    printf("    times(): uCPU:%.4f + sCPU:%.4f   %.4f\n",
            (double) t.tms_utime / clockTicks,
            (double) t.tms_stime / clockTicks,
            (double) (t.tms_utime + t.tms_stime) / clockTicks);

    struct timespec ts;
    if (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts) == -1)
        errExit("clock_gettime");
    printf("    CLOCK_PROCESS_CPUTIME_ID:   %10ld.%09ld\n",
            (long) ts.tv_sec, ts.tv_nsec);

    if (clock_getres(CLOCK_PROCESS_CPUTIME_ID, &ts) == -1)
        errExit("clock_getres");
    printf("    CLOCK_PROCESS_CPUTIME_ID resolution: %ld.%09ld\n",
            (long) ts.tv_sec, ts.tv_nsec);
}

int
main(int argc, char *argv[])
{
    int numCalls = (argc > 1) ? atoi(argv[1]) : 100000000;

    displayProcessTimes("At program start:\n");

    /* Call getppid() a large number of times, so that
       some user and system CPU time are consumed */

    for (int j = 0; j < numCalls; j++)
        (void) getppid();

    displayProcessTimes("After getppid() loop:\n");

    exit(EXIT_SUCCESS);
}
