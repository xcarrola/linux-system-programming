/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* zombie_parent.c

   Suppose that we have three processes related as grandparent, parent,
   and child, and that the parent immediately exits but the grandparent
   doesn't immediately perform a wait() after the parent exits, with
   the result that the parent becomes a zombie.

   When is the (grand)child adopted by 'init' (so that getppid() in the
   child returns 1): after the parent terminates or after the grandparent
   does a wait()? This program can be used to demonstrate what happens.
*/
#include <sys/wait.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    printf("Grandparent PID = %5ld\n", (long) getpid());

    /* FIXME: See exercise instructions */

}
