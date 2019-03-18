/*
 * $Log: vfilter_select.c,v $
 * Revision 2.12  2017-05-01 20:19:56+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.11  2008-09-08 09:57:37+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.10  2008-05-28 17:41:04+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.9  2003-03-24 19:28:07+05:30  Cprogrammer
 * copy forwarding address if bounce_action is 2 or 3
 *
 * Revision 2.8  2002-12-06 01:56:24+05:30  Cprogrammer
 * corrected copying of forwarding address
 *
 * Revision 2.7  2002-11-13 13:37:41+05:30  Cprogrammer
 * added filter name
 *
 * Revision 2.6  2002-10-27 21:38:32+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.5  2002-10-14 21:08:45+05:30  Cprogrammer
 * initialize forward with NULL if forward information is not relevant
 *
 * Revision 2.4  2002-10-14 21:04:52+05:30  Cprogrammer
 * sql statement changed to order by filter_no
 * added code for intelligent mailing list
 *
 * Revision 2.3  2002-10-09 23:25:55+05:30  Cprogrammer
 * fixed memory leak
 *
 * Revision 2.2  2002-10-09 21:09:11+05:30  Cprogrammer
 * added call to getmailingList()
 *
 * Revision 2.1  2002-09-30 23:56:39+05:30  Cprogrammer
 * function to select filters
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_select.c,v 2.12 2017-05-01 20:19:56+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_select(char *emailid, int *filter_no, char *filter_name, int *header_name, int *comparision, char *keyword, 
	char *destination, int *bounce_action, char *forward)
{
	char            SqlBuf[SQL_BUF_SIZE];
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if (!res)
	{
		if (vauth_open((char *) 0))
			return (-1);
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority filter_no, filter_name, header_name, comparision, keyword, destination, \
			bounce_action from vfilter where  emailid = \"%s\" order by filter_no", emailid);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT);
				return (-1);
			} else
			{
				fprintf(stderr, "vfilter_select: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
				return (-1);
			}
		}
		if (!(res = mysql_store_result(&mysql[1])))
			return (-1);
	}
	if ((row = mysql_fetch_row(res)))
	{
		*filter_no = atoi(row[0]);
		scopy(filter_name, row[1], MAX_BUFF);
		*header_name = atoi(row[2]);
		*comparision = atoi(row[3]);
		scopy(keyword, row[4], MAX_BUFF);
		scopy(destination, row[5], MAX_BUFF);
		*bounce_action = atoi(row[6]);
		if(*bounce_action == 2 || *bounce_action == 3)
			scopy(forward, row[6] + 1, AUTH_SIZE);
		else
			*forward = 0;
		return(0);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return (-2);
}
#endif

void
getversion_vfilter_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
