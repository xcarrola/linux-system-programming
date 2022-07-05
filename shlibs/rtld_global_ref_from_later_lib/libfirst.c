/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* libfirst.c

*/
#include <stdio.h>

void
funcx(void)
{
    printf("libfirst: funcx called\n");
}

void
func1(void)
{
    printf("libfirst: func1() called\n");
}
