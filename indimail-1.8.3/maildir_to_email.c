/*
 * $Log: maildir_to_email.c,v $
 * Revision 2.6  2009-02-18 21:29:24+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.5  2009-02-06 11:39:09+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.4  2005-12-29 22:46:29+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-07-02 18:08:34+05:30  Cprogrammer
 * renamed .domain to domain
 *
 * Revision 2.2  2004-06-20 16:38:01+05:30  Cprogrammer
 * rename ISOCOR_BASE_PATH to BASE_PATH
 * use .email in maildir to figure out email address
 *
 * Revision 2.1  2002-09-15 14:49:49+05:30  Cprogrammer
 * function for getting email from a maildir
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: maildir_to_email.c,v 2.6 2009-02-18 21:29:24+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <string.h>

char           *
maildir_to_email(char *maildir, char *domain)
{
	static char     email[AUTH_SIZE];
	char            tmpbuf[AUTH_SIZE];
	char           *ptr, *cptr, *base_path;
	FILE           *fp;

	snprintf(tmpbuf, AUTH_SIZE, "%s/email", maildir);
	if((fp = fopen(tmpbuf, "r")) != NULL)
	{
		if (fscanf(fp, "%s", email) == 1)
		{
			fclose(fp);
			return(email);
		}
		fclose(fp);
	}
	if (!(ptr = strstr(maildir, "/Maildir")))
		return ((char *) 0);
	for (ptr--; *ptr && ptr != maildir && *ptr != '/'; ptr--);
	if (ptr == maildir || !*ptr)
		return ((char *) 0);
	for (cptr = email, ptr++; *ptr && *ptr != '/';
		 *cptr++ = *ptr++);
	*cptr++ = '@';
	if(domain && *domain)
	{
		for(ptr = domain;*ptr;*cptr++ = *ptr++);
		*cptr = 0;
		return(email);
	} else
	{
		snprintf(tmpbuf, AUTH_SIZE, "%s/domain", maildir);
		if((fp = fopen(tmpbuf, "r")) != NULL)
		{
			if (fscanf(fp, "%s", cptr) == 1)
			{
				fclose(fp);
				return(email);
			}
			fclose(fp);
		}
	}
	getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	ptr = maildir + slen(base_path);
	for (ptr++; *ptr && *ptr != '/'; ptr++);
	for (ptr++; *ptr && *ptr != '/'; *cptr++ = *ptr++);
	*cptr++ = 0;
	return (email);
}

void
getversion_maildir_to_email_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
