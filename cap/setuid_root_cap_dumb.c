/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* setuid_root_cap_dumb.c

   Set this program up as follows:

       sudo chown root setuid_root_cap_dumb
       sudo chmod u+s setuid_root_cap_dumb
       sudo setcap cap_kill=pe setuid_root_cap_dumb

   This program illustrates how a traditional set-user-ID-root program can
   be converted to a less powerful (i.e., less dangerous) "capabilities
   dumb" application. The program remains set-UID-root, but has capabilities
   attached. When run, the file capabilities will take precedence over
   set-UID-root (i.e., the process will receive only the capabilities
   attached to the file, not all capabilities as would occur when executing
   a set-UID-root program). As the program switches its UIDs between 0 and
   nonzero values, this automatically causes changes in the process's
   capability sets.
*/
#define _GNU_SOURCE
#include <sys/capability.h>
#include <sys/wait.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <linux/securebits.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

/* Display process UIDs and capabilities */

static void
showCredsAndCaps(void)
{
    uid_t ruid, euid, suid;
    if (getresuid(&ruid, &euid, &suid) == -1)
        errExit("getresuid");

    printf("\trUID: %4ld, eUID: %4ld, sUID: %4ld;  ",
            (long) ruid, (long) euid, (long) suid);

    cap_t caps = cap_get_proc();
    if (caps == NULL)
        errExit("cap_get_proc");

    char *str = cap_to_text(caps, NULL);
    if (str == NULL)
        errExit("cap_to_text");

    printf("capabilities: %s\n", str);

    cap_free(caps);
    cap_free(str);
}

#define STACK_SIZE (1024 * 1024)

int
main(int argc, char *argv[])
{
    if (geteuid() != 0 || geteuid() == getuid()) {
        fprintf(stderr, "Run this as a set-user-ID root program\n");
        exit(EXIT_SUCCESS);
    }

    uid_t euid = geteuid();     /* Save copy of privileged UID */

    printf("Program startup\n");
    showCredsAndCaps();

    /* If a command-line argument was supplied (any string), then set the
       SECBIT_NO_SETUID_FIXUP securebits flag. (The process must have the
       CAP_SETPCAP effective capability in order to do this.) As a result,
       the UID transitions that are subsequently performed will *not*
       cause the process's capability sets to be modified. */

    if (argc > 1) {
        if (prctl(PR_SET_SECUREBITS, SECBIT_NO_SETUID_FIXUP) == -1)
            errExit("prctl-PR_SET_SECUREBITS");
    }

    printf("Switching effective UID to nonzero\n");
    if (setresuid(-1, getuid(), -1) == -1)
        errExit("setresuid1");
    showCredsAndCaps();

    printf("Switching effective UID to zero\n");
    if (setresuid(-1, euid, -1) == -1)
        errExit("setresuid2");
    showCredsAndCaps();

    printf("Making all UIDs the same as the real UID\n");
    if (setresuid(-1, getuid(), getuid()) == -1)
        errExit("setresuid3");
    showCredsAndCaps();

    exit(EXIT_SUCCESS);
}
