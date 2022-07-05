/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* memcg_eventfd.c

   Experiment with the cgroup v1 memory resource controller feature that
   provides eventfd notifications when the memory usage of a cgroup
   passes specified thresholds.

   Example usage:

   (1) Create a memory cgroup and run this program, specifying a range
       of threshold values:

           # mkdir /sys/fs/cgroup/memory/m1
           # ./memcg_eventfd /sys/fs/cgroup/memory/m1 2000000 3000000 4000000

   (2) Place a different shell in the 'm1' cgroup, and from that cgroup
       execute a program that steadily allocates memory:

           # echo $$ > /sys/fs/cgroup/memory/m1/cgroup.procs
           # ./alloc_mem 10000 25000 100000

   (3) Optionally, before performing step (2), you may find it helpful
       to set up a watch command to continuously display the cgroup
       memory usage value:

           $ watch -n 0.1 cat /sys/fs/cgroup/memory/m1/memory.usage_in_bytes
*/
#include <sys/eventfd.h>
#include <fcntl.h>
#include <limits.h>
#include <inttypes.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <cgroup-path> <threshold>...\nn", argv[0]);

    char *cgroupPath = argv[1];

    /* Create eventfd file descriptor that will be used to receive memory
       threshold notifications */

    int eventFd = eventfd(0, 0);
    if (eventFd == -1)
        errExit("eventfd");

    /* Obtain a readable file descriptor for the cgroup's
       'memory.usage_in_bytes' file */

    char memusagePath[PATH_MAX];
    snprintf(memusagePath, sizeof(memusagePath),
            "%s/memory.usage_in_bytes", cgroupPath);
    int memusageFd = open(memusagePath, O_RDONLY);
    if (memusageFd == -1)
        errExit("open-memory.usage_in_bytes");

    /* Obtain a writable file descriptor for the cgroup's
       'cgroup.event_control' file */

    char evctrlPath[PATH_MAX];
    snprintf(evctrlPath, sizeof(evctrlPath),
            "%s/cgroup.event_control", cgroupPath);
    int evctrlFd = open(evctrlPath, O_WRONLY);
    if (evctrlFd == -1)
        errExit("open-cgroup.event_control");

    /* For each threshold specified on the command line, write a record
       of the form '<eventfd-fd> <memory-usage-fd> <threshold>' to the
       'cgroup.event_control' file */

    for (int j = 2; j < argc; j++) {
        char evctrlBuf[1000];
        snprintf(evctrlBuf, sizeof(evctrlBuf), "%d %d %s",
                eventFd, memusageFd, argv[j]);

        if (write(evctrlFd, evctrlBuf, strlen(evctrlBuf)) == -1)
            errExit("write");
    }

    close(evctrlFd);

    /* Loop receiving memory threshold notifications via the eventfd */

    for (int k = 1; ; k++) {

        /* A read() on an eventfd blocks until the integer associated with
           the eventfd gets a nonzero value. The kernel will set the integer
           to 1 each time one of the memory threshold values written to
           'cgroup.event_control' is crossed. */

        uint64_t eventfdVal;
        read(eventFd, &eventfdVal, sizeof(eventfdVal));
        printf("%d: read() returned; eventfd value = %" PRIu64 "\n",
                k, eventfdVal);

        /* Read and display the cgroup's current memory usage */

        if (lseek(memusageFd, 0, SEEK_SET) == -1)
            errExit("lseek");

        char memusageBuf[1000];
        ssize_t s = read(memusageFd, memusageBuf, sizeof(memusageBuf));
        if (s == -1)
            errExit("read");

        printf("memory.usage_in_bytes = %.*s\n", (int) s, memusageBuf);
    }

    exit(EXIT_SUCCESS);
}
