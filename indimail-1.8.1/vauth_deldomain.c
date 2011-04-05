/*
 * $Log: vauth_deldomain.c,v $
 * Revision 2.7  2010-02-19 19:34:37+05:30  Cprogrammer
 * delete from smtp_port only if domain is distributed
 *
 * Revision 2.6  2008-05-28 16:38:54+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2008-05-28 15:23:11+05:30  Cprogrammer
 * removed ldap, cdb modules
 *
 * Revision 2.4  2004-05-17 00:51:49+05:30  Cprogrammer
 * force flag argument to delusercntrl()
 *
 * Revision 2.3  2002-08-11 00:30:20+05:30  Cprogrammer
 * continue with deletion of vauth_updateflag() or delusercntrl() fails
 *
 * Revision 2.2  2002-08-03 04:34:30+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-06-26 03:19:37+05:30  Cprogrammer
 * VLDAP_BASEDN changed to LDAP_BASEDN
 *
 * Revision 1.11  2001-12-14 10:34:21+05:30  Cprogrammer
 * return error if table does not exist
 *
 * Revision 1.10  2001-12-10 00:16:25+05:30  Cprogrammer
 * remove entries from smtp_port during domain deletions
 *
 * Revision 1.9  2001-12-08 23:53:29+05:30  Cprogrammer
 * code for deletion from inactive table
 *
 * Revision 1.8  2001-12-08 00:39:34+05:30  Cprogrammer
 * changed delete to delete low_priority
 *
 * Revision 1.7  2001-11-29 13:21:01+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.6  2001-11-29 00:43:57+05:30  Cprogrammer
 * delete users from central db when local users are deleted
 *
 * Revision 1.5  2001-11-28 23:03:26+05:30  Cprogrammer
 * vdelfiles() interface change
 *
 * Revision 1.4  2001-11-24 12:20:54+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:50+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:26:45+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_deldomain.c,v 2.7 2010-02-19 19:34:37+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

int
vauth_deldomain(char *domain)
{
	char           *tmpstr;
	struct passwd  *pw;
	int             is_dist, err;
    char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (1);
	if ((is_dist = is_distributed_domain(domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n",domain);
		return(1);
	}
	for (err = 0, pw = vauth_getall(domain, 1, 0); pw; pw = vauth_getall(domain, 0, 0))
	{
		if (verbose)
			printf("Removing user %s\n", pw->pw_name);
		vdelfiles(pw->pw_dir, pw->pw_name, domain);
#ifdef CLUSTERED_SITE
		if (is_dist)
		{
			if (vauth_updateflag(pw->pw_name, domain, DEL_FLAG) || delusercntrl(pw->pw_name, domain, 0))
				continue;
		}
#endif
	}
	if (site_size == LARGE_SITE)
	{
		tmpstr = vauth_munch_domain(domain);
		snprintf(SqlBuf, SQL_BUF_SIZE, "drop table %s", tmpstr);
	}
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_domain = \"%s\"", default_table, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_deldomain: %s", SqlBuf);
		err = (err ? 1 : 0);
	}
	if (site_size == SMALL_SITE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_domain = \"%s\"", inactive_table, domain);
		if (mysql_query(&mysql[1], SqlBuf) && mysql_errno(&mysql[1]) != ER_NO_SUCH_TABLE)
		{
			mysql_perror("vauth_deldomain: %s", SqlBuf);
			err = (err ? 1 : 0);
		}
	}
#ifdef ENABLE_AUTH_LOGGING
	if (snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from lastauth where domain = \"%s\"", domain) == -1)
		SqlBuf[SQL_BUF_SIZE - 1] = 0;
	if (mysql_query(&mysql[1], SqlBuf) && mysql_errno(&mysql[1]) != ER_NO_SUCH_TABLE)
	{
		mysql_perror("vauth_deldomain: %s", SqlBuf);
		err = (err ? 1 : 0);
	}
#endif
#ifdef VALIAS
	err = (valias_delete_domain(domain) ? 1 : err);
#endif
#ifdef CLUSTERED_SITE
	if (is_dist)
		err = (vsmtp_delete_domain(domain) ? 1 : err);
#endif
	return (err);
}

void
getversion_vauth_deldomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
