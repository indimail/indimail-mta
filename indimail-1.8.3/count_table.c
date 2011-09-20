/*
 * $Log: count_table.c,v $
 * Revision 2.4  2008-09-08 09:32:52+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:34:22+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-12-08 20:58:20+05:30  Cprogrammer
 * added check for null argument
 *
 * Revision 2.1  2003-12-07 00:23:37+05:30  Cprogrammer
 * function to count rows in a table
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: count_table.c,v 2.4 2008-09-08 09:32:52+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

long
count_table(char *table)
{
	char            SqlBuf[SQL_BUF_SIZE];
	long            row_count;
	MYSQL_ROW       row;
	MYSQL_RES      *select_res;

	if (vauth_open((char *) 0))
		return(-1);
	if (!table || !*table)
		return(-1);
	snprintf(SqlBuf, sizeof(SqlBuf), "select count(*) from %s", table);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			return(0);
		mysql_perror("count_table: %s", SqlBuf);
		return(-1);
	}
	if(!(select_res = mysql_store_result(&mysql[1])))
		return(-1);
	if ((row = mysql_fetch_row(select_res)))
	{
		row_count = atol(row[0]);
		mysql_free_result(select_res);
		return (row_count);
	}
	/*- should not happen */
	mysql_free_result(select_res);
	return(-1);
}

void
getversion_count_table_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
