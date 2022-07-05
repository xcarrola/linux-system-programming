/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* lib.c */

#include <stdio.h>

#ifdef INCL_ABC
void
abc(void)
{
    printf("Called %s::%s\n", LIB, __func__);
}
#endif

#ifdef INCL_XYZ
void
xyz(void)
{
    printf("Called %s::%s\n", LIB, __func__);
}
#endif
