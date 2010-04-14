/*
 * $Log: vhostid_delete.c,v $
 * Revision 2.2  2002-10-27 21:41:14+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-10-27 00:30:57+05:30  Cprogrammer
 * corrected return code
 *
 * Revision 1.2  2002-08-03 04:38:47+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.1  2002-03-29 20:32:31+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_delete.c,v 2.2 2002-10-27 21:41:14+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>
int
vhostid_delete(char *hostid)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if (open_master())
	{
		fprintf(stderr, "vhostid_delete: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from host_table where host=\"%s\"", hostid);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
				return (-1);
			return (1);
		}
		fprintf(stderr, "vhostid_delete: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "vhostid_delete: mysql_affected_rows: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	return (err > 0 ? 0 : 1);
}
#endif

void
getversion_vhostid_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
