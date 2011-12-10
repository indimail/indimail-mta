/*
 * $Log: AliasInLookup.c,v $
 * Revision 2.7  2010-05-01 14:10:26+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user()
 *
 * Revision 2.6  2008-06-13 08:34:59+05:30  Cprogrammer
 * return NULL result if VALIAS is not defined
 *
 * Revision 2.5  2008-05-28 16:33:33+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2005-12-29 22:39:12+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2002-12-27 16:39:34+05:30  Cprogrammer
 * improvized memory allocation logic
 *
 * Revision 2.2  2002-08-31 15:56:38+05:30  Cprogrammer
 * take domain as the DEFAULT_DOMAIN if email does not contain domain
 *
 * Revision 2.1  2002-07-04 00:29:38+05:30  Cprogrammer
 * incorrect checking for malloc() return value
 *
 * Revision 1.1  2002-04-09 14:37:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: AliasInLookup.c,v 2.7 2010-05-01 14:10:26+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <stdlib.h>
#include <errno.h>
#include <string.h>
char *
AliasInLookup(char *email)
{
	char            user[MAX_BUFF], domain[MAX_BUFF];
	char           *ptr, *cptr, *real_domain;
	static char    *memptr;
	int             len, tmplen;

	if (!email)
	{
		if (memptr)
		{
			free(memptr);
			memptr = (char *) 0;
		}
		return((char *) 0);
	}
	for(cptr = user, ptr = email;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for(cptr = domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 1))
#else
	if (vauth_open((char *) 0))
#endif
		return((char *) 0);
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	/* Allocate one byte for NULL */
	if (!memptr && !(memptr = (char *) malloc(1 * sizeof(char))))
	{
		fprintf(stderr, "malloc: 1 bytes: %s\n", strerror(errno));
#ifdef CLUSTERED_SITE
		is_open = 0;
#endif
		return((char *) 0);
	}
	for(*memptr = 0, len = 1;;)
	{
		if (!(cptr = valias_select(user, real_domain)))
			break;
		tmplen = slen(cptr) + 1;
		len += tmplen;
		if (!(memptr = (char *) realloc(memptr, len)))
		{
			fprintf(stderr, "realloc: %d bytes: %s\n", len, strerror(errno));
#ifdef CLUSTERED_SITE
			is_open = 0;
#endif
			return((char *) 0);
		}
		scat(memptr, cptr, len);
		scat(memptr, "\n", len);
	}
#ifdef CLUSTERED_SITE
	is_open = 0;
#endif
	return(memptr);
}
#else
char *
AliasInLookup(char *email)
{
	return((char *) 0);
}
#endif

void
getversion_AliasInLookup_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
