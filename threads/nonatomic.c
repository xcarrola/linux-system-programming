/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* nonatomic.c

   Accessing some kinds of integers can be nonatomic, even on x86 platforms.
   This can result in "memory tearing" on fetches and stores, where the memory
   access requires two bus transactions. Consequently, the state of the integer
   can be inconsistent if it is simultaneously accessed across two CPUs.

   For example, 'long long' accesses are nonatomic on x86-32, as shown by
   the following example:

        $ uname -m
        x86_64
        $ cc -m32 -DSIZE=8 ./nonatomic.c -pthread
        $ ./a.out | head
        ip = 0x9795000 (size: 8 bytes)
        Unexpected: 00000000ffffffff (loop 949)
        Unexpected: 00000000ffffffff (loop 1480)
        Unexpected: ffffffff00000000 (loop 1497)
        Unexpected: ffffffff00000000 (loop 1498)
        Unexpected: ffffffff00000000 (loop 1530)
        Unexpected: 00000000ffffffff (loop 1538)
        Unexpected: ffffffff00000000 (loop 1555)
        Unexpected: 00000000ffffffff (loop 1706)
        Unexpected: ffffffff00000000 (loop 1709)

   On x86-64, even accesses of a 4-byte integer can be nonatomic, if the
   integer is aligned across a cache-line (64-byte) boundary, as shown by
   this example:

        $ cc -DSIZE=4 ./nonatomic.c -pthread
        $ ./a.out | head
        ip = 0x11e7fff (size: 4 bytes)
        Unexpected: 000000ff (loop 4638)
        Unexpected: ffffff00 (loop 67401)
        Unexpected: 000000ff (loop 121616)
        Unexpected: ffffff00 (loop 126131)
        Unexpected: 000000ff (loop 132458)
        Unexpected: 000000ff (loop 138088)
        Unexpected: ffffff00 (loop 146650)
        Unexpected: ffffff00 (loop 174049)
        Unexpected: 000000ff (loop 177802)
*/
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define errExitEN(en, msg)      do { errno = en; perror(msg); \
                                     exit(EXIT_FAILURE); } while (0)

#ifndef SIZE
#    define SIZE 4
#endif

#if SIZE == 4
#    define TYPE uint32_t
#    define FMT "%08" PRIx32
#elif SIZE == 8
#    define TYPE uint64_t
#    define FMT "%016" PRIx64
#else
#    error "Bad width"
#endif

#define ZERO ((TYPE) 0)

static char *block;
static TYPE * volatile ip;      /* Use 'volatile' to prevent optimizer from
                                   optimizing away code that accesses '*ip'. */

static void *
threadFunc(void *arg)
{
    /* The following loop switches the value of '*ip' between all bits zero
       and all bits one. This operation may possibly not be atomic. */

    for (long long j = 0; ; j++) {
        *ip = ~*ip;

        if ((j > 0) && ((j % 1000000000) == 0))
            printf("%lld\n", j);
    }
    return NULL;
}

#define MEM_SIZE 65536

int
main(int argc, char *argv[])
{
    /* Allocate memory, aligned on a page boundary */

    if (posix_memalign((void **) &block, 4096, MEM_SIZE) == -1)
        errExit("posix_memalign");

    unsigned int offset = 4095;
    if (argc > 1)
        offset = atoi(argv[1]);

    if (offset > MEM_SIZE - SIZE) {
        printf("Offset too large (exceeds %d)\n", MEM_SIZE - SIZE);
        exit(EXIT_FAILURE);
    }

    /* Initialize 'ip' to be a pointer 'offset' bytes into 'block'
       and zero the location it points to. */

    ip = (TYPE *) &block[offset];
    *ip = ZERO;

    printf("ip = %p (size: %zd bytes)\n", (void *) ip, sizeof(*ip));

    pthread_t t1;
    int s = pthread_create(&t1, NULL, threadFunc, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create");

    /* Loop, fetching the value of '*ip' (the fetch may possibly not be
       atomic) and checking whether the result is all bits zero or all bits
       one. If it is not, print the fetched value and the loop counter. */

    for (long long j = 0; ; j++) {
        TYPE loc = *ip;
        if (loc != ZERO && loc != ~ZERO)
            printf("Unexpected: " FMT " (loop %lld)\n", loc, j);
    }
}
