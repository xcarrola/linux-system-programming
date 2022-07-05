/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* show_eff_ids.c

*/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pwd.h>
#include <grp.h>

int
main(int argc, char *argv[])
{
    struct passwd *pwd = getpwuid(getuid());
    printf("Real UID:      %4ld (%s)\n", (long) getuid(),
            (pwd != NULL) ? pwd->pw_name : "<noname>");

    pwd = getpwuid(geteuid());
    printf("Effective UID: %4ld (%s)\n", (long) geteuid(),
            (pwd != NULL) ? pwd->pw_name : "<noname>");

    struct group* grp = getgrgid(getgid());
    printf("Real GID:      %4ld (%s)\n", (long) getgid(),
            (grp != NULL) ? grp->gr_name : "<noname>");

    grp = getgrgid(getegid());
    printf("Effective GID: %4ld (%s)\n", (long) getegid(),
            (grp != NULL) ? grp->gr_name : "<noname>");

    exit(EXIT_SUCCESS);
}
