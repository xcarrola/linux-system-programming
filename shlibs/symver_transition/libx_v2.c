/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Source file used to build symbol-versioned library */

#include <stdio.h>

__asm__(".symver abc_v1,xxx_abc@VER_1");
__asm__(".symver abc_v2,xxx_abc@@VER_2");
__asm__(".symver def_v2,xxx_def@@VER_2");

void
abc_v1(void)
{
    printf("Called abc() initial version\n");
}

void
abc_v2(void)
{
    printf("Called abc() v2\n");
}

void
def_v2(void)
{
    printf("Called def() v2\n");
}
