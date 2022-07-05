/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* i_epoll.c

   Interactively experiment with epoll.
*/
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>
#include "inet_sockets.h"
#include "tlpi_hdr.h"

/* A handler for SIGINT whose only purpose is to allow the interruption
   of blocking system calls */

static void
sigintHandler(int sig)
{
}

/* Convert a flags string into corresponding flags bit mask for open()
   and events bit mask for epoll */

static bool
getFlags(char *flagsStr, int *oflags, int *eflags)
{
    bool rflag = strchr(flagsStr, 'r') != NULL;
    bool wflag = strchr(flagsStr, 'w') != NULL;
    int of = 0;

    *eflags = 0;

    if (rflag && wflag) {
        of = O_RDWR;
        *eflags = EPOLLIN | EPOLLOUT;
    } else if (wflag) {
        of = O_WRONLY;
        *eflags = EPOLLOUT;
    } else if (rflag) {
        of = O_RDONLY;
        *eflags = EPOLLIN;
    }

    if (strchr(flagsStr, 'e') != NULL)
        *eflags |= EPOLLET;
    if (strchr(flagsStr, 'o') != NULL)
        *eflags |= EPOLLONESHOT;

    if (oflags != NULL)
        *oflags = of;

    return true;
}

/* Set the blocking mode associated with a file descriptor */

static void
setBlockingMode(int fd, bool on)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags == -1)
        errExit("fcntl");

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if (fcntl(fd, F_SETFL, flags) == -1)
        errExit("fcntl");
}

/* Perform an epoll_wait() and display the results */

#define MAX_EVENTS     100

static void
doEpollWait(int epfd, int timeout)
{
    struct epoll_event evlist[MAX_EVENTS];

    int ready = epoll_wait(epfd, evlist, MAX_EVENTS, timeout);
    if (ready == -1) {
        errMsg("epoll_wait");
        return;
    }

    printf("Ready: %d\n", ready);

    for (int j = 0; j < ready; j++) {
        printf("    fd: %d; events: %s%s%s%s\n", evlist[j].data.fd,
                (evlist[j].events & EPOLLIN)  ? "EPOLLIN "  : "",
                (evlist[j].events & EPOLLOUT) ? "EPOLLOUT " : "",
                (evlist[j].events & EPOLLHUP) ? "EPOLLHUP " : "",
                (evlist[j].events & EPOLLERR) ? "EPOLLERR " : "");
    }
}

/* Use the settings specified in 'flagsStr' to add or modify the events mask
   associated with the file descriptor 'fd' in the epoll interest list
   referred to by 'epfd'. */

static void
epollCtl(int epfd, int fd, char commChar, char *flagsStr)
{
    struct epoll_event ev;
    int eflags;

    if (!getFlags(flagsStr, NULL, &eflags))
        return;

    ev.events = eflags;
    ev.data.fd = fd;

    int cmd = (commChar == 'm') ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

    if (epoll_ctl(epfd, cmd, fd, &ev) == -1)
        errMsg("epoll_ctl");
}

/* Perform a read or write operation (as specified in 'cmd') transferring
   'size' bytes to/from the file descriptor 'fd'. For write operations,
   the buffer of output bytes is initialized to 'ch'. */

static void
performIO(char cmd, int fd, size_t size, char ch)
{
    char *buf = malloc(size);
    if (buf == NULL)
        errExit("malloc");

    if (cmd == 'R' || cmd == 'W')
        setBlockingMode(fd, true);

    if (cmd == 'r' || cmd == 'R') {

        ssize_t s = read(fd, buf, size);
        if (s == -1) {
            if (errno == EAGAIN)
                printf("EAGAIN!\n");
            else if (errno == EINTR)
                /* Interrupted by signal handler  */ ;
            else
                errMsg("read");
        } else {
            printf("Read %zd bytes", s);
            if (s < size)
                printf(" (partial read)");
            printf("\n");
        }

    } else if (cmd == 'w' || cmd == 'W') {

        memset(buf, ch, size);

        ssize_t s = write(fd, buf, size);

        if (s == -1) {
            if (errno == EAGAIN)
                printf("EAGAIN!\n");
            else if (errno == EINTR)
                /* Interrupted by signal handler  */ ;
            else
                errMsg("write");
        } else {
            printf("Wrote %zd bytes", s);
            if (s < size)
                printf(" (partial write)");
            printf("\n");
        }

    } else {    /* Should never happen */
        fatal("Unexpected command in performIO()");
    }

    if (cmd == 'R' || cmd == 'W')
        setBlockingMode(fd, false);

    free(buf);
}

