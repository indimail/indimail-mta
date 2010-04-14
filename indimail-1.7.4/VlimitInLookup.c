/*
 * $Log: $
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: UserInLookup.c,v 2.16 2010-03-07 09:27:27+05:30 Cprogrammer Stab mbhangui $";
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
	if (vauthOpen_user(email))
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
