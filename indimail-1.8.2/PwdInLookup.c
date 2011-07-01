/*
 * $Log: PwdInLookup.c,v $
 * Revision 2.5  2010-05-01 14:13:28+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.4  2008-05-28 16:37:29+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2005-12-29 22:48:57+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2002-08-31 15:57:16+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 2.1  2002-07-04 00:32:16+05:30  Cprogrammer
 * is_open to be reset only for distributed code
 *
 * Revision 1.1  2002-04-09 14:37:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: PwdInLookup.c,v 2.5 2010-05-01 14:13:28+05:30 Cprogrammer Stab mbhangui $";
#endif

struct passwd *
PwdInLookup(char *email)
{
	char            user[MAX_BUFF], domain[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
	struct passwd  *pw;

#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 1))
#else
	if (vauth_open((char *) 0))
#endif
		return((struct passwd *) 0);
	for(cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for(cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	pw = vauth_getpw(user, real_domain);
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return(pw);
}

void
getversion_PwdInLookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
