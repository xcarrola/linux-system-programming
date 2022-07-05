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
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

__attribute__((weak)) void func(void);

static void
dlsymFunc(void)
{
    dlerror();          /* Clear any outstanding error */

    void (*p)(void) = (void (*)(void)) dlsym(RTLD_DEFAULT, "func");

    printf("RUN TIME:  ");
    if (p == NULL) {
        printf("dlsym(RTLD_DEFAULT) look-up did not find symbol \"func\"\n");
    } else {
        printf("dlsym(RTLD_DEFAULT) look-up succeeded\n");
        (*p)();
    }
}

int
main(int argc, char *argv[])
{
    printf("\n");

    printf("LINK TIME: ");
    if (func) {
        printf("A definition of \"func\" was visible\n");
        func();
    } else {
        printf("A definition of \"func\" was NOT visible\n");
    }
    printf("\n");

    /* If there were no command-line arguments, do one dlsym() look-up of
       "func" and terminate. */

    if (argc == 1) {
        dlsymFunc();
        exit(EXIT_SUCCESS);
    }

    /* If there are any command-line arguments, then there must be at
       least two... */

    if (argc < 3) {
        fprintf(stderr, "Usage: %s [<library> {global|local}...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Perform a series of dlopen() calls on library named in argv[1],
       and look up "func" using dlsym(). */

    for (int j = 2; j < argc; j++) {
        int flags = RTLD_NOW;

        /* Open library with RTLD_GLOBAL or RTLD_LOCAL, depending on
           initial letter in current argv element. */

        printf("Executing dlopen(\"%s\", ", argv[1]);

        if (argv[j][0] == 'g') {
            printf("RTLD_GLOBAL");
            flags |= RTLD_GLOBAL;
        } else {
            printf("RTLD_LOCAL");
            flags |= RTLD_LOCAL;
        }
        printf(")\n\n");

        void *handle = dlopen(argv[1], flags);
        if (handle == NULL) {
            printf("ERROR: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        dlsymFunc();
        printf("\n");

        dlclose(handle);
    }

    exit(EXIT_SUCCESS);
}
