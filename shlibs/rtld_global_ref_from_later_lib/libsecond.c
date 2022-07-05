/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* libsecond.c

*/
#include <stdio.h>

void
funcx(void)
{
    printf("libsecond: funcx called\n");
}

void
func2(void)
{
    extern void func1(void);

    printf("libsecond: func2() called\n\n");

    printf("libsecond::func2(): about to call funcx()\n");
    funcx();

    printf("\nlibsecond::func2(): about to call func1()\n");
    func1();
}
