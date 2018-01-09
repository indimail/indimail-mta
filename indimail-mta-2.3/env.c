/*
 * $Log: env.c,v $
 * Revision 1.5  2009-05-03 22:42:42+05:30  Cprogrammer
 * simplified restore_env()
 *
 * Revision 1.4  2004-10-22 20:24:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-09-25 23:59:37+05:30  Cprogrammer
 * BUG fix. set env_isnit to zero in restore_env()
 *
 * Revision 1.2  2004-07-17 21:18:30+05:30  Cprogrammer
 * added RCS log
 *
 *
 * env.c, envread.c, env.h: environ library
 * Daniel J. Bernstein, djb@silverton.berkeley.edu.
 * Depends on str.h, alloc.h.
 * Requires environ.
 * 19960113: rewrite. warning: interface is different.
 * No known patent problems.
 */

#include "str.h"
#include "alloc.h"
#include "env.h"

int             env_isinit = 0;	/*- if env_isinit: */
static int      ea;				/*- environ is a pointer to ea+1 char*'s.  */
static int      en;				/*- the first en of those are ALLOCATED. environ[en] is 0.  */
static char   **origenv;

static void
env_goodbye(i)
	int             i;
{
	alloc_free(environ[i]);
	environ[i] = environ[--en];
	environ[en] = 0;
}

static char    *null = 0;

void
env_clear()
{
	if (env_isinit)
	{
		while (en)
			env_goodbye(0);
		alloc_free((char *) environ);
		ea = en = env_isinit = 0;
	} else
		environ = &null;
}

static void
env_unsetlen(s, len)
	char           *s;
	int             len;
{
	int             i;

	for (i = en - 1; i >= 0; --i)
	{
		if (!str_diffn(s, environ[i], len) && environ[i][len] == '=')
			env_goodbye(i);
	}
}

int
env_unset(s)
	char           *s;
{
	if (!env_isinit && !env_init())
		return 0;
	env_unsetlen(s, str_len(s));
	return 1;
}

static int
env_add(s)
	char           *s;
{
	char           *t;

	if ((t = env_findeq(s)))
		env_unsetlen(s, t - s);
	if (en == ea)
	{
		ea += 30;
		if (!alloc_re((char *) &environ, (en + 1) * sizeof(char *), (ea + 1) * sizeof(char *)))
		{
			ea = en;
			return 0;
		}
	}
	environ[en++] = s;
	environ[en] = 0;
	return 1;
}

int
env_put(s)
	char           *s;
{
	char           *u;

	if (!env_isinit && !env_init())
		return 0;
	if (!(u = alloc(str_len(s) + 1)))
		return 0;
	str_copy(u, s);
	if (!env_add(u))
	{
		alloc_free(u);
		return 0;
	}
	return 1;
}

int
env_put2(s, t)
	char           *s;
	char           *t;
{
	char           *u;
	int             slen;

	if (!env_isinit && !env_init())
		return 0;
	slen = str_len(s);
	if (!(u = alloc(slen + str_len(t) + 2)))
		return 0;
	str_copy(u, s);
	u[slen] = '=';
	str_copy(u + slen + 1, t);
	if (!env_add(u))
	{
		alloc_free(u);
		return 0;
	}
	return 1;
}

int
env_init()
{
	char          **newenviron;
	int             i;

	for (en = 0; environ[en]; ++en);
	ea = en + 10;
	if (!(newenviron = (char **) alloc((ea + 1) * sizeof(char *))))
		return 0;
	for (en = 0; environ[en]; ++en)
	{
		if (!(newenviron[en] = alloc(str_len(environ[en]) + 1)))
		{
			for (i = 0; i < en; ++i)
				alloc_free(newenviron[i]);
			alloc_free((char *) newenviron);
			return 0;
		}
		str_copy(newenviron[en], environ[en]);
	}
	newenviron[en] = 0;
	if (!origenv)
		origenv = environ; /*- the pristine environment */
	environ = newenviron;
	env_isinit = 1;
	return 1;
}

void
restore_env()
{
	if (origenv)
	{
		env_clear();
		environ = origenv;
		origenv = 0;
	}
	return;
}

void
getversion_env_c()
{
	static char    *x = "$Id: env.c,v 1.5 2009-05-03 22:42:42+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
