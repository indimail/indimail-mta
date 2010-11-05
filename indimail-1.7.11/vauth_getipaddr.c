/*
 * $Log: vauth_getipaddr.c,v $
 * Revision 2.2  2008-05-28 16:39:08+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.1  2002-10-27 21:33:48+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.2  2002-08-03 04:35:26+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.1  2002-03-29 23:56:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_getipaddr.c,v 2.2 2008-05-28 16:39:08+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

char           *
vauth_getipaddr(char *hostid)
{
	static char     ipaddr[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
		return((char *) 0);
	if(!hostid || !*hostid)
		return((char *) 0);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority ipaddr from host_table where  host=\"%s\"", 
		hostid);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT);
		else
			fprintf(stderr, "vauth_getipaddr: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return((char *) 0);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "vauth_getipaddr: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return((char *) 0);
	}
	if (!(row = mysql_fetch_row(res)))
	{
		mysql_free_result(res);
		return((char *) 0);
	}
	scopy(ipaddr, row[0], MAX_BUFF);
	mysql_free_result(res);
	return(ipaddr);
}
#endif

void
getversion_vauth_getipaddr_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
