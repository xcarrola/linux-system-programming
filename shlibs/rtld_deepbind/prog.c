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

   Demonstrate the use of the dlopen() RTLD_DEEPBIND flag, which causes
   a symbol reference in a dlopen()-ed shared library to bind locally,
   rather than binding to the definition found elsewhere in the link map.

   To make use of the RTLD_DEEPBIND flag, specify a command-line argument
   when running the program.
*/
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void
func(void)
{
    printf("Called func in " __FILE__ "\n");
}

int
main(int argc, char *argv[])
{
    int flags = RTLD_LAZY;
    if (argc > 1)
        flags |= RTLD_DEEPBIND;

    void *handle = dlopen("./libmod.so", flags);
    if (!handle) {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    dlerror();          /* Clear any existing error */

    void (*modfunc)(void);
    *(void **) (&modfunc) = dlsym(handle, "mod");

    char *error = dlerror();
    if (error != NULL)  {
        fprintf(stderr, "%s\n", error);
        exit(EXIT_FAILURE);
    }

    (*modfunc)();

    exit(EXIT_SUCCESS);
}
