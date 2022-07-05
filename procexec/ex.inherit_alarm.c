/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* inherit_alarm.c

   Test to see whether a child process inherits signal dispositions and
   alarm timers from its parent.
*/
#define _GNU_SOURCE
#include <string.h>
#include <signal.h>
#include "tlpi_hdr.h"

static void             /* Handler for SIGALRM */
handler(int sig)
{
    /* UNSAFE: This handler uses non-async-signal-safe functions
      (printf()); see Section 21.1.2) */

    printf("%ld Caught signal %d (%s)\n",
            (long) getpid(), sig, strsignal(sig));
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;

    /* FIXME: Install handler for SIGALRM */

    /* FIXME: Start an alarm timer that expires after two seconds */

    /* FIXME: Create a child process */

    /* FIXME: In the child, test whether the child inherits signal dispositions
       by retrieving the disposition of the SIGALRM signal and checking if the
       'sa_handler' field contains the address of the handler() function */

    /* FIXME: Both parent and child go on to execute a loop in which the PID is
       printed and the process uses usleep() to sleep for half a second. */

    exit(EXIT_SUCCESS);
}
