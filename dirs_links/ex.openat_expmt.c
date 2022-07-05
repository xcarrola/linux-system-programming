/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* openat_expmt.c
*/
#if ! defined(_XOPEN_SOURCE) || _XOPEN_SOURCE < 700
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#endif
#include <limits.h>
#include <fcntl.h>
#include <libgen.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    char rlbuf[PATH_MAX];
    char buf[256];

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    /* Obtain the dirname and basename of the command-line argument.
       dirname() and basename() can modify their argument, so
       work on copies of argv[1] */

    char *p1 = strdup(argv[1]);
    char *p2 = strdup(argv[1]);
    char *dir = dirname(p1);
    char *base = basename(p2);

    printf("dirname = %s\n", dir);
    printf("basename = %s\n", base);

    /* Obtain a file descriptor referring to the dirname of the pathname */

    int dirfd = open(dir, O_RDONLY);
    if (dirfd == -1)
        errExit("open");

    /* FIXME: Fetch the target of the 'dir' symbolic link (using
       readlink(dir, ...)') and display the target */

    printf("About to sleep\n");
    sleep(15);

    /* FIXME: Once more fetch and display the target of the 'dir'
       symbolic link */

    /* FIXME: Use open(2) to open the pathname specified in argv[1]
       (*not* 'p1', which may have been modified by dirname()).
       Read and display the contents of the opened file */

    /* FIXME: Use openat(2) to open the file via 'dirfd' and 'base'.
       Read and display the contents of the opened file */

    free(p1);
    free(p2);

    exit(EXIT_SUCCESS);
}
