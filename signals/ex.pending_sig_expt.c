/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* pending_sig_expt.c

   Write a program, pending_sig_expt.c, that:

   * Establishes a handler for SIGINT that does nothing but return
   * Blocks all signals except SIGINT (sigprocmask())
   * Calls pause() to wait for a signal
   * After pause() returns, retrieves the list of pending signals for
     the process (sigpending()) and prints them (strsignal())

   Run the program and send it various signals (other than SIGINT)
   using the kill(1) command, or by using keyboard keys to generate signals:
   ^Z (SIGTSTP) and ^\ (SIGQUIT). (Send signals whose default disposition is
   not "ignore".) Then type Control-C to generate SIGINT and inspect
   the list of pending signals.

   Extend the program created in the preceding exercise so that:
   * Just after installing the handler for SIGINT, it installs an
     additional handler for the SIGQUIT signal (generated when the
     Control-\ key is pressed). The handler should print a message
     "SIGQUIT received", and return.
   * After displaying the list of pending signals, unblock SIGQUIT
     and call pause() once more.

   While the program is blocking signals (i.e., before typing
   Control-C) try typing Control-\ multiple times. After Control-C
   is typed, how many times does the SIGQUIT handler display its
   message? Why?
*/
#define _GNU_SOURCE
#include <signal.h>
#include "tlpi_hdr.h"

/* FIXME: Implement handlers for SIGINT and SIGQUIT; the latter handler
   should display a message that SIGQUIT has been caught */

int
main(int argc, char *argv[])
{

    /* FIXME: Block all signals except for SIGINT */

    /* FIXME: Set up handlers for SIGINT and SIGQUIT */

    /* Pause until SIGINT handler is invoked */

    printf("Pausing... send me some signals (PID=%ld)\n", (long) getpid());
    pause();

    /* FIXME: Retrieve and display set of pending signals */

    /* FIXME: Unblock SIGQUIT */

    pause();

    exit(EXIT_SUCCESS);
}
