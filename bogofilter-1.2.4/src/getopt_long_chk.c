#include "config.h"
#include "longoptions.h"
#include <stdlib.h>
#include <stdio.h>

enum state {
    NO = no_argument,
    REQ = required_argument,
    OPT = optional_argument
};

int getopt_long_chk(int argc, char * const argv[],
	char const *optstring, const struct option *longopts,
	int *longindex)
{
    const char *p = optstring;
    enum state want;
    int error = 0;
    while(p && *p) {
	if (!(*p == '?' || *p == '-')) {
	    if (p[1] == ':') {
		if (p[2] == ':')	want = OPT;
		else			want = REQ;
	    } else			want = NO;
	    int i = 0;
	    if (longopts) {
		while (longopts[i].name) {
		    if (longopts[i].val == *p
			    && longopts[i].has_arg != (int)want) {
			fprintf(stderr,
				"option '%c' == '%s' mismatch: short %d long %d\n",
				*p, longopts[i].name, want, longopts[i].has_arg);
			error = 1;
		    }
		    i++;
		}
	    }
	}
	p++;
    }
    if (error) abort();
    return getopt_long(argc, argv, optstring, longopts, longindex);
}
