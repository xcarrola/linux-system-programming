/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* seek_write.c

   Demonstrate the use of lseek() and file I/O system calls.

   Usage: seek_write pathname offset string
*/
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc != 4 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname offset string\n", argv[0]);

    char *pathname = argv[1];
    off_t offset = strtoll(argv[2], NULL, 0);
    char *string = argv[3];

    /* FIXME: Open 'pathname', seek to 'offset', and write 'string' */

    exit(EXIT_SUCCESS);
}
