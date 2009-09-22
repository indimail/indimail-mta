/*
 * $Log: mlist_filterno.c,v $
 * Revision 2.6  2008-09-08 09:50:18+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 16:37:10+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2002-11-18 12:40:45+05:30  Cprogrammer
 * corrected bug with null result by mysql_fetch_row()
 *
 * Revision 2.3  2002-10-27 21:27:23+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-10-14 20:56:41+05:30  Cprogrammer
 * use mysql_num_rows() to check detect zero rows
 *
 * Revision 2.1  2002-10-12 10:28:12+05:30  Cprogrammer
 * function to get filter_no for option 'Not in To, CC, Bcc' filter option
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mlist_filterno.c,v 2.6 2008-09-08 09:50:18+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
mlist_filterno(char *emailid)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             filter_no;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"select high_priority max(filter_no) from vfilter where  emailid = \"%s\" and comparision=5 or \
		comparision=6", emailid);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT);
			return (0);
		} 
		fprintf(stderr, "mlist_filterno: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
		return (-1);
	}
	if (!(res = mysql_store_result(&mysql[1])))
		return (-1);
	if(!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return(-2);
	}
	if ((row = mysql_fetch_row(res)))
	{
		if(row[0] && *(row[0]))
			filter_no = atoi(row[0]);
		else
			filter_no = -2;
		mysql_free_result(res);
		return (filter_no);
	}
	mysql_free_result(res);
	return (0);
}
#endif

void
getversion_mlist_filterno_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
