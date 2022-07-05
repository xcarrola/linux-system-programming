/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* openlibs.c

   Test program to observe when constructors and destructors are called
   foy dynamically loaded libraries.

   Usage: LD_LIBRARY_PATH=. ./openlibs arg...

   Each argument is one of the following:

   lib-path     This is the pathname of a library. The library is explicitly
                opened using dlopen() (which may cause dependent libraries to
                be implicitly opened). The handle of the library is saved as
                handle 'N' (which is displayed in the program output), where
                'N' is the ordinal numer of this library (starting at 0) in
                the list of libraries so far opened.

   -N           Close library handle 'N'.

   .            Display the current state of the link map, using
                dl_iterate_phdr().
*/
#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int
callback(struct dl_phdr_info *info, size_t size, void *data)
{
    printf("    name=%s (%d segments)\n", info->dlpi_name, info->dlpi_phnum);
    return 0;
}

int
main(int argc, char *argv[])
{
#define MAX_LIBS 1000
    void *libHandle[MAX_LIBS];
    char *libName[MAX_LIBS];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s lib-path...\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int lcnt = 0;

    for (int j = 1; j < argc; j++) {

        if (argv[j][0] == '-') {        /* "-N" closes the Nth handle */
            int i = atoi(&argv[j][1]);
            printf("Closing handle %d (%s)\n", i, libName[i]);
            dlclose(libHandle[i]);

        } else if (argv[j][0] == '.') { /* "." displays the link map */

            printf("Link map:\n");
            dl_iterate_phdr(callback, NULL);

        } else {                        /* lib-path */

            if (lcnt >= MAX_LIBS) {
                fprintf(stderr, "Too many libraries (limit: %d)\n", MAX_LIBS);
                exit(EXIT_FAILURE);
            }

            printf("[%d] Opening %s\n", lcnt, argv[j]);

            libHandle[lcnt] = dlopen(argv[j], RTLD_NOW | RTLD_GLOBAL);
            if (libHandle[lcnt] == NULL) {
                fprintf(stderr, "dlopen: %s\n", dlerror());
                exit(EXIT_FAILURE);
            }

            libName[lcnt] = argv[j];
            lcnt++;
        }

        sleep(1);
        printf("\n");
    }

    printf("Program about to exit\n");

    exit(EXIT_SUCCESS);
}
