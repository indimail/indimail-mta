/*
 * $Log: vfilter_filterNo.c,v $
 * Revision 2.5  2008-09-08 09:57:15+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.4  2008-05-28 17:40:55+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:37:41+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-10-16 20:06:27+05:30  Cprogrammer
 * corrected SQL Statement
 * corrected incorrect return value of -1 (while getting the highest value of filter_no)
 *
 * Revision 2.1  2002-10-14 21:10:54+05:30  Cprogrammer
 * function to get the lowest filter_no available for use in a new filter
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_filterNo.c,v 2.5 2008-09-08 09:57:15+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_filterNo(char *emailid)
{
	int             i;
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	MYSQL_RES      *res;

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, sizeof(SqlBuf), 
		"select high_priority filter_no from vfilter where emailid=\"%s\" and filter_no > 1 order by filter_no", emailid);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return(-1);
			return(2);
		}
		mysql_perror("vfilter_getfilterNo: %s", SqlBuf);
		return(-1);
	} 
	if(!(res = mysql_store_result(&mysql[1])))
		return (-1);
	if(!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return (2);
	}
	for(i = 2;(row = mysql_fetch_row(res));i++)
	{
		if(i != atoi(row[0]))
		{
			mysql_free_result(res);
			return(i);
		}
	} 
	mysql_free_result(res);
	return (i);
}
#endif

void
getversion_vfilter_filterNo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
