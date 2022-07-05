/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* nohup.c
*/
#include <signal.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s command [arg]...\n", argv[0]);

    struct sigaction sa;

    /* FIXME: Ignore the SIGHUP signal (set its disposition to SIG_IGN) */

    /* FIXME: Execute the command in argv[1], along with the remaining
       command-line arguments. */

    exit(EXIT_SUCCESS);
}
