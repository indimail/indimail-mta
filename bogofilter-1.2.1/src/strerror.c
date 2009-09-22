/* strerror.c - replacement strerror for systems that have sys_errlist */
/* (C) 2004 by Matthias Andree. License: GNU GPL v2. */

#include <errno.h>
#include <stdio.h>

extern int sys_nerr;
extern char *sys_errlist[];

char *strerror(int errnum) {
    static char buf[80];
    if (errnum >= 0 && errnum < sys_nerr)
	return sys_errlist[errnum];
    sprintf(buf, "Unknown error %d", errnum);
    return buf;
}
