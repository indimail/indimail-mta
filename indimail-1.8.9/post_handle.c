/*
 * $Log: post_handle.c,v $
 * Revision 2.3  2011-04-08 17:26:58+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.2  2010-02-16 13:30:15+05:30  Cprogrammer
 * return 0 if post_handle script does not exist
 *
 * Revision 2.1  2010-02-16 11:33:59+05:30  Cprogrammer
 * post handle routine for indimail
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: post_handle.c,v 2.3 2011-04-08 17:26:58+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
int
post_handle(const char *fmt, ...)
#else
#include <varargs.h>
int
post_handle(va_alist)
va_dcl
#endif
{
#ifndef HAVE_STDARG_H
	char           *fmt;
#endif
	va_list         ap;
	char           *ptr, *cptr;
	int             ret;

#ifndef HAVE_STDARG_H
	va_start(ap);
	fmt = va_arg(ap, char *);
#endif
	if (fmt && *fmt) {
#ifdef HAVE_STDARG_H
		va_start(ap, fmt);
#endif
		if (vasprintf(&ptr, fmt, ap) == -1) {
			fprintf(stderr, "post_handle: vasprintf: %s\n", strerror(errno));
			return (-1);
		}
		va_end(ap);
		if ((cptr = strchr(ptr, ' ')))
		{
			*cptr = 0;
			if (access(ptr, F_OK))
			{
				free(ptr);
				return (0);
			}
			*cptr = ' ';
		} else
		if (access(ptr, F_OK))
		{
			free(ptr);
			return (0);
		}
		ret = runcmmd(ptr, 0);
		free(ptr);
		return (ret);
	}
	return (0);
}

void
getversion_post_handle_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
