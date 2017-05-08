/*
 * $Log: vhostid_select.c,v $
 * Revision 2.1  2002-10-27 21:42:00+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.1  2002-03-29 20:48:17+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vhostid_select.c,v 2.1 2002-10-27 21:42:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

char           *
vhostid_select()
{
	static char     tmpbuf[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if(open_central_db(0))
		return((char *) 0);
	if(!res)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority host, ipaddr from host_table");
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			{
				if(create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT))
					return((char *) 0);
				return((char *) 0);
			} else
			{
				fprintf(stderr, "vhostid_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return((char *) 0);
			}
		}
		if(!(res = mysql_store_result(&mysql[0])))
			return ((char *) 0);
	}
	if ((row = mysql_fetch_row(res)))
	{
		snprintf(tmpbuf, MAX_BUFF, "%-30s %s", row[0], row[1]);
		return (tmpbuf);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif

void
getversion_vhostid_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
