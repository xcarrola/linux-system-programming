/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

void
dynload_xyz(char *fname)
{

    printf("Entered %s() (in " __FILE__ ")\n", __func__);

    printf("    About to call dlsym(RTLD_DEFAULT, \"%s\")\n", fname);
    (void) dlerror();                           /* Clear dlerror() */
#pragma GCC diagnostic ignored "-Wpedantic"
    void (*funcp)(const char *);
    funcp = (void (*)(const char *)) dlsym(RTLD_DEFAULT, fname);

    const char *err = dlerror();
    if (err != NULL) {
        fprintf(stderr, "    FAILED: dlsym: %s\n", err);
        exit(EXIT_FAILURE);
    }

    (*funcp)(__func__);
}
