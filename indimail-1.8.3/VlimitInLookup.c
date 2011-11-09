/*
 * $Log: VlimitInLookup.c,v $
 * Revision 2.2  2010-05-01 14:15:04+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.1  2010-04-15 22:58:32+05:30  Cprogrammer
 * InLookup function to fetch vlimit
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: VlimitInLookup.c,v 2.2 2010-05-01 14:15:04+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>

#ifdef ENABLE_DOMAIN_LIMITS
int
VlimitInLookup(char *email, struct vlimits *lim)
{
	char            user[MAX_BUFF], domain[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
#ifdef ENABLE_DOMAIN_LIMITS
	struct vlimits  limits;
#endif

#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 1))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if(userNotFound)
			return(1);
		else
			return(-1);
	}
	for (cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	if (vget_limits(real_domain, &limits))
	{
		fprintf(stderr, "%s: failed to get domain limits\n", real_domain);
		return (-1);
	}
	*lim = limits;
	return (0);
}
#endif

void
getversion_VlimitInLookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
