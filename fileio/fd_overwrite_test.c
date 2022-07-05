/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* fd_overwrite_test.c

   Obtain two file descriptors that refer to the file named in the first
   command-line argument, either by opening the file twice (the default)
   or--if the '-d' command-line option is supplied--by opening the file
   once and then duplicating the resulting file descriptor. Then write the
   remaining command-line arguments alternately to each file descriptor.
   Inspecting the resulting file output allows us to see that two separate
   opens of a file do not share a file offset (so that output by one file
   descriptor can overwrite output by the other file descriptor), whereas
   duplicate file descriptors do share a file offset (so that overwriting
   does not occur).
*/
#include <fcntl.h>
#include "tlpi_hdr.h"

static void
usageError(char *error, char *pname)
{
    fprintf(stderr, "Error: %s\n", error);
    fprintf(stderr, "Usage: %s [-d] <file> <string>....\n", pname);
    exit(EXIT_SUCCESS);
}

int
main(int argc, char *argv[])
{
    int fd1, fd2, opt, doDup;

    doDup = 0;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
        case 'd':
            doDup = 1;
            break;
        default:
            usageError("Bad option", argv[0]);
            break;
        }
    }

    if (argc < optind + 1)
        usageError("Missing argument(s)", argv[0]);

    fd1 = open(argv[optind], O_CREAT | O_TRUNC |  O_WRONLY,
               S_IRUSR | S_IWUSR);
    if (fd1 == -1)
        errExit("open1");

    if (doDup) {
        fd2 = dup(fd1);
        if (fd2 == -1)
            errExit("dup");
    } else {
        fd2 = open(argv[optind], O_WRONLY);
        if (fd2 == -1)
            errExit("open1");
    }

    for (int p = optind + 1; p < argc; p++) {
        size_t len = strlen(argv[p]);

        if ((p - optind) % 2 == 1) {
            if (write(fd1, argv[p], len) != len)
                fatal("write error or partial write - 1");
        } else {
            if (write(fd2, argv[p], len) != len)
                fatal("write error or partial write - 2");
        }
    }

    lseek(fd1, 0, SEEK_END);
    write(fd1, "\n", 1);

    if (close(fd1) == -1)
        errExit("close");
    if (close(fd2) == -1)
        errExit("close");

    exit(EXIT_SUCCESS);
}
