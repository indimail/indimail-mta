/*
 * $Log: unsetenv.c,v $
 * Revision 2.4  2011-04-08 17:27:22+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.3  2008-07-14 19:49:30+05:30  Cprogrammer
 * fixed empty symbols warning on Mac OS X
 *
 * Revision 2.2  2008-06-10 10:37:48+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.1  2004-09-04 00:01:24+05:30  Cprogrammer
 * unsetenv() function
 *
 */
#ifndef	lint
static char     sccsid[] = "$Id: unsetenv.c,v 2.4 2011-04-08 17:27:22+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if !defined(HAVE_SETENV) || !defined(HAVE_UNSETENV)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int      copied = 0;
static int      emalloc = 0;
extern char   **environ;
extern char    *xmalloc();
extern char    *xrealloc();

static void
copyenv(void)
{
	char          **x, **y;
	int             n;

	for (x = environ, n = 0; *x != NULL; x++)
		n++;
	n++;
	x = environ;

	emalloc = n + 10;
	y = environ = (char **) xmalloc(emalloc * sizeof(*environ));

	while (*x != NULL)
	{
		int             n = strlen(*x) + 1;
		*y = xmalloc(n);
		memcpy(*y, *x, n);
		x++, y++;
	}
	*y = NULL;

	copied = 1;
	return;
}
#endif

#ifndef HAVE_SETENV
int
setenv(const char *name, const char *value, int overwrite)
{
	char          **x;
	int             n, nlen;

	if (!copied)
		copyenv();

	if (strchr(name, '=') != NULL)
	{
		fprintf(stderr, "setenv: Illegal variable name %s", name);
		return -1;
	}

	nlen = strlen(name);

	x = environ;
	n = 0;
	while ((*x != NULL) && ((memcmp(*x, name, nlen) != 0) || (*(*x + nlen) != '=')))
		x++, n++;
	n++;

	if (*x == NULL)
	{
		if (emalloc < n)
		{
			emalloc = n + 10;
			x = environ = (char **) xrealloc(environ, emalloc * sizeof(*environ));
			while (*x != NULL)
				x++;
		}
		*x = xmalloc(nlen + strlen(value) + 2);
		strcpy(*x, name);
		*(*x + nlen) = '=';
		strcpy(*x + nlen + 1, value);
		*(++x) = NULL;
	} else
	if (overwrite)
	{
		int             vlen = strlen(value);
		free(*x);
		*x = xmalloc(nlen + vlen + 2);
		strcpy(*x, name);
		*(*x + nlen) = '=';
		strcpy(*x + nlen + 1, value);
	}
	return 0;
}
#endif

#ifndef HAVE_UNSETENV
void
unsetenv(const char *name)
{
	char          **x, **y;
	int             nlen;

	if (!copied)
		copyenv();
	if (strchr(name, '=') != NULL)
	{
		fprintf(stderr, "unsetenv: Illegal variable name %s", name);
		return;
	}
	nlen = strlen(name);
	x = environ;
	while ((*x != NULL) && ((memcmp(*x, name, nlen) != 0) || (*(*x + nlen) != '=')))
		x++;
	if (*x != NULL)
	{
		free(*x);
		y = x;
		x++;
		while (*y != NULL)
			*(y++) = *(x++);
	}
	return;
}
#endif

#include <stdio.h>
void
getversion_unsetenv_c()
{
	printf("%s\n", sccsid);
}
