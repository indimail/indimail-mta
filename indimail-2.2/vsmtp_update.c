/*
 * $Log: vsmtp_update.c,v $
 * Revision 2.3  2008-05-28 17:43:12+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:45:57+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-07 18:41:56+05:30  Cprogrammer
 * no point in updating when table does not exist
 *
 * Revision 1.7  2002-08-03 04:40:23+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.6  2002-02-23 20:27:41+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.5  2001-12-22 18:19:17+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.4  2001-12-12 13:45:27+05:30  Cprogrammer
 * change for mda ip address
 *
 * Revision 1.3  2001-12-11 11:36:27+05:30  Cprogrammer
 * open master for updates
 *
 * Revision 1.2  2001-12-10 00:04:52+05:30  Cprogrammer
 * update only when old port matches
 *
 * Revision 1.1  2001-12-09 23:49:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_update.c,v 2.3 2008-05-28 17:43:12+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

int
vsmtp_update(char *host, char *src_host, char *domain, int oldport, int newport)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vsmtp_update: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority smtp_port set port = %d where host = \"%s\" and src_host = \"%s\" and domain = \"%s\" and port=%d",
		newport, host, src_host, domain, oldport);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			fprintf(stderr, "vsmtp_update: No rows selected\n");
			return (0);
		}
		fprintf(stderr, "vsmtp_update: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vsmtp_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
