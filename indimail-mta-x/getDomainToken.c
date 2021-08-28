/*
 * $Log: getDomainToken.c,v $
 * Revision 1.3  2021-08-28 23:05:10+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.2  2021-05-29 23:37:58+05:30  Cprogrammer
 * refactored for slowq-send
 *
 * Revision 1.1  2021-05-26 05:20:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <str.h>
#include <regex.h>
#include <stralloc.h>
#include <env.h>
#include <regex.h>
#include "wildmat.h"
#include "getDomainToken.h"

#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)
#ifndef REG_NOERROR
#define REG_NOERROR 0
#endif

dtype           delivery;

/*
 * read control file filterargs which can be one of the three
 * 1. domain:local:command with args
 * 2. domain:remote:command with args
 * 3. domain:command with args
 * here domain can be a wildcard or a regular expression if
 * QREGEX environment variable is defined
 */
char           *
getDomainToken(char *domain, stralloc *sa)
{
	regex_t         qreg;
	int             len, n, retval;
	char           *ptr, *p;

	for (len = 0, ptr = sa->s;len < sa->len;) {
		len += ((n = str_len(ptr)) + 1);
		for (p = ptr;*p && *p != ':';p++);
		if (*p == ':') {
			*p = 0;
			/*- build the regex */
			if ((retval = str_diff(ptr, domain))) {
				if (env_get("QREGEX")) {
					if ((retval = REGCOMP(qreg, ptr)) == 0)
						retval = (REGEXEC(qreg, domain) == REG_NOMATCH ? 1 : REG_NOERROR);
					regfree(&qreg);
				} else
					retval = !wildmat_internal(domain, ptr);
			}
			*p = ':';
			if (!retval) { /*- match occurred for domain or wildcard */
				/* check for local/remote directives */
				if (delivery == remote_delivery) { /*- domain:remote:command - remote delivery */
					if (!str_diffn(p + 1, "remote:", 7))
						return (p + 8);
					if (!str_diffn(p + 1, "local:", 6)) {
						ptr = sa->s + len;
						continue; /*- skip local directives for remote mails */
					}
				} else
				if (delivery == local_delivery) { /*- domain:local:command - local delivery */
					if (!str_diffn(p + 1, "local:", 6))
						return (p + 7);
					if (!str_diffn(p + 1, "remote:", 7)) {
						ptr = sa->s + len;
						continue; /*- skip remote directives for local mails */
					}
				} if (delivery == local_or_remote) { /*- local/remote delivery */
					if (!str_diffn(p + 1, "local:", 6))
						return (p + 7);
					if (!str_diffn(p + 1, "remote:", 7))
						return (p + 8);
				}
				return (p + 1); /*- domain:command */
			}
		}
		ptr = sa->s + len;
	} /*- for (len = 0, ptr = sa->s;len < sa->len;) */
	return ((char *) 0);
}

void
getversion_getdomaintoke_c()
{
	static char    *x = "$Id: getDomainToken.c,v 1.3 2021-08-28 23:05:10+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidwildmath;
	x = sccsidgetdomainth;
	x++;
}
