/** \file xatoi.c
 * Implements xatoi, an easy to use atoi() replacement with error
 * checking.
 *
 * \author Matthias Andree
 * \date 2003
 */

#include "xatox.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int xatoi(int *i, const char *in) {
    long val;
    char *end;
    errno = 0;
    val = strtol(in, &end, 10);
    if (end == in /* input string empty or does not start with sign/digit */
	    || end < in + strlen(in) /* junk at end of in */
	    || errno == EINVAL /* "base not supported" (shouldn't happen) */
	    || errno == ERANGE /* underflow or overflow */) return 0;
    if (val > INT_MAX || val < INT_MIN) { /* out of range for 'int' type */
	errno = ERANGE;
	return 0;
    }
    *i = (int)val; /* safe after range check */
    return 1;
}

#ifdef MAIN
#include <stdio.h>

int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
	int d;
	int s = xatoi(&d, argv[i]);
	printf("%s -> errno=%d_(%s) status=%d int=%d\n", argv[i], errno,
		strerror(errno), s, d);
    }
    exit(EXIT_SUCCESS);
}
#endif
