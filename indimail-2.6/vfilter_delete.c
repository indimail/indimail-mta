/*
 * $Log: vfilter_delete.c,v $
 * Revision 2.8  2008-09-08 09:57:09+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.7  2008-05-28 17:40:53+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.6  2002-10-27 21:37:12+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.5  2002-10-27 00:29:17+05:30  Cprogrammer
 * added code to create filter table if mysql_query() returns ER_NO_SUCH_TABLE
 *
 * Revision 2.4  2002-10-11 20:04:57+05:30  Cprogrammer
 * removed code to delete mailing list
 *
 * Revision 2.3  2002-10-10 01:16:06+05:30  Cprogrammer
 * added deletion of mailing list
 *
 * Revision 2.2  2002-10-09 23:24:59+05:30  Cprogrammer
 * emailid properly escaped
 *
 * Revision 2.1  2002-09-30 23:56:07+05:30  Cprogrammer
 * function to delete filters
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_delete.c,v 2.8 2008-09-08 09:57:09+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_delete(char *emailid, int filter_no)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	if(filter_no == -1)
		snprintf(SqlBuf, sizeof(SqlBuf), "delete low_priority from vfilter where emailid=\"%s\"", emailid);
	else
		snprintf(SqlBuf, sizeof(SqlBuf), "delete low_priority from vfilter where emailid=\"%s\" and filter_no=%d", 
			emailid, filter_no);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return (-1);
			return(0);
		}
		mysql_perror("vfilter_delete: %s", SqlBuf);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	return((err > 0) ? 0 : 1);
}
#endif

void
getversion_vfilter_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
