/*
 * $Log: vset_lastauth.c,v $
 * Revision 2.6  2008-09-08 09:58:50+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 15:34:56+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.4  2002-10-27 21:43:53+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-05 01:18:01+05:30  Cprogrammer
 * added mysql_escape for user
 *
 * Revision 2.2  2002-08-03 04:39:41+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-07-27 01:01:28+05:30  Cprogrammer
 * removed vlogauth()
 *
 * Revision 1.7  2002-02-23 20:27:08+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.6  2001-12-22 18:18:36+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.5  2001-12-08 00:38:06+05:30  Cprogrammer
 * formatting changes
 *
 * Revision 1.4  2001-11-24 12:22:29+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:14+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:35+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <time.h>

#ifndef	lint
static char     sccsid[] = "$Id: vset_lastauth.c,v 2.6 2008-09-08 09:58:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
vset_lastauth(char *user, char *domain, char *service, char *remoteip, char *gecos, int quota)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if ((err = vauth_open((char *) 0)) != 0)
		return (err);
	snprintf(SqlBuf, SQL_BUF_SIZE, "replace delayed into lastauth set user=\"%s\", \
		domain=\"%s\",  service=\"%s\", remote_ip=\"%s\", quota=%d, gecos=\"%s\", \
		timestamp=FROM_UNIXTIME(%lu)", user, domain, service, remoteip, quota, gecos, time(0));
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT))
				return(1);
			if (!mysql_query(&mysql[1], SqlBuf))
				return(0);
		}
		mysql_perror("vset_lastauth: %s", SqlBuf);
		return(1);
	}
	return (0);
}
#endif /*- #ifdef ENABLE_AUTH_LOGGING */

void
getversion_vset_lastauth_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
