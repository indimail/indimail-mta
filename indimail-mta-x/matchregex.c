/*
 * $Log: matchregex.c,v $
 * Revision 1.5  2021-08-21 18:50:43+05:30  Cprogrammer
 * handle regexec error
 *
 * Revision 1.4  2020-09-16 19:02:25+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.3  2019-03-07 00:52:18+05:30  Cprogrammer
 * added regcomp error cases for documentation
 *
 * Revision 1.2  2015-07-23 15:01:42+05:30  Cprogrammer
 * fixed comparision of constant '-1' with 0
 *
 * Revision 1.1  2009-05-01 10:34:29+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <regex.h>
#include <unistd.h>
#include "stralloc.h"
#include "error.h"
#include "qregex.h"

#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)

int
matchregex(char *text, char *regex, char **errStr)
{
	regex_t         qreg;
	char            errbuf[512];
	int             retval = 0;
	static stralloc err_str = { 0 };

	if (errStr)
		*errStr = 0;
	/*- build the regex */
	if ((retval = regcomp(&qreg, regex, REG_EXTENDED|REG_ICASE)) != 0) {
		regerror(retval, &qreg, errbuf, sizeof(errbuf));
		regfree(&qreg);
		if (!stralloc_copys(&err_str, text) ||
				!stralloc_cats(&err_str, ": ") ||
				!stralloc_cats(&err_str, regex) ||
				!stralloc_cats(&err_str, ": ") ||
				!stralloc_cats(&err_str, errbuf) ||
				!stralloc_0(&err_str))
			return (AM_MEMORY_ERR);
		if (errStr)
			*errStr = err_str.s;
		switch (retval)
		{
			case REG_BADBR:
			case REG_BADPAT:
			case REG_BADRPT:
			case REG_EBRACE:
			case REG_EBRACK:
			case REG_ECOLLATE:
			case REG_ECTYPE:
#ifdef REG_EEND
			case REG_EEND:
#endif
			case REG_EESCAPE:
			case REG_EPAREN:
			case REG_ERANGE:
#ifdef REG_ESIZE
			case REG_ESIZE:
#endif
			case REG_ESPACE:
			case REG_ESUBREG:
			default:
				return (AM_REGEX_ERR);
		}
	}
	/*- execute the regex */
	if ((retval = regexec(&qreg, text, (size_t) NULL, (regmatch_t *) NULL, 0)) == -1) {
		if (!stralloc_copys(&err_str, text) ||
				!stralloc_cats(&err_str, ": ") ||
				!stralloc_cats(&err_str, regex) ||
				!stralloc_cats(&err_str, ": ") ||
				!stralloc_cats(&err_str, errbuf) ||
				!stralloc_0(&err_str))
			return (AM_MEMORY_ERR);
		if (errStr)
			*errStr = err_str.s;
		return -1;
	}
	regfree(&qreg);
	return (retval == REG_NOMATCH ? 0 : 1);
}

void
getversion_matchregex_c()
{
	static char    *x = "$Id: matchregex.c,v 1.5 2021-08-21 18:50:43+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
