/* dumbhead.c -- trivial head(1) like program */

/* (C) 2002 by Matthias Andree <matthias.andree@gmx.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details, it is in the file named
 * COPYING.
 */

/* $Id: dumbhead.c 1578 2003-02-03 17:09:47Z relson $ */

#include <stdio.h>
#include <stdlib.h>

/*@noreturn@*/
static void die(const char *tag)
#ifdef __GNUC__
  __attribute__((noreturn))
#endif
;

static void die(const char *tag)
{
    perror(tag);
    exit(EXIT_FAILURE);
}

/* this is a very dumb "head" command that only copies from stdin to
 * stdout, but keeps embedded NUL characters.
 * It can accept a -N option, where N gives the number of lines.
 * If not given, defaults to 10.
 * Further or malformatted arguments are SILENTLY ignored.
 *
 * Diagnosis: read/write errors are detected, printed with perror and
 * let this program exit(EXIT_FAILURE).
 */

int main(int argc, char **argv) {
    int lines = 10, c;

    if (argc >= 2 && argv[1][0] == '-')
	lines = atoi(&argv[1][1]);

    while(lines && (c = getchar()) != EOF) { /* RATS: ignore */
	if (putchar(c) == EOF) die("stdout");
	if (c == '\n') lines--;
    }
    if (ferror(stdin)) die("stdin");
    if (fflush(stdout) == EOF) die("stdout");
    exit (EXIT_SUCCESS);
}
