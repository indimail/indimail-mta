/*
 * $Log: post_hook.c,v $
 * Revision 2.2  2010-02-16 13:30:15+05:30  Cprogrammer
 * return 0 if post_hook script does not exist
 *
 * Revision 2.1  2010-02-16 11:33:59+05:30  Cprogrammer
 * post hook routine for indimail
 *
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "config.h"
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: post_hook.c,v 2.2 2010-02-16 13:30:15+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
int
post_hook(const char *fmt, ...)
#else
#include <varargs.h>
int
post_hook(va_alist)
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
			fprintf(stderr, "post_hook: vasprintf: %s\n", strerror(errno));
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
getversion_post_hook_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
