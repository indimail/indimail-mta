/*
 * $Log: filewrt.c,v $
 * Revision 2.3  2009-11-08 00:56:18+05:30  Cprogrammer
 * makde code more compact
 *
 * Revision 2.2  2004-05-03 22:04:37+05:30  Cprogrammer
 * use stdarg.h instead of varargs.h
 *
 * Revision 2.1  2002-11-28 00:36:27+05:30  Cprogrammer
 * fprintf equivalent function to use with file descriptors
 *
 */
#include "indimail.h"
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: filewrt.c,v 2.3 2009-11-08 00:56:18+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
/* function to write to a file */
int
filewrt(int fout, char *fmt, ...)
#else
#include <varargs.h>
int
filewrt(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif
#ifndef HAVE_STDARG_H
	int             fout;
	char           *fmt;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	fout = va_arg(ap, int);	/* file descriptor */
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	for (len = 0, ptr = buf; *ptr++; len++);
	return(write(fout, buf, len) != len ? -1 : len);
}

void
getversion_filewrt_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
