/*
 * $Log: vhostid_update.c,v $
 * Revision 2.2  2003-07-02 18:41:11+05:30  Cprogrammer
 * return error if record not found for host
 *
 * Revision 2.1  2002-10-27 21:42:28+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.2  2002-08-03 04:38:57+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.1  2002-03-29 20:32:37+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_update.c,v 2.2 2003-07-02 18:41:11+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>
int
vhostid_update(char *hostid, char *ipaddr)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "vhostid_update: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority host_table set ipaddr=\"%s\" where host=\"%s\"",
		ipaddr, hostid);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			fprintf(stderr, "No rows selected for hostid %s\n", hostid);
			if (create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
				return (-1);
			return (1);
		}
		fprintf(stderr, "vhostid_update: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	if (mysql_affected_rows(&mysql[0]) == -1)
	{
		fprintf(stderr, "vhostid_insert: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}
#endif

void
getversion_vhostid_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
