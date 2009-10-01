/*
 * $Log: mysql_perror.c,v $
 * Revision 2.3  2008-08-02 09:08:15+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2008-05-28 16:37:24+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2004-05-03 22:05:10+05:30  Cprogrammer
 * use stdarg instead of vararg
 *
 * Revision 1.4  2001-11-24 12:19:41+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:55:37+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:23:45+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mysql_perror.c,v 2.3 2008-08-02 09:08:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>

#ifdef HAVE_STDARG_H
#include <stdarg.h>
char           *
mysql_perror(char *fmt, ...)
{
	va_list         ap;
	char           *ptr;
	char            temp[2048];

	va_start(ap, fmt);
	if(vsnprintf(temp, 2048, fmt, ap) == -1)
		temp[2047] = 0;
	va_end(ap);
	error_stack(stderr, "%s: %s\n", temp, ptr = (char *) mysql_error(&mysql[1]));
	return(ptr);
}
#else
#include <varargs.h>
char           *
mysql_perror(va_alist)
	va_dcl
{
	va_list         args;
	char           *fmt, *ptr;
	char            temp[2048];

	va_start(args);
	fmt = va_arg(args, char *);
	if(vsnprintf(temp, 2048, fmt, args) == -1)
		temp[2047] = 0;
	va_end(args);
	error_stack(stderr, "%s: %s\n", temp, ptr = (char *) mysql_error(&mysql[1]));
	return(ptr);
}
#endif

void
getversion_mysql_perror_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
