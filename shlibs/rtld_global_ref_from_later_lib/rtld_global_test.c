/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* rtld_global_test.c

   This program loads two shared libraries, libfirst.so and libsecond.so,
   and then calls a function, func2(), in libsecond.so that:

   * calls a function, funcx(), that is defined in both libraries;
   * calls a function, func1(), that is defined only in libfirst.so.

   The behavior in both cases depends on whether libfirst.so was opened with
   the RTLD_GLOBAL flag:

   * in the first case, if libfirst.so was opened with RTLD_GLOBAL,
     then the funcx() in libfirst.so will be called, otherwise the funcx()
     in libsecond.so will be called.
   * in the second case, if libfirst.so was opened with RTLD_GLOBAL,
     then the call to func1() succeeds. Otherwise, the function
     call made from libsecond.so fails ("symbol lookup error").

   By default, the program opens libfirst.so with the RTLD_GLOBAL flag,
   If a command-line argument is supplied when running this program, then
   libfirst.so is opened without the RTLD_GLOBAL flag.

*/
#define _GNU_SOURCE
#include <elf.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int
main(int argc, char *argv[])
{
    void (*funcp)(void);
    const char *err;
    int flags = RTLD_LAZY;

    /* Open first library. If no command-line arguments were supplied, then
       open the library with RTLD_GLOBAL, so that the symbols in this library
       are available for libraries that are subsequently dynamically loaded. */

    if (argc == 1)
        flags |= RTLD_GLOBAL;
    else
        flags |= RTLD_LOCAL;

    printf("Main: dlopen()-ing \"./libfirst.so\" with %s flag\n",
            (flags & RTLD_GLOBAL) ? "RTLD_GLOBAL" : "RTLD_LOCAL");

    void *firstHandle = dlopen("./libfirst.so", flags);
    if (firstHandle == NULL) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    /* Open second library */

    printf("Main: dlopen()-ing \"./libsecond.so\"\n");

    void *secondHandle = dlopen("./libsecond.so", RTLD_LAZY | RTLD_LOCAL);
    if (secondHandle == NULL) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    /* Look up address of 'func2' in libsecond.so */

    (void) dlerror();
    *(void **) (&funcp) = dlsym(secondHandle, "func2");
    err = dlerror();
    if (err != NULL) {
        fprintf(stderr, "dlsym failed: %s\n", err);
        exit(EXIT_FAILURE);
    }

    printf("Main: successfully looked up \"func2\" in \"libsecond.so\"\n");

    /* Now call 'func2', which will in turn try to call 'func1' */

    printf("Main: about to call \"func2\"\n");
    (*funcp)();

    exit(EXIT_SUCCESS);
}
