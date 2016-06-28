/*
 * $Log: vsmtp_insert.c,v $
 * Revision 2.2  2008-05-28 17:42:49+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2002-10-27 21:45:19+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.6  2002-08-03 04:40:15+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.5  2002-02-23 20:27:32+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.4  2001-12-22 18:19:09+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.3  2001-12-12 13:45:17+05:30  Cprogrammer
 * change for mda ip address
 *
 * Revision 1.2  2001-12-11 11:36:08+05:30  Cprogrammer
 * open master for updates
 *
 * Revision 1.1  2001-12-09 23:49:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_insert.c,v 2.2 2008-05-28 17:42:49+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

int
vsmtp_insert(char *host, char *src_host, char *domain, int smtp_port)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vsmtp_insert: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into smtp_port (host, src_host, domain, port) values (\"%s\", \"%s\", \"%s\", %d)", 
		host, src_host, domain, smtp_port);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT))
				return (-1);
			if (!mysql_query(&mysql[0], SqlBuf))
				return (0);
		}
		fprintf(stderr, "vsmtp_insert: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vsmtp_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
