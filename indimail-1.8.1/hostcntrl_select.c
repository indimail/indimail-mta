/*
 * $Log: hostcntrl_select.c,v $
 * Revision 2.4  2008-09-08 09:44:00+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2004-06-19 00:18:25+05:30  Cprogrammer
 * added funtion to select all hostcntrl entries
 *
 * Revision 2.2  2004-05-17 14:01:30+05:30  Cprogrammer
 * added time field
 *
 * Revision 2.1  2004-05-17 00:59:40+05:30  Cprogrammer
 * function to select hostcntrl records
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: hostcntrl_select.c,v 2.4 2008-09-08 09:44:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
hostcntrl_select(char *user, char *domain, time_t *tmval, char *hostid, int len)
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if(open_central_db(0))
		return(1);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"select high_priority host,unix_timestamp(timestamp) from %s where  pw_name=\"%s\" and pw_domain=\"%s\"",
		cntrl_table, user, domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			return (0);
		} 
		fprintf(stderr, "vhostid_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	}
	if(!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "hostcntrl_select: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return (1);
	}
	if ((row = mysql_fetch_row(res)))
	{
		snprintf(hostid, len, "%s", row[0]);
		if (tmval)
			*tmval = atol(row[1]);
		mysql_free_result(res);
		return (0);
	}
	mysql_free_result(res);
	return (1);
}

MYSQL_ROW
hostcntrl_select_all()
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if(!select_res)
	{
		if(open_central_db(0))
			return(0);
		snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority pw_name, pw_domain, host, unix_timestamp(timestamp) from %s",
			cntrl_table);
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
				return (0);
			} 
			fprintf(stderr, "vhostid_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (0);
		}
		if(!(select_res = mysql_store_result(&mysql[0])))
		{
			fprintf(stderr, "hostcntrl_select: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			return (0);
		}
	}
	if ((row = mysql_fetch_row(select_res)))
		return (row);
	mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return (0);
}
#endif

void
getversion_hostcntrl_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
