/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* pipe_ucase.c

   Demonstrate the use of pipes for communication between a parent and
   child process. The parent reads text from standard input and sends
   it to the child via a pipe.  The child reads text from the pipe,
   converts it to uppercase, and displays it on standard output.
*/
#include <sys/wait.h>
#include <ctype.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 100

int
main(int argc, char *argv[])
{
    char buf[BUF_SIZE];
    int pfd[2];         /* Pipe to send data from parent to child */
    ssize_t cnt;

    /* FIXME: Create a pipe */

    /* FIXME: Create a child process using fork(). After the fork(), the child
       loops, reading data from the pipe, converting the data to upper
       case (toupper()), and writing the converted data to standard output.
       Upon seeing end of file on the pipe, the child terminates. The parent
       loops, reading data from standard input and writing that data to the
       pipe.  Once it sees end of file on standard input, the parent waits
       (wait()) on the child process. */

}