/* Open an object and add it to epoll interest list.
   'arg' is a string identifying an object (e.g., a file or socket),
   followed by a colon plus flags that define how the resulting file
   descriptors should be monitored with epoll. */

static void
openObject(int epfd, char *arg)
{
    char *flagsStr;
    int oflags, eflags;
    int fd;

    /* Split out the flags from the argument */

    char *p = strchr(arg, ':');

    if (p == NULL)
        flagsStr = "r";         /* Default is open/monitor for input */
    else {
        flagsStr = p + 1;
        *p = '\0';
    }

    if (!getFlags(flagsStr, &oflags, &eflags))
        exit(EXIT_FAILURE);

    if (strncmp(arg, "s%", 2) == 0) {           /* Connect a TCP socket */

        /* Arg has form 's%host%port'; <host> is at arg[j][2];
           <port> starts at character after second '%' */

        p = strchr(&arg[2], '%');
        if (p == NULL) {
            fprintf(stderr, "Badly formed socket ID\n");
            exit(EXIT_FAILURE);
        }

        *p = '\0';      /* Null-terminate <host> */

        fd = inetConnect(&arg[2], p + 1, SOCK_STREAM);

        printf("Socket connected to [%s, %s] on fd %d\n",
                &arg[2], p + 1, fd);

    } else {                                    /* Open a pathname */

        fd = open(arg, oflags);
        if (fd == -1)
            errExit("open");
        printf("Opened \"%s\" on fd %d\n", arg, fd);
    }

    /* Add file descriptor to the epoll interest list */

    struct epoll_event ev;
    ev.events = eflags;
    ev.data.fd = fd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1)
        errExit("epoll_ctl");
}

static void
showCommandHelp(void)
{
    printf("\n");
    printf("p [<timeout>]\n");
    printf("        Do epoll_wait() with timeout (milliseconds; default 0;"
           " -1 == infinite)\n");
    printf("m <fd> [<flags>]\n");
    printf("        Modify epoll settings of <fd>; <flags> can include:\n");
    printf("        'r' - EPOLLIN\n");
    printf("        'w' - EPOLLOUT\n");
    printf("        'e' - EPOLLET\n");
    printf("        'o' - EPOLLONESHOT\n");
    printf("        If no flags are given, disable <fd> in the "
                   "interest list\n");
    printf("a <fd> [<flags>]\n");
    printf("        Add <fd> to epoll list; <flags> is as for 'm' command\n");
    printf("x <fd>  Delete <fd> from epoll list\n");
    printf("r <fd> <size>\n");
    printf("        Blocking read of <size> bytes from <fd>\n");
    printf("R <fd> <size>\n");
    printf("        Nonblocking read of <size> bytes from <fd>\n");
    printf("w <fd> <size> [<char>]\n");
    printf("        Blocking write of <size> bytes to <fd>; <char> is "
                   "character\n");
    printf("        to write (default: 'x')\n");
    printf("W <fd> <size> [<char>]\n");
    printf("        Nonblocking write of <size> bytes to <fd>\n");
    printf("o <object>[:<flags>]\n");
    printf("        Open an object and add it to epoll list\n");
    printf("        <object> is a pathname or a socket (s%%host%%port)\n");
    printf("        <flags> is as for 'm' command, with a default or 'r'\n");
    printf("d <fd>  Duplicate file descriptor (dup())\n");
    printf("c <fd>  Close file descriptor\n");
    printf("\n");
    printf("Blocked commands can be interrupted with control-C\n");
}

/* Execute a command */

