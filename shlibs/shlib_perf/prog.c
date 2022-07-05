/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* prog.c

*/
#include <unistd.h>
#include <stdlib.h>

void mod_f(void);
extern long long mod_y;

int
main(int argc, char *argv[])
{
    long long j, lim;

    lim = (argc > 1) ? atol(argv[1]) : 2000000000;

    for (j = 0; j < lim; j++) {
        mod_y++;
        mod_f();
    }

    exit(EXIT_SUCCESS);
}
