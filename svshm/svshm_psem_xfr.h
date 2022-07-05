/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/*  svshm_psem_xfr.h

   Header file used by the svshm_psem_xfr_reader.c and svshm_psem_xfr_writer.c
   programs.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 1024           /* Size of transfer buffer */

#define SHM_KEY 0x1234          /* Hard coded key for shared memory segment */

#define SHM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
                                /* Permissions for shared memory */

struct shmseg {                 /* Defines structure of shared memory segment */
    sem_t wsem;                 /* Post when it's writer's turn */
    sem_t rsem;                 /* Post when it's reader's turn */
    int   cnt;                  /* Number of bytes used in 'buf' */
    char  buf[BUF_SIZE];        /* Data being transferred */
};