static void
executeCommand(char *line, int epfd)
{
    char cmd[10], arg1[512], arg2[512], arg3[512];
    int timeout;

    int nr = sscanf(line, "%s %s %s %s", cmd, arg1, arg2, arg3);

    if (nr <= 0)        /* Empty command-line */
        return;

    /* Execute the specified command */

    switch (cmd[0]) {
    case 'h':
        showCommandHelp();
        break;

    case 'r':                           /* read/write */
    case 'R':
    case 'w':
    case 'W':
        if (nr < 3) {
            printf("Command requires arguments: <fd> and <size>\n");
            break;
        }

        performIO(cmd[0], atoi(arg1), atoi(arg2),
                (nr < 4) ? 'x' : arg3[0]);
        break;

    case 'p':                           /* epoll_wait() */
        timeout = (nr == 1) ? 0 : atoi(arg1);

        doEpollWait(epfd, timeout);
        break;

    case 'm':                           /* EPOLL_CTL_MOD */
    case 'a':                           /* EPOLL_CTL_ADD */
        if (nr < 2) {
            printf("Command requires at least one argument, <fd>, "
                    "and, optionally, <flags>\n");
            break;
        }

        epollCtl(epfd, atoi(arg1), cmd[0], (nr < 3) ? "" : arg2);
        break;

    case 'x':                           /* EPOLL_CTL_DEL */
        if (nr < 2) {
            printf("Command requires <fd> argument\n");
            break;
        }

        if (epoll_ctl(epfd, EPOLL_CTL_DEL, atoi(arg1), NULL) == -1)
            errExit("epoll_ctl");
        break;

    case 'o':                           /* open() + EPOLL_CTL_ADD */
        if (nr < 2) {
            printf("Command requires <object>[:<flags>] argument\n");
            break;
        }

        openObject(epfd, arg1);

        break;

    case 'c':                           /* close() */
        if (nr < 2) {
            printf("Command requires <fd> argument\n");
            break;
        }

        if (close(atoi(arg1)) == -1)
            errMsg("close");
        break;

    case 'd':                           /* dup() */
        if (nr < 2) {
            printf("Command requires <fd> argument\n");
            break;
        }

        int d = dup(atoi(arg1));
        if (d == -1)
            errMsg("dup");
        else
            printf("New FD: %d\n", d);
        break;

    default:
        printf("Unknown command; type 'h' for help\n");
        break;
    }
}

/* Read and execute commands */

static void
processCommands(int epfd)
{
    printf("Type 'h' for help\n");

    for (;;) {
        char line[1024];

        printf("i_epoll> ");
        fflush(stdout);

        /* Read a command */

        errno = 0;
        char *res = fgets(line, sizeof(line), stdin);
        if (res == NULL) {
            if (errno == EINTR)
                continue;
            else {
                puts("");
                break;
            }
        }

        executeCommand(line, epfd);
    }
}

static void
usageError(char *progName)
{
    fprintf(stderr, "Usage: %s <arg>...\n", progName);
    fprintf(stderr, "<arg> is one of:\n");
    fprintf(stderr, "        <file>[:<flags>]\n");
    fprintf(stderr, "            Open a file\n");
    fprintf(stderr, "        s%%host%%<port>[:<flags>]\n");
    fprintf(stderr, "            Connect a TCP socket to host/port\n");
    fprintf(stderr, "<flags> can be one or more of:\n");
    fprintf(stderr, "        r  - EPOLLIN (and open O_RDONLY)\n");
    fprintf(stderr, "        w  - EPOLLOUT (and open O_WRONLY)\n");
    fprintf(stderr, "        rw - EPOLLIN | EPOLLOUT (and open O_RDWR)\n");
    fprintf(stderr, "        e  - EPOLLET\n");
    fprintf(stderr, "        o  - EPOLLONESHOT\n");
    fprintf(stderr, "        Default: r\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageError(argv[0]);

    /* Set up SIGINT handler */

    struct sigaction sa;
    sa.sa_flags = 0;
    sa.sa_handler = sigintHandler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    /* Create the epoll instance */

    int epfd = epoll_create(argc - 1);
    if (epfd == -1)
        errExit("epoll_create");

    /* Open each argument on command line, adding it to the "interest list"
       for the epoll instance */

    for (int j = 1; j < argc; j++)
        openObject(epfd, argv[j]);

    processCommands(epfd);

    exit(EXIT_SUCCESS);
}
