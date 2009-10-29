/*
 * $Log: filewrt.c,v $
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
static char     sccsid[] = "$Id: filewrt.c,v 2.2 2004-05-03 22:04:37+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
/* function to write to a file */
int
filewrt(int fout, char *fmt, ...)
{
	va_list         ap;
	char           *ptr;
	char            buf[2048];
	unsigned        len;

	va_start(ap, fmt);
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	for (len = 0, ptr = buf; *ptr++; len++);
	return(write(fout, buf, len) != len ? -1 : len);
}
#else
#include <varargs.h>
int
filewrt(va_alist)
va_dcl
{
	va_list         args;
	char           *fmt;
	char            buf[2048];
	int             fout;
#ifdef SUN41
	int             len;
#else
	unsigned        len;
#endif

	va_start(args);
	fout = va_arg(args, int);	/* file descriptor */
	fmt = va_arg(args, char *);
	(void) vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	for (len = 0, fmt = buf; *fmt++; len++);
	return(write(fout, buf, len) != len ? -1 : len);
}
#endif

void
getversion_filewrt_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
