/*
 * $Log: mlist_filterupdate.c,v $
 * Revision 2.4  2008-09-08 09:50:24+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:37:12+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:27:55+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-10-14 21:10:19+05:30  Cprogrammer
 * function to update filter_no for a mailing list
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mlist_filterupdate.c,v 2.4 2008-09-08 09:50:24+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
mlist_filterupdate(char *emailid, int filter_no)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, sizeof(SqlBuf), \
		"update low_priority mailing_list set filter_no=%d where emailid=\"%s\"", filter_no, emailid);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "mailing_list", MAILING_LIST_TABLE_LAYOUT);
			return(1);
		}
		mysql_perror("mlist_filterupdate: %s", SqlBuf);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	if(!verbose)
		return (err);
	if(err)
		printf("Updated mailing list filter_no %d for email [%s]\n", filter_no, emailid);
	else
		fprintf(stderr, "No mailing list %s or no mailing list to update\n", emailid);
	return ((err > 0) ? 0 : 1);
}
#endif

void
getversion_mlist_filterupdate_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
