/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* cal_time.c

   Demonstrate the use of functions for retrieving calendar time.
*/
#include <time.h>
#include <sys/time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    /* Retrieve calendar time by various methods */

    time_t t = time(NULL);
    printf("time():         %22ld\n", (long) t);

    struct timespec ts;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        errExit("clock_gettime");
    printf("CLOCK_REALTIME: %22ld.%09ld\n", (long) ts.tv_sec, ts.tv_nsec);

    if (clock_getres(CLOCK_REALTIME, &ts) == -1)
        errExit("clock_getres");
    printf("CLOCK_REALTIME resolution:  %10ld.%09ld\n",
            (long) ts.tv_sec, ts.tv_nsec);

    exit(EXIT_SUCCESS);
}
