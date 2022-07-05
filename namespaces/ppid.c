/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* ppid.c

   Print the process's PID and PPID. This program is useful for
   some experiments with PID namespaces.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
   printf("PID:        %6ld\n", (long) getpid());
   printf("Parent PID: %6ld\n", (long) getppid());

   /* If no command-line argument was specified, sleep for a bit. */

   if (argc == 1)
       sleep(1000);

   exit(EXIT_SUCCESS);
}
