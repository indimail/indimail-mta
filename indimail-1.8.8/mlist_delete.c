/*
 * $Log: mlist_delete.c,v $
 * Revision 2.6  2008-09-08 09:50:04+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 16:37:07+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2002-10-27 21:26:54+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-10-27 00:28:24+05:30  Cprogrammer
 * added code to create filter table if mysql_query returns ER_NO_SUCH_TABLE
 *
 * Revision 2.2  2002-10-11 20:00:51+05:30  Cprogrammer
 * changed function interface to delete on mailing_list instead of filter_no
 *
 * Revision 2.1  2002-10-09 23:31:23+05:30  Cprogrammer
 * function to delete mailing lists
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mlist_delete.c,v 2.6 2008-09-08 09:50:04+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
mlist_delete(char *emailid, char *mailing_list)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	if(!mailing_list || !*mailing_list)
		snprintf(SqlBuf, sizeof(SqlBuf), "delete low_priority from mailing_list where emailid=\"%s\"", emailid);
	else
		snprintf(SqlBuf, sizeof(SqlBuf), "delete low_priority from mailing_list where emailid=\"%s\" \
			and mailing_list=\"%s\"", emailid, mailing_list);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "mailing_list", MAILING_LIST_TABLE_LAYOUT);
			return(1);
		}
		mysql_perror("mlist_delete: %s", SqlBuf);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	return((err >= 0) ? 0 : 1);
}
#endif

void
getversion_mlist_delete_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
