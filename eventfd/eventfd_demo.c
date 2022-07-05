/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* eventfd_demo.c
*/
#include <sys/eventfd.h>
#include <inttypes.h>           /* Definition of PRIu64 & PRIx64 */
#include <stdint.h>             /* Definition of uint64_t */
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int sleepInterval = 5;
    int flags  = 0;
    int opt;

    while ((opt = getopt(argc, argv, "Bn:s")) != -1) {
        switch (opt) {
        case 'B':
             flags |= EFD_NONBLOCK;
             break;
        case 's':
             flags |= EFD_SEMAPHORE;
             break;
        case 'n':
            sleepInterval = atoi(optarg);
            break;
        default:
            usageErr("Usage: %s [-B] [-s] [-n nsecs] [init-val]\n", argv[0]);
        }
    }

    int initval = (argc > optind) ? atoi(argv[optind]) : 0;
    int efd = eventfd(initval, flags);
    if (efd == -1)
        errExit("eventfd");

    pid_t childPid = fork();
    if (childPid == -1)
        errExit("fork");

    if (childPid == 0) {        /* Child */
        for (;;) {
            char buf[100];
            fgets(buf, sizeof(buf), stdin);

            uint64_t val = strtoull(buf, NULL, 0);
            ssize_t nw = write(efd, &val, sizeof(uint64_t));
            if (nw != sizeof(uint64_t))
                errMsg("write");
        }
    } else {                    /* Parent */
        for (;;) {
            printf("\t\tParent about to sleep\n");
            sleep(sleepInterval);

            printf("\t\t\tParent about to read\n");
            uint64_t val;
            ssize_t nr = read(efd, &val, sizeof(uint64_t));
            if (nr == sizeof(uint64_t)) {
                printf("\t\t\t\tParent read %"PRIu64"\n", val);
            } else {
                if (nr == -1 && errno == EAGAIN)
                    printf("\t\t\t\tread() returned EAGAIN\n");
                else
                    fatal("Bad read");
            }
        }
    }

    exit(EXIT_FAILURE);
}
