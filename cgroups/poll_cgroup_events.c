/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* poll_cgroup_events.c

   Use poll() to monitor cgroups v2 "events" files for POLLPRI events.
*/
#include <fcntl.h>
#include <poll.h>
#include "curr_time.h"
#include "tlpi_hdr.h"

#define MAX_FD 1024

static void
displayFile(int fd)
{
    /* Reset file offset to start of file before each read() */

    if (lseek(fd, 0, SEEK_SET) == -1)
        errExit("lseek");

    char buf[65536];
    ssize_t nr = read(fd, buf, sizeof(buf));
    if (nr == -1)
        errExit("read");
    if (nr == 0)
        fatal("Unexpected EOF");

    write(STDOUT_FILENO, buf, nr);
    write(STDOUT_FILENO, "\n", 1);
}

int
main(int argc, char *argv[])
{
    char *filename[MAX_FD];
    struct pollfd pollfd[MAX_FD];

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <cgroup-events-file>...\n", argv[0]);

    /* Open the events files named on the command line and add resulting
       file descriptors to a pollfd array for use with poll(). */

    int nfds = argc - 1;
    for (int j = 0; j < nfds; j++) {
        int fd = open(argv[j + 1], O_RDONLY);
        if (fd == -1)
            errExit("open");
        if (fd >= MAX_FD)
            fatal("Too many files");

        filename[fd] = argv[j + 1];

        pollfd[j].fd = fd;
        pollfd[j].events = POLLPRI;
    }

    /* Poll the file descriptors repeatedly for POLLPRI events. */

    for (;;) {
        int ready = poll(pollfd, nfds, -1);
        if (ready == -1)
            errExit("poll");

        printf("========== %s: poll() returned %d\n", currTime("%T"), ready);

        /* Iterate through the returned pollfd array. For each element
           where a POLLPRI event occurred, read and display the contents
           of the corresponding cgroup events file. */

        for (int j = 0; j < nfds; j++) {
            if (pollfd[j].revents & POLLPRI) {
                printf("FD %d - %s\n", pollfd[j].fd, filename[pollfd[j].fd]);

                displayFile(pollfd[j].fd);
            }
        }
    }

    exit(EXIT_SUCCESS);
}
