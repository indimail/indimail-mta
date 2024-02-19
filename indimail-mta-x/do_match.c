/*
 * $Id: do_match.c,v 1.1 2024-02-19 20:27:34+05:30 Cprogrammer Exp mbhangui $
 */

#include <matchregex.h>
#include <env.h>
#include "do_match.h"
#include "wildmat.h"
#include <fnmatch.h>

int
do_match(int use_regex, char *text, char *regex, char **errStr)
{
	int             i;

	if (errStr)
		*errStr = 0;
	if (use_regex)
		return (matchregex(text, regex, errStr));
	else
	if (env_get("USE_FNMATCH")) {
#ifdef FNM_CASEFOLD
		i = FNM_PATHNAME|FNM_CASEFOLD;
#else
		i = FNM_PATHNAME;
#endif
		switch(fnmatch(regex, text, i))
		{
		case 0:
			return (1);
		case FNM_NOMATCH:
			return (0);
		default:
			return (AM_REGEX_ERR);
		}
	} else
		return (wildmat_internal(text, regex));
}

void
getversion_do_match_c()
{
	static char    *x = "$Id: do_match.c,v 1.1 2024-02-19 20:27:34+05:30 Cprogrammer Exp mbhangui $";
	x = sccsidwildmath;
	x++;
}
