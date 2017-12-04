/*
 * $Log: vsmtp_delete_domain.c,v $
 * Revision 2.4  2008-05-28 17:42:46+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2003-07-02 18:42:34+05:30  Cprogrammer
 * return success if table is not present
 *
 * Revision 2.2  2002-10-27 21:44:57+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-07 18:40:24+05:30  Cprogrammer
 * no point in deleting if table does not exist
 *
 * Revision 1.5  2002-08-03 04:40:08+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.4  2002-02-23 20:27:28+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.3  2001-12-22 18:19:06+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.2  2001-12-11 11:36:03+05:30  Cprogrammer
 * open master for updates
 *
 * Revision 1.1  2001-12-09 23:49:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_delete_domain.c,v 2.4 2008-05-28 17:42:46+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>
int
vsmtp_delete_domain(char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vsmtp_delete_domain: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from smtp_port where domain = \"%s\"", domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			return (0);
		}
		fprintf(stderr, "vsmtp_delete_domain: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vsmtp_delete_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
