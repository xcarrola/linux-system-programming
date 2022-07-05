/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/*  svshm_psem_xfr_writer.c

   Read buffers of data data from standard input into a System V shared memory
   segment from which it is copied by svshm_psem_xfr_reader.c

   We use a pair of POSIX unnamed semaphores to ensure that the writer and
   reader have exclusive, alternating access to the shared memory. (I.e., the
   writer writes a block of text, then the reader reads, then the writer
   writes etc). This ensures that each block of data is processed in turn by
   the writer and reader.

   This program needs to be started before the reader process as it creates
   the shared memory and semaphores used by both processes.

   Together, these two programs can be used to transfer a stream of data
   through shared memory as follows:

        $ svshm_psem_xfr_writer < infile &
        $ svshm_psem_xfr_reader > out_file
*/
#include "svshm_psem_xfr.h"

int
main(int argc, char *argv[])
{
    int shmid;
    struct shmseg *shmp;

    /* FIXME: Create shared memory; size it according to the structure to be placed
       in the memory; attach the segment at an address chosen by system. */

    /* FIXME: Initialize reader and writer semaphores; initialize so that
       writer has first access to shared memory. */

    /* FIXME: Transfer blocks of data from stdin to shared memory */

    /* Detach the shared memory */

    if (shmdt(shmp) == -1)
        errExit("shmdt");

    /* Remove the shared memory segment. Even if the reader is still using the
       segment, this is okay. The segment will be removed only after all open
       references are closed. */

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        errExit("shmctl");

    exit(EXIT_SUCCESS);
}
