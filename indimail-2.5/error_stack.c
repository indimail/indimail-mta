/*
 * $Log: error_stack.c,v $
 * Revision 2.6  2011-04-08 17:26:04+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.5  2010-02-16 13:31:54+05:30  Cprogrammer
 * free memory allocated by vasprintf
 *
 * Revision 2.4  2009-11-08 00:53:11+05:30  Cprogrammer
 * added missing declaration for non stdarg.h system
 *
 * Revision 2.3  2009-02-18 21:25:01+05:30  Cprogrammer
 * check return value of vasprintf for error
 *
 * Revision 2.2  2008-08-07 13:19:34+05:30  Cprogrammer
 * added discard_stack()
 * fixed bug with allocation
 *
 * Revision 2.1  2008-08-02 10:30:19+05:30  Cprogrammer
 * Function to stack all error messages till exit
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "error_stack.h"

#ifndef	lint
static char     sccsid[] = "$Id: error_stack.c,v 2.6 2011-04-08 17:26:04+05:30 Cprogrammer Stab mbhangui $";
#endif

void
discard_stack(void)
{
	error_stack(0, 0);
	return;
}

void
flush_stack(void)
{
	error_stack(stderr, 0);
	return;
}

#ifdef HAVE_STDARG_H
#include <stdarg.h>
char           *
error_stack(FILE *ferr, const char *fmt, ...)
#else
#include <varargs.h>
char           *
error_stack(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
	FILE           *ferr;
#endif
	va_list         ap;
	int             len, i;
	static int      mylen;
	static char    *error_store;
	char           *ptr, *errorstr;
	static char     sserrbuf[512];

#ifndef HAVE_STDARG_H
	va_start(ap);
	ferr = va_arg(ap, FILE *);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		if (vasprintf(&errorstr, fmt, ap) == -1) {
			fprintf(ferr, "error_stack: vasprintf: %s\n", strerror(errno));
			fflush(ferr);
			return ((char *) 0);
		}
		va_end(ap);
		len = strlen(errorstr) + 1;
		if (!(error_store = realloc(error_store, mylen + len + 1))) {	/*- The man page is wierd on Mac OS */
			fprintf(ferr, "%s", errorstr);
			free(errorstr);
			fprintf(ferr, "error_stack: realloc: %s\n", strerror(errno));
			fflush(ferr);
			return ((char *) 0);
		}
		if (setvbuf(ferr, sserrbuf, _IOFBF, 512) == EOF)
		{
			fprintf(ferr, "error_stack: setvbuf: %s\n", strerror(errno));
			fflush(ferr);
			return((char *) 0);
		}
		if (!mylen && atexit(flush_stack))
		{
			fprintf(ferr, "atexit: %s\n", strerror(errno));
			fflush(ferr);
			free(errorstr);
			return((char *) 0);
		}
		strncpy(error_store + mylen, errorstr, len);
		free(errorstr);
		error_store[mylen + len - 1] = 0;
		mylen += len;
		return (error_store);
	} else {
		if (!error_store)
			return ((char *) 0);
		if (!ferr) {
			free((void *) error_store);
			error_store = (char *) 0;
			mylen = 0;
			fflush(ferr);
			return ((char *) 0);
		}
		for (ptr = error_store, i = len = 0; len < mylen; len++, ptr++) {
			if (*ptr == 0) {
				fprintf(ferr, "%s", error_store + i);
				i = len + 1;
			}
		}
		fflush(ferr);
		free((void *) error_store);
		error_store = (char *) 0;
		mylen = 0;
		return ("");
	}
}

void
getversion_error_stack_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsid_error_stackh);
}
