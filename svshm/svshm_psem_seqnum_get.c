/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* svshm_psem_seqnum_get.c

   Fetch and display current value of sequence number in System V
   shared memory object, and optionally add a value to sequence number.

   See also svshm_psem_seqnum_init.c.
*/
#include "svshm_psem_seqnum.h"

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <pathname> [run-length]\n", argv[0]);

    /* Obtain object key */

    key_t key = ftok(argv[1], PROJ_ID);
    if (key == -1)
        errExit("ftok");

    /* Get ID of existing shared memory segment; attach at address chosen
       by system */

    int shmid = shmget(key, 0, 0);
    if (shmid == -1)
        errExit("shmget");

    struct shmbuf *shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1)
        errExit("shmat");

    /* Reserve the semaphore while accessing sequence number */

    if (sem_wait(&shmp->sem) == -1)
        errExit("sem_init");

    printf("Current value of sequence number: %d\n", shmp->seqnum);

    /* If a second command-line argument (a number) was supplied,
       add that number to the sequence number */

    if (argc > 2) {
        int runLength = atoi(argv[2]);
        if (runLength <= 0)
            fprintf(stderr, "Invalid run-length\n");
        else {
            sleep(3);           /* Make update slow */
            shmp->seqnum += runLength;
            printf("Updated sequence number\n");
        }
    }

    if (sem_post(&shmp->sem) == -1)
        errExit("sem_init");

    exit(EXIT_SUCCESS);
}
