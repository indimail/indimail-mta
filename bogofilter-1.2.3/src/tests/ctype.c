/** \file ctype.c
 * This file tests whether isspace() agrees with the POSIX definition of whitespace.
 * If so, it exits with EXIT_SUCCESS (0).
 * On broken systems such as OpenBSD 2.9 to 3.6 that return true for
 * isspace(0xa0), it will exit with EXIT_FAILURE.
 * \author Matthias Andree
 * \date 2005
 *
 * GNU General Public License v2, without "any later version" option.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char POSIX_space[] = " \f\n\r\t\v";

int main(void) {
    int i;

    for (i=1; i < 256; i += 1) {
	int posix = strchr(POSIX_space, i) != NULL;
	int space = isspace(i) != 0;
	if (posix != space) {
	    fprintf( stderr, "Error at %d.  posix: %d, space: %d\n", i, posix, space );
	    fflush(stderr);
	    exit(EXIT_FAILURE);
	}
    }

    exit(EXIT_SUCCESS);
}
