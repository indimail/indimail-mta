/*
 * $Log: getmailingList.c,v $
 * Revision 2.4  2008-09-08 09:43:32+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:35:47+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:15:23+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-10-09 23:32:15+05:30  Cprogrammer
 * function to fetch mailing lists
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: getmailingList.c,v 2.4 2008-09-08 09:43:32+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

char  **
getmailingList(char *emailid, int filter_no)
{
	char            SqlBuf[SQL_BUF_SIZE];
	static char   **SqlPtr;
	unsigned long   num, more;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (vauth_open((char *) 0) != 0)
	{
		mysql_perror("getmailingList: vauth_open: %s", SqlBuf);
		return ((char **) 0);
	}
	if(filter_no == -1)
		snprintf(SqlBuf, SQL_BUF_SIZE, "select mailing_list from mailing_list where emailid = \"%s\"", emailid);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "select mailing_list from mailing_list where emailid = \"%s\" and \
			filter_no=%d", emailid, filter_no);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "mailing_list", MAILING_LIST_TABLE_LAYOUT);
			return ((char **) 0);
		} 
		mysql_perror("getmailingList: mysql_query: %s", SqlBuf);
		return ((char **) 0);
	}
#ifdef LOW_MEM
	if (!(res = mysql_use_result(&mysql[1])))
#else
	if (!(res = mysql_store_result(&mysql[1])))
#endif
	{
		mysql_perror("getmailingList: mysql_store_result");
		return ((char **) 0);
	}
	if(!(num = mysql_num_rows(res)))
	{
		mysql_free_result(res);
		return ((char **) 0);
	}
	if(!(SqlPtr = (char **) calloc(1, sizeof(char *) * (num + 1))))
	{
		perror("malloc");
		mysql_free_result(res);
		return ((char **) 0);
	}
	for(more = 0;(row = mysql_fetch_row(res));more++)
	{
		if(!(SqlPtr[more] = malloc(num = (slen(row[0]) + 1))))
		{
			perror("malloc");
			mysql_free_result(res);
			return ((char **) 0);
		}
		scopy(SqlPtr[more], row[0], num);
	}
#ifdef LOW_MEM
	if(!mysql_eof(res))
	{
		perror("malloc");
		mysql_free_result(res);
		return ((char **) 0);
	}
#endif
	mysql_free_result(res);
	SqlPtr[more] = (char *) 0;
	return(SqlPtr);
}

#endif

void
getversion_getmailingList_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
