/*
 * $Log: vset_lastdeliver.c,v $
 * Revision 2.10  2008-11-06 15:07:16+05:30  Cprogrammer
 * fix for mysql_real_escape_string()
 *
 * Revision 2.9  2008-09-08 09:58:56+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.8  2008-06-13 11:02:51+05:30  Cprogrammer
 * compile insert entries into userquota only if ENABLE_AUTH_LOGGING defined
 *
 * Revision 2.7  2008-05-28 17:42:38+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.6  2008-05-28 15:35:05+05:30  Cprogrammer
 * removed ldap code
 *
 * Revision 2.5  2003-05-30 00:02:20+05:30  Cprogrammer
 * bypass ldap to prevent wrong setting of pw_gid
 *
 * Revision 2.4  2002-10-27 21:44:11+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-05 01:18:55+05:30  Cprogrammer
 * added mysql_escape for user
 *
 * Revision 2.2  2002-08-03 04:39:46+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-07-03 01:27:58+05:30  Cprogrammer
 * copy passwd structure to prevent overwriting static location returned by vauth_getpw()
 *
 * Revision 1.7  2002-02-23 20:27:18+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.6  2001-12-22 18:18:58+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.5  2001-12-08 00:38:13+05:30  Cprogrammer
 * erroneous message correction
 *
 * Revision 1.4  2001-11-24 12:22:30+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:16+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:39+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#ifdef ENABLE_AUTH_LOGGING
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vset_lastdeliver.c,v 2.10 2008-11-06 15:07:16+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vset_lastdeliver(char *user, char *domain, int quota)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
#ifdef ENABLE_AUTH_LOGGING
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;
#endif

#ifdef ENABLE_AUTH_LOGGING
	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	snprintf(SqlBuf, SQL_BUF_SIZE, "replace delayed into userquota set user=\"%s\", \
		domain=\"%s\",  quota=%d, \
		timestamp=FROM_UNIXTIME(%lu)", 
		user, domain, quota, time(0));
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
				return(1);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("vset_lastdeliver: %s", SqlBuf);
				return(1);
			}
		} else
		{
			mysql_perror("vset_lastdeliver: %s", SqlBuf);
			return(1);
		}
	}
#endif
	if((pw = vauth_getpw(user, domain)))
	{
		int gid;

		gid = pw->pw_gid;
		PwTmp = *pw;
		pw = &PwTmp;
		if(!quota  && (pw->pw_gid & BOUNCE_MAIL))
			pw->pw_gid = 0;
		else
		if(quota)
			pw->pw_gid |= BOUNCE_MAIL;
		if(pw->pw_gid != gid)
			return(vauth_setpw(pw, domain));
	}
	return (0);
}

void
getversion_vset_lastdeliver_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
