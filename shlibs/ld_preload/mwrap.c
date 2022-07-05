/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* mwrap.c

   Define wrapper functions for malloc() and free() that use
   dlsym(RTLD_NEXT) to find "next" versions of these functions
   in the library search order and then call those functions.
   Used to test preloading.
*/
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void *
malloc(size_t size)
{
    char *s = NAME " malloc() entered\n";
    write(STDOUT_FILENO, s, strlen(s));

    void *(*nextMalloc)(size_t);
    nextMalloc = (void * (*)(size_t)) dlsym(RTLD_NEXT, "malloc");

    void *p = (*nextMalloc)(size);

    s = NAME " malloc() returning\n";
    write(STDOUT_FILENO, s, strlen(s));

    return p;
}

void
free(void *ptr)
{
    char *s = NAME " free() entered\n";
    write(STDOUT_FILENO, s, strlen(s));

    void (*nextFree)(void *);
    nextFree = dlsym(RTLD_NEXT, "free");

    (*nextFree)(ptr);

    s = NAME " free() returning\n";
    write(STDOUT_FILENO, s, strlen(s));
}
