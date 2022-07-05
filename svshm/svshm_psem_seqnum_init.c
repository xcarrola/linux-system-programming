/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* svshm_psem_seqnum_init.c

   Reset semaphore inside System V shared memory seqnum, and optionally
   reinitialize sequence number inside object.

   See also svshm_psem_seqnum_get.c.
*/
#include "svshm_psem_seqnum.h"

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <pathname> [init-value]\n", argv[0]);

    char *pathname = argv[1];

    /* Create file to be used by ftok() */

    int fd = open(pathname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");
    close(fd);

    /* Obtain object key */

    key_t key = ftok(pathname, PROJ_ID);
    if (key == -1)
        errExit("ftok");

    /* Create shared memory segment, sized according to structure
       be placed in memory; attach at address chosen by system */

    int shmid = shmget(key, sizeof(struct shmbuf),
                       IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        errExit("shmget");

    struct shmbuf *shmp = shmat(shmid, NULL, 0);
    if (shmp == (void *) -1)
        errExit("shmat");

    /* Reset semaphore to be available */

    if (sem_init(&shmp->sem, 1, 1) == -1)
        errExit("sem_init");

    /* If argv[2] was specified, reinitialize sequence number */

    if (argc > 2)
        shmp->seqnum = atoi(argv[2]);

    exit(EXIT_SUCCESS);
}
