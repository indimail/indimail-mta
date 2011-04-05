/*
 * $Log: vauth_gethostid.c,v $
 * Revision 2.2  2008-05-28 16:39:05+05:30  Cprogrammer
 * removed USE_MYSQL.
 *
 * Revision 2.1  2002-10-27 21:33:24+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.3  2002-08-03 04:35:21+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.2  2002-04-03 01:43:05+05:30  Cprogrammer
 * use 127.0.0.1 if ipaddr is specified as localhost
 *
 * Revision 1.1  2002-03-29 23:56:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_gethostid.c,v 2.2 2008-05-28 16:39:05+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <mysqld_error.h>

char           *
vauth_gethostid(char *ipaddr)
{
	static char     hostid[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
		return((char *) 0);
	if(!ipaddr || !*ipaddr)
		return((char *) 0);
	if(!strncmp(ipaddr, "localhost", 9))
		ptr = "127.0.0.1";
	else
		ptr = ipaddr;
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority host from host_table where ipaddr=\"%s\"", 
		ptr);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "host_table", HOST_TABLE_LAYOUT);
		else
			fprintf(stderr, "vauth_gethostid: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return((char *) 0);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "vauth_gethostid: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return((char *) 0);
	}
	if (!(row = mysql_fetch_row(res)))
	{
		mysql_free_result(res);
		return((char *) 0);
	}
	scopy(hostid, row[0], MAX_BUFF);
	mysql_free_result(res);
	return(hostid);
}
#endif

void
getversion_vauth_gethostid_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
