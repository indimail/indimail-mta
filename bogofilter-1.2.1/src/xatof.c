/** \file xatof.c
 * Implements xatof, an easy to use strtod() wrapper.
 *
 * \author Matthias Andree
 * \date 2003
 */

#include "xatox.h"

#include <math.h>	/* for strtod() for SunOS 4.1.X */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

int xatof(double *d, const char *in) {
    double val;
    char *end;
    errno = 0;
    val = strtod(in, &end);
    if (end == in /* input string empty or does not start with sign/digit */
	    || end < in + strlen(in) /* junk at end of in */
	    || errno == EINVAL /* SUSv3: "no conversion could be performed" */
	    || errno == ERANGE /* overflow, SUSv3 CX underflow */) 
	if (*end != ',')
	    return 0;
    *d = val;
    return 1;
}

#ifdef MAIN
#include <stdio.h>

int main(int argc, char **argv) {
    int i;
    for (i = 1; i < argc; i++) {
	double d;
	int s = xatof(&d, argv[i]);
	printf("%s -> errno=%d_(%s) status=%d", argv[i], errno,
		strerror(errno), s);
	if (s) printf(" double=%g", d);
	printf("\n");
    }
    exit(EXIT_SUCCESS);
}
#endif
