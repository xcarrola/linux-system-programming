/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* pipeline_builder.c

   Demonstrate the use of multiple pipes to connect multiple filters.
   This program implements a general-purpose function that can be
   used to build a single stage of a pipeline, thus simplifying the
   task of building a long pipeline.

   Compare this program with count_unique_tokens.c
*/
#include <fcntl.h>
#include <sys/wait.h>
#include <stdarg.h>
#include "tlpi_hdr.h"

/* (Optionally) create a pipe (if 'makePipe' is true) and execute the
   command specified in 'argv'. Duplicate 'infd' in the child so that
   it becomes standard input for the executed command. If a pipe was
   created, then duplicate the write end of the pipe so that it
   becomes the standard output for the executed command. Return
   a file descriptor for the read end of the pipe, or -1 if no pipe
   was created. The file descriptor 'infd' is closed by this function. */

static int
execvPipeline(int infd, bool makePipe, char *const argv[])
{
    int pfd[2];

    /* FIXME: To be implemented */

}

/* Convert the variable-length argument list started by 'arg'
   into a vector and then call execvPipeline() */

static int
execlPipeline(int infd, bool makePipe, char *arg, ...)
{
    int argvSize = 100;
    char **argv = calloc(argvSize, sizeof(void *));
    if (argv == NULL)
        errExit("calloc");

    argv[0] = (char *) arg;
    int argc = 1;

    /* Walk through variable-length argument list until NULL terminator
       is found, building argv as we go */

    va_list argList;
    va_start(argList, arg);
    while (argv[argc - 1] != NULL) {
        if (argc + 1 >= argvSize) {     /* Resize argv if required */
            argvSize += 100;
            argv = realloc(argv, sizeof(void *));
            if (argv == NULL)
                errExit("realloc");
        }

        argv[argc] = va_arg(argList, char *);
        argc++;
    }
    va_end(argList);

    /* Execute the command */

    int fd = execvPipeline(infd, makePipe, argv);

    free(argv);

    return fd;
}

int
main(int argc, char *argv[])
{
    if (argc != 2)
        usageErr("%s filename\n", argv[0]);

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1)
        errExit("open");

    /* Build a pipeline whose first command reads from the file descriptor
       opened in the previous step */

    fd =   execlPipeline(fd, true, "tr", " \t", "\012", (char *) NULL);
    fd =   execlPipeline(fd, true, "sort", "-u", (char *) NULL);
    (void) execlPipeline(fd, false, "wc", "-w", (char *) NULL);

    /* Wait for all children to complete */

    while (waitpid(-1, NULL, 0) > 0)
        continue;

    exit(EXIT_SUCCESS);
}
