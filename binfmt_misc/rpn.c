/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* rpn.c

   A simple RPN calculator for use as a binfmt_misc interpreter.

   Each line begins with one of the following (no leading spaces allowed):

   [0-9]        Read a floating-point number into 'current' register
   p            Push 'current' onto stack
   =            Print value of 'current'
   +            current = TOS +  current; pop stack
                (TOS = top of stack)
   -            current = TOS -  current; pop stack
   *            current = TOS *  current; pop stack
   /            current = TOS /  current; pop stack
   **           current = TOS ** current; pop stack
   #            Remainder of line is treated as a comment
*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
main(int argc, char *argv[])
{
#define LMAX 100
    char line[LMAX];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <script>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

#define STACK_SIZE 10
    double stack[STACK_SIZE];
    int tos = 0;
    double current = 0.0;

    for (int cnt = 1; ; cnt++) {
        if (fgets(line, LMAX, fp) == NULL)
            break;

        char *newline = strchr(line , '\n');
        if (newline != NULL)
            *newline = '\0';

        char ch = line[0];

        if (strchr("+-/*", ch) != NULL) {       /* Arithmetic */
            if (tos == 0) {
                fprintf(stderr, "Empty stack at line %d\n", cnt);
                exit(EXIT_FAILURE);
            }

            tos--;
            double popped = stack[tos];

            if (ch == '+') {
                current = popped + current;
            } else if (ch == '-') {
                current = popped - current;
            } else if (ch == '/') {
                current = popped / current;
            } else if (ch == '*') {
                if (strcmp(line, "**") == 0) {  /* Exponentiation */
                    current = pow(popped, current);
                } else {
                    current = popped * current;
                }
            }
        } else if (ch == '=') {                 /* Print 'current' */
            printf("%f\n", current);
        } else if (ch == 'p') {                 /* Push 'current' onto stack */
            if (tos >= STACK_SIZE) {
                fprintf(stderr, "Stack overflow at line %d\n", cnt);
                exit(EXIT_FAILURE);
            }
            stack[tos] = current;
            tos++;
        } else if (ch >= '0' && ch <= '9') {    /* Read into 'current' */
            current = atof(line);
        } else if (ch == '#') {                 /* Comment */
            /* Nothing */
        } else {
            fprintf(stderr, "Bad input: %s\n", line);
            exit(EXIT_FAILURE);
        }

    }
    exit(EXIT_SUCCESS);
}
