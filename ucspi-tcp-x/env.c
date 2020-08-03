/*
 * $Log: env.c,v $
 * Revision 1.2  2020-08-03 17:23:37+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2003-10-21 11:21:08+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <str.h>
#include <env.h>

extern /*- @null@ */
char       *
env_get(char *s)
{
	int             i;
	unsigned int    len;

	if (!s)
		return 0;
	len = str_len(s);
	for (i = 0; environ[i]; ++i) {
		if (str_start(environ[i], s) && (environ[i][len] == '='))
			return environ[i] + len + 1;
	}
	return 0;
}
