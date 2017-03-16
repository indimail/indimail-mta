/*
 * $Log: passwd.c,v $
 * Revision 2.8  2010-05-01 14:13:11+05:30  Cprogrammer
 * close database before return
 *
 * Revision 2.7  2008-08-02 09:08:21+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.6  2008-06-24 21:49:00+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.5  2008-06-13 09:53:19+05:30  Cprogrammer
 * set passwd change in lastauth table only if ENABLE_AUTH_LOGGING is defined
 *
 * Revision 2.4  2008-05-28 15:13:37+05:30  Cprogrammer
 * removed sqwebmail, ldap
 *
 * Revision 2.3  2003-10-24 23:14:07+05:30  Cprogrammer
 * added code to override ldap while setting password and prevent
 * clobbering of pw_gid field
 *
 * Revision 2.2  2002-08-28 20:36:22+05:30  Cprogrammer
 * is_open was set to 0 incorrectly
 *
 * Revision 2.1  2002-07-04 22:13:38+05:30  Cprogrammer
 * code to centrally update user passwd for a distributed domain incorporated
 *
 * Revision 1.5  2002-02-24 04:11:49+05:30  Cprogrammer
 * added code for MAILDROP Maildir quota
 *
 * Revision 1.4  2001-12-08 00:34:45+05:30  Cprogrammer
 * set entry pass in lastauth for password changes
 *
 * Revision 1.3  2001-11-24 12:19:49+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:44+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: passwd.c,v 2.8 2010-05-01 14:13:11+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * update a users virtual password file entry with a different password
 */
int
vpasswd(char *username, char *domain, char *password,
		int apop)
{
	struct passwd  *pw;
	int             i;
	char            Dir[MAX_BUFF], Crypted[MAX_BUFF], email[MAX_BUFF];
	mdir_t          quota;

	lowerit(username);
	lowerit(domain);
	snprintf(email, MAX_BUFF, "%s@%s", username, domain);
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 0))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound)
			return(0);
		else
			return(-1);
	}
	if (!(pw = vauth_getpw(username, domain)))
		return (0);
	if (pw->pw_gid & NO_PASSWD_CHNG)
	{
		error_stack(stderr, "User not allowed to change passwd\n");
		return (-1);
	}
	if (apop & USE_APOP)
	{
		i = vauth_vpasswd (username, domain, password, apop);
		vclose();
		return(i);
	} else
	{
		mkpasswd3(password, Crypted, MAX_BUFF);
		if ((i = vauth_vpasswd(username, domain, Crypted, apop)) == 1)
		{
			snprintf(Dir, MAX_BUFF, "%s/Maildir", pw->pw_dir);
			if (access(Dir, F_OK))
				quota = 0l;
			else
			{
#ifdef USE_MAILDIRQUOTA
				quota = check_quota(Dir, 0);
#else
				quota = check_quota(Dir);
#endif
			}
#ifdef ENABLE_AUTH_LOGGING
			vset_lastauth(username, domain, "pass", GetIpaddr(), pw->pw_gecos, quota);
#endif
		}
		vclose();
		return(i);
	}
}

void
getversion_passwd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
