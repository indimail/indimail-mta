/*
 * $Log: vsmtp_delete.c,v $
 * Revision 2.4  2008-09-08 09:59:21+05:30  Cprogrammer
 * formatting of long lines
 *
 * Revision 2.3  2008-05-28 17:42:44+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:44:34+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-07 18:39:08+05:30  Cprogrammer
 * no point in trying delete if table does not exist
 *
 * Revision 1.6  2002-08-03 04:40:04+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.5  2002-02-23 20:27:24+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.4  2001-12-22 18:19:03+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.3  2001-12-12 13:45:11+05:30  Cprogrammer
 * change for mda ip address
 *
 * Revision 1.2  2001-12-11 11:35:57+05:30  Cprogrammer
 * open master for updates
 *
 * Revision 1.1  2001-12-09 23:48:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_delete.c,v 2.4 2008-09-08 09:59:21+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

int
vsmtp_delete(char *host, char *src_host, char *domain, int port)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vsmtp_delete: Failed to open Master Db\n");
		return (-1);
	}
	if (port)
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from smtp_port where host = \"%s\" and \
			src_host = \"%s\" and domain = \"%s\" and port =%d",
			host, src_host, domain, port);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from smtp_port where host = \"%s\" and src_host = \"%s\" and domain = \"%s\"",
			host, src_host, domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
			fprintf(stderr, "vsmtp_delete: No rows selected\n");
			return (1);
		} 
		fprintf(stderr, "vsmtp_delete: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "vsmtp_delete: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	return (err > 0 ? 0 : 1);
}
#endif

void
getversion_vsmtp_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
