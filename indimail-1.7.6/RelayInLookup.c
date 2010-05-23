/*
 * $Log: RelayInLookup.c,v $
 * Revision 2.6  2010-05-01 14:13:51+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.5  2008-06-13 09:58:41+05:30  Cprogrammer
 * return -2 if POP_AUTH_OPEN_RELAY not defined
 *
 * Revision 2.4  2008-05-28 16:37:32+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2005-12-29 22:49:00+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2002-08-31 15:57:21+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 2.1  2002-07-04 00:32:43+05:30  Cprogrammer
 * is_open to be reset only for distributed code
 *
 * Revision 1.1  2002-04-09 14:37:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: RelayInLookup.c,v 2.6 2010-05-01 14:13:51+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 *  0: User not authenticated (no entry in relay table)
 *  1: User Authenticated.
 * -1: System Error
 */
#ifdef POP_AUTH_OPEN_RELAY
int
RelayInLookup(char *mailfrom, char *remoteip)
{
	char            user[MAX_BUFF], domain[MAX_BUFF], email[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
	int             retval;

#ifdef CLUSTERED_SITE
	if (vauthOpen_user(mailfrom, 1))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound)
			return(0);
		else
			return(-1);
	}
	for(cptr = user, ptr = mailfrom;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for(cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(email, MAX_BUFF, "%s@%s", user, real_domain);
	retval = relay_select(email, remoteip);
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return(retval);
}
#else
int
RelayInLookup(char *mailfrom, char *remoteip)
{
	return(-2);
}
#endif

void
getversion_RelayInLookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
