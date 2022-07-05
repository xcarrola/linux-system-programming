/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* svshm_psem_seqnum.h

   Header file used by svshm_psem_seqnum_int.c and svshm_psem_seqnum_get.c.
*/
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "tlpi_hdr.h"

#define PROJ_ID 'x'             /* For use with ftok() */

struct shmbuf {                 /* Shared memory buffer */
    sem_t sem;                  /* Semaphore to protect access */
    int seqnum;                 /* Sequence number */
};
