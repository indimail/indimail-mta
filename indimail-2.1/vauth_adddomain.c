/*
 * $Log: vauth_adddomain.c,v $
 * Revision 2.5  2016-01-19 00:34:20+05:30  Cprogrammer
 * missing call to create table indimail
 *
 * Revision 2.4  2008-05-28 16:38:47+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2008-05-28 15:22:40+05:30  Cprogrammer
 * removed cdb, ldap modules
 *
 * Revision 2.2  2002-06-26 03:19:11+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 2.1  2002-06-21 20:38:23+05:30  Cprogrammer
 * create inactive auth table if not exist
 *
 * Revision 1.5  2001-12-08 12:37:35+05:30  Cprogrammer
 * added clause IF NOT EXISTS
 *
 * Revision 1.4  2001-11-24 12:20:51+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:47+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:26:34+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_adddomain.c,v 2.5 2016-01-19 00:34:20+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vauth_adddomain(char *domain)
{
	char           *tmpstr;
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if ((err = vauth_open((char *) 0)))
		return (err);
	if (site_size == LARGE_SITE)
	{
		if (!domain || !*domain)
			tmpstr = MYSQL_LARGE_USERS_TABLE;
		else
			tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE %s ( %s )", tmpstr, LARGE_TABLE_LAYOUT);
	} else
		snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE IF NOT EXISTS %s ( %s )", default_table, SMALL_TABLE_LAYOUT);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_adddomain: %s", SqlBuf);
		return (1);
	}
	if (site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE IF NOT EXISTS %s ( %s )", inactive_table, SMALL_TABLE_LAYOUT);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_adddomain: %s", SqlBuf);
			return (1);
		}
	}
	return (0);
}

void
getversion_vauth_adddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
