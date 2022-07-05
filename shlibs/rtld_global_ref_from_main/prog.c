/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* prog.c */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

void abc(void);
void xyz(void);

int
main(int argc, char *argv[])
{
    /* Open library specified in argv[1], if provided */

    if (argc > 1) {
        printf("About to dlopen(\"%s\")\n", argv[1]);

        /* If a second argument was supplied, open the library with RTLD_LOCAL
           instead of the default RTLD_GLOBAL. */

        int flags = RTLD_LAZY;
        flags |= (argc > 2) ? RTLD_LOCAL : RTLD_GLOBAL;

        void *handle = dlopen(argv[1], flags);
        if (handle == NULL) {
            fprintf(stderr, "dlopen failure: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }

    /* Call the functions abc() and xyz(). Assuming that lazy binding is
       being used, symbol resolution will happen at this point. */

    abc();
    xyz();

    exit(EXIT_SUCCESS);
}
