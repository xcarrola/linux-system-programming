/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* svshm_psem_xfr_reader.c

   Read data from a System V shared memory segment using two POSIX unnamed
   semaphores to synchronize; see svshm_psem_xfr_writer.c
*/
#include "svshm_psem_xfr.h"

int
main(int argc, char *argv[])
{
    int shmid;
    struct shmseg *shmp;

    /* FIXME: Get ID of shared memory created by writer and attach it at an
       address chosen by the system */

    /* FIXME: Transfer blocks of data from shared memory to stdout */

    /* Detach shared memory */

    if (shmdt(shmp) == -1)
        errExit("shmdt");

    exit(EXIT_SUCCESS);
}
