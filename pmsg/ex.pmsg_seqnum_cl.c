/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* pmsg_seqnum_cl.c

   A sequence-number client using POSIX message queues.

   See also pmsg_seqnum_sv.c.
*/
#include <mqueue.h>
#include <fcntl.h>

/* FIXME: Include header file defining messages sent from client to server */

#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct requestMsg req;      /* Message sent to server */

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s /server-mq-name [run-length]\n", argv[0]);

    /* FIXME: Construct pathname for client message queue. Send request to
       send to server containing "run length" and client queue name */

    struct mq_attr attr;        /* For mq_open() call */

    /* FIXME: Create/open client's message queue */

    /* FIXME: Open server queue, and send request */

    int seqnum;

    /* FIXME: Receive server's response, and display sequence number */

    /* FIXME: Remove client's message queue */

    exit(EXIT_SUCCESS);
}
