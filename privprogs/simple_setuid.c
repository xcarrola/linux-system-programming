/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* simple_setuid.c

   Display process UIDs and attempt to open the file named in argv[1].

   This program can be used to do a simple demonstration of the effect of
   the set-user-ID mode bit.

   To make this a set-UID-root program:

        sudo chown root simple_setuid
        sudo chmod u+s simple_setuid
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int
main(int argc, char *argv[])
{
    printf("rUID = %ld, eUID = %ld\n", (long) getuid(), (long) geteuid());

    /* If an argument was supplied, try to open that file */

    if (argc > 1) {
        int fd = open(argv[1], O_RDONLY);
        if (fd >= 0)
            printf("Successfully opened %s\n", argv[1]);
        else
            perror("Open failed");
    }

    exit(EXIT_SUCCESS);
}
