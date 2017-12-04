/*
 * $Log: vfilter_update.c,v $
 * Revision 2.10  2017-05-01 20:20:19+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.9  2008-09-08 09:57:45+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.8  2008-05-28 17:41:07+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2003-03-24 19:28:54+05:30  Cprogrammer
 * do not return error if all fields to be updated are same
 *
 * Revision 2.6  2002-12-05 18:33:27+05:30  Cprogrammer
 * forwarding address setting corrected
 *
 * Revision 2.5  2002-10-27 21:38:58+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.4  2002-10-11 20:06:54+05:30  Cprogrammer
 * corrections in return values
 * corrected logic to update mailing lists with a new list
 *
 * Revision 2.3  2002-10-10 01:16:29+05:30  Cprogrammer
 * added updation of mailing list
 * added return status
 *
 * Revision 2.2  2002-10-09 23:26:09+05:30  Cprogrammer
 * escaped emailid
 *
 * Revision 2.1  2002-09-30 23:56:51+05:30  Cprogrammer
 * function to update filters
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_update.c,v 2.10 2017-05-01 20:20:19+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_update(char *emailid, int filter_no, int header_name, int comparision, char *keyword, char *folder, int bounce_action, 
		char *faddr)
{
	int             err, terr;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, sizeof(SqlBuf),
		"update low_priority vfilter set header_name=%d, comparision=%d, keyword=\"%s\", \
		destination=\"%s\", bounce_action=\"%d%s\" where emailid=\"%s\" and filter_no=%d", 
		header_name, comparision, keyword, folder, bounce_action, (bounce_action == 2 || bounce_action == 3) ? faddr : "", 
		emailid, filter_no);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT);
			return(1);
		}
		mysql_perror("vfilter_update: %s", SqlBuf);
		return (-1);
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	terr = 0;
	if(!verbose)
		return ((err >= 0 && !terr) ? 0 : 1);
	if(err)
		printf("Updated filter no %d header %d keyword [%s] comparision %d folder [%s] bounce_action %d email [%s]\n",
			filter_no, header_name, keyword, comparision, folder, bounce_action, emailid);
	else
		fprintf(stderr, "No filter No %d for %s or no filter to update\n", filter_no, emailid);
	return ((err >= 0 && !terr) ? 0 : 1);
}
#endif

void
getversion_vfilter_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
