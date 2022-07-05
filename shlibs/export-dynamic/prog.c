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

   Usage: ./prog library-path function-name
*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define usageErr(msg, progName) \
                        do { fprintf(stderr, "Usage: "); \
                             fprintf(stderr, msg, progName); \
                             exit(EXIT_FAILURE); } while (0)

void
main_f1(const char *caller)
{
    printf("    Called %s() (in " __FILE__ ") from %s()\n", __func__, caller);
}

void
main_f2(const char *caller)
{
    printf("    Called %s() (in " __FILE__ ") from %s()\n", __func__, caller);
}

int
main(int argc, char *argv[])
{
    void abc(void);

    if (argc != 3 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "Usage: %s lib-path func-name\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    abc();

    /* Load the shared library named in argv[1]. */

    void *libHandle = dlopen(argv[1], RTLD_LAZY);
    if (libHandle == NULL) {
        fprintf(stderr, "FAILED: dlopen: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    /* Look up the address of the function symbol dynload_xyz() in
      the library we just loaded. */

    (void) dlerror();                           /* Clear dlerror() */
#pragma GCC diagnostic ignored "-Wpedantic"
    void (*funcp)(char *);
    funcp = (void (*)(char *)) dlsym(libHandle, "dynload_xyz");

    const char *err = dlerror();
    if (err != NULL) {
        fprintf(stderr, "FAILED: dlsym: %s\n", err);
        exit(EXIT_FAILURE);
    }

    /* Call the address returned by dlsym() with argv[2] as the argument. */

    (*funcp)(argv[2]);

    dlclose(libHandle);

    exit(EXIT_SUCCESS);
}
