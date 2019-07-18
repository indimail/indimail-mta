/*
 * $Log: pathexec_env.c,v $
 * Revision 1.4  2011-05-07 15:58:21+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2010-06-08 21:59:53+05:30  Cprogrammer
 * return allocated environment variable
 *
 * Revision 1.2  2004-10-22 20:27:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:57:35+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "alloc.h"
#include "str.h"
#include "byte.h"
#include "env.h"
#include "pathexec.h"

static stralloc plus;
static stralloc tmp;

int
pathexec_env(char *s, char *t)
{
	if (!s)
		return 1;
	if (!stralloc_copys(&tmp, s))
		return 0;
	if (t)
	{
		if (!stralloc_cats(&tmp, "="))
			return 0;
		if (!stralloc_cats(&tmp, t))
			return 0;
	}
	if (!stralloc_0(&tmp))
		return 0;
	return stralloc_cat(&plus, &tmp);
}

char **
pathexec(char **argv)
{
	char          **e;
	unsigned int    elen;
	unsigned int    i;
	unsigned int    j;
	unsigned int    split;
	unsigned int    t;

	if (!stralloc_cats(&plus, ""))
		return ((char **) 0);
	elen = 0;
	for (i = 0; environ[i]; ++i)
		++elen;
	for (i = 0; i < plus.len; ++i)
		if (!plus.s[i])
			++elen;
	if (!(e = (char **) alloc((elen + 1) * sizeof(char *))))
		return ((char **) 0);
	elen = 0;
	for (i = 0; environ[i]; ++i)
		e[elen++] = environ[i];
	j = 0;
	for (i = 0; i < plus.len; ++i)
	{
		if (!plus.s[i])
		{
			split = str_chr(plus.s + j, '=');
			for (t = 0; t < elen; ++t)
			{
				if (byte_equal(plus.s + j, split, e[t]) && e[t][split] == '=')
				{
					--elen;
					e[t] = e[elen];
					break;
				}
			}
			if (plus.s[j + split])
				e[elen++] = plus.s + j;
			j = i + 1;
		}
	}
	e[elen] = 0;
	if (argv)
		pathexec_run(*argv, argv, e);
	return (e);
}

void
getversion_pathexec_env_c()
{
	static char    *x = "$Id: pathexec_env.c,v 1.4 2011-05-07 15:58:21+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
