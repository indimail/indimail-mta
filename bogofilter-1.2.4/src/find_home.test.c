/* find_home.test.c -- program to test find_home library */

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

/* $Id: find_home.test.c 6484 2006-05-29 14:28:00Z relson $ */

#include "common.h"

#include <ctype.h>
#include <stdlib.h>

#include "find_home.h"

enum mode { current = 42, by_user, tilde };

/* run:
 * without arguments or with 0 argument to get home by user id
 * with tilde argument to try tilde expansion
 * with non-0 argument to try $HOME, then try user id
 */
int main(int argc, char **argv)
{
    const char *h;
    char *tofree = NULL;
    int read_env = 0;
    enum mode mode = current;

    if (argc >= 2) {
	if (!isdigit((unsigned char)argv[1][0])) {
	    if (argv[1][0] == '~') {
		mode = tilde;
	    } else {
		mode = by_user;
	    }
	} else {
	    read_env = atoi(argv[1]);
	}
    }

    switch (mode) {
	case by_user:
	    h = find_home_user(argv[1]);
	    break;
	case tilde:
	    h = tofree = tildeexpand(argv[1]);
	    break;
	case current:
	    h = find_home(read_env);
	    break;
	default:
	    abort();
    }

    if (h != NULL) {
	(void)puts(h);
    } else {
	perror(argv[0]);
    }
    if (tofree)
	free(tofree);

    exit(h ? EXIT_SUCCESS : EX_ERROR);
}
