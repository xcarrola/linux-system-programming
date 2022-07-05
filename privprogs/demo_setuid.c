/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* demo_setuid.c

   This program can be used to demonstrate the effect of the set-user-ID
   mode bit and set*uid() calls.

   To make this a set-UID-root program:

        sudo chown root demo_setuid
        sudo chmod u+s demo_setuid

  Usage: ./demo_setuid arg...

  Each command-line argument is one of:

  <filename>    A file to be opened (for reading)
  -             Set effetive UID to same value as real UID
  +             Set effectve UID back to the value it had at program start
  .             Display current values of process UIDs
  --            Set effective and save set UID to same value as real UID
*/
#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

static void
usage(char *pname)
{
    fprintf(stderr, "Usage: %s arg...\n", pname);
    fprintf(stderr, "Each argument is one of:\n");
    fprintf(stderr, "    <pathname>   Open file for reading\n");
    fprintf(stderr, "    .            Display current UIDs\n");
    fprintf(stderr, "    -            Set eUID to rUID\n");
    fprintf(stderr, "    +            Reset eUID to original value\n");
    fprintf(stderr, "    --           Set eUID and sUID to rUID\n");

    exit(EXIT_FAILURE);
}

static void
displayIds(void)
{
    uid_t ruid, euid, suid;

    if (getresuid(&ruid, &euid, &suid) == -1)
        errExit("getresuid");

    printf("\t\t\t\trUID = %4ld, eUID = %4ld, sUID = %4ld\n",
            (long) ruid,  (long) euid,  (long) suid);
}

int
main(int argc, char *argv[])
{
    if (argc == 1)
        usage(argv[0]);

    uid_t euidOrig = geteuid();         /* Save initial effective UID */

    /* Iterate through the command-line arguments, each of which specifies
       either an operation to perform on the process UIDs or the name of
       a file to be opened. */

    for (int j = 1; j < argc; j++) {
        if (strcmp(argv[j], "+") == 0) {         /* Raise privilege */
            printf("seteuid(%ld)\n", (long) euidOrig);
            if (seteuid(euidOrig) == -1)
                perror("    seteuid");

        } else if (strcmp(argv[j], "-") == 0) {  /* Lower privilege */
            printf("seteuid(%ld)\n", (long) getuid());
            if (seteuid(getuid()) == -1)
                perror("    seteuid");

        } else if (strcmp(argv[j], "--") == 0) { /* Drop privilege forever */
            printf("setresuid(-1, %ld, %ld)\n",
                    (long) getuid(), (long) getuid());
            if (setresuid(-1, getuid(), getuid()) == -1)
                perror("    setresuid");

        } else if (strcmp(argv[j], ".") == 0) {  /* Display UIDs */
            displayIds();

        } else {                                 /* Open a file */
            int fd = open(argv[j], O_RDONLY);
            if (fd == -1)
                perror("Open failed");
            else {
                printf("Successfully opened %s\n", argv[j]);
                close(fd);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
