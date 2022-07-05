/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* auxvec.c

   Display contents of auxiliary vector. For more information, see
   getauxval(3) and "getauxval() and the auxiliary vector",
   https://lwn.net/Articles/519085/
*/
#include <link.h>
#include <elf.h>
#include "tlpi_hdr.h"

extern char **environ;

int
main(int argc, char *argv[])
{
    /* Walk through the array of pointers in environ */

    char **p;
    for (p = environ; *p != NULL; p++)
        continue;

    /* The auxiliary vector sits just above environ */

    p++;        /* Step past NULL pointer at end of 'environ' */

    printf("auxvec                        %12p\n", (void *) p);

    ElfW(auxv_t) *auxv = (ElfW(auxv_t) *) p;

    while (auxv->a_type != 0) {

        switch (auxv->a_type) {
        case AT_PAGESZ: /* Page size */
            printf("AT_PAGESZ:        %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_ENTRY:  /* Entry address of the executable */
            printf("AT_ENTRY:         0x%llx\n", (long long) auxv->a_un.a_val);
            break;
        case AT_UID:    /* Real UID */
            printf("AT_UID:           %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_EUID:   /* Effective UID */
            printf("AT_EUID:          %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_GID:    /* Real GID */
            printf("AT_GID:           %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_EGID:   /* Effective GID */
            printf("AT_EGID:          %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_CLKTCK: /* Frequency with which times(2) counts */
            printf("AT_CLKTCK:        %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_PLATFORM:
                        /* String identifying hardware platform */
            printf("AT_PLATFORM:      0x%llx (string: %s)\n",
                    (long long) auxv->a_un.a_val, (char *) auxv->a_un.a_val);
            break;
        case AT_SECURE: /* Was secure-execution mode triggered for dynamic
                           linker? See ld.so(8). */
            printf("AT_SECURE:        %lld\n", (long long) auxv->a_un.a_val);
            break;
        case AT_RANDOM: /* 16 bytes of randomness */
            printf("AT_RANDOM:        0x%llx\n", (long long) auxv->a_un.a_val);
            printf("                  ");
            uint8_t *b = (uint8_t *) auxv->a_un.a_val;
            for (int j = 0; j < 16; j++)
                printf("%02x ", b[j]);
            printf("\n");
            break;
        case AT_EXECFN: /* Pathname used to execute program */
            printf("AT_EXECFN:        0x%llx (string: %s)\n",
                    (long long) auxv->a_un.a_val, (char *) auxv->a_un.a_val);
            break;
        default:
            /* We ignore other types of entries in auxiliary vector */
            break;
        }

        auxv++;
    }

    exit(EXIT_SUCCESS);
}
