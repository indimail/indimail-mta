/*
 * $Log: pathexec_env.c,v $
 * Revision 1.3  2017-04-22 11:53:05+05:30  Cprogrammer
 * added pathexec_dl() function
 *
 * Revision 1.2  2003-12-30 00:32:36+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.1  2003-10-21 11:23:08+05:30  Cprogrammer
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

#ifdef LOAD_SHARED_OBJECTS
void
pathexec_dl(int argc, char **argv, char **envp, int (*func) (int, char **, char **))
{
	char          **e;
	unsigned int    elen;
	unsigned int    i;
	unsigned int    j;
	unsigned int    split;
	unsigned int    t;

	if (!stralloc_cats(&plus, ""))
		return;
	elen = 0;
	for (i = 0; envp[i]; ++i)
		++elen;
	for (i = 0; i < plus.len; ++i)
	{
		if (!plus.s[i])
			++elen;
	}
	if(!(e = (char **) alloc((elen + 1) * sizeof(char *))))
		return;
	elen = 0;
	for (i = 0; envp[i]; ++i)
		e[elen++] = envp[i];
	j = 0;
	for (i = 0; i < plus.len; ++i)
	{
		if (!plus.s[i])
		{
			split = str_chr(plus.s + j, '=');
			for (t = 0; t < elen; ++t)
			{
				if (byte_equal(plus.s + j, split, e[t]))
				{
					if (e[t][split] == '=')
					{
						--elen;
						e[t] = e[elen];
						break;
					}
				}
			}
			if (plus.s[j + split])
				e[elen++] = plus.s + j;
			j = i + 1;
		}
	}
	e[elen] = 0;
	(*func) (argc, argv, e); /*- execute the function */
	alloc_free(e);
}
#endif

void
pathexec(char **argv)
{
	char          **e;
	unsigned int    elen;
	unsigned int    i;
	unsigned int    j;
	unsigned int    split;
	unsigned int    t;

	if (!stralloc_cats(&plus, ""))
		return;

	elen = 0;
	for (i = 0; environ[i]; ++i)
		++elen;
	for (i = 0; i < plus.len; ++i)
	{
		if (!plus.s[i])
			++elen;
	}
	if(!(e = (char **) alloc((elen + 1) * sizeof(char *))))
		return;
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
				if (byte_equal(plus.s + j, split, e[t]))
				{
					if (e[t][split] == '=')
					{
						--elen;
						e[t] = e[elen];
						break;
					}
				}
			}
			if (plus.s[j + split])
				e[elen++] = plus.s + j;
			j = i + 1;
		}
	}
	e[elen] = 0;
	pathexec_run(*argv, argv, e);
	alloc_free(e);
}
