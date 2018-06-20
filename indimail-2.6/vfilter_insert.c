/*
 * $Log: vfilter_insert.c,v $
 * Revision 2.14  2017-05-01 20:18:52+05:30  Cprogrammer
 * removed mailing list feature from vfilter
 *
 * Revision 2.13  2008-09-08 09:57:20+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.12  2008-05-28 17:40:58+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.11  2002-12-05 18:32:52+05:30  Cprogrammer
 * forwarding address setting corrected
 *
 * Revision 2.10  2002-11-18 12:43:26+05:30  Cprogrammer
 * added option to insert header_name
 *
 * Revision 2.9  2002-10-27 21:38:06+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.8  2002-10-21 01:33:06+05:30  Cprogrammer
 * create table vfilter if it does not exist
 *
 * Revision 2.7  2002-10-16 20:07:37+05:30  Cprogrammer
 * filter_no is 0 and 1 for comparision 5 and 6
 *
 * Revision 2.6  2002-10-14 21:03:37+05:30  Cprogrammer
 * moved out code for determining filter_no to use for a new record into function vfilter_filterNo()
 *
 * Revision 2.5  2002-10-11 20:05:27+05:30  Cprogrammer
 * fixed memory leak
 * set default filter_no to 1 when no rows present
 *
 * Revision 2.4  2002-10-11 00:06:29+05:30  Cprogrammer
 * insert mailing list only if specified
 *
 * Revision 2.3  2002-10-10 01:16:22+05:30  Cprogrammer
 * added return status
 *
 * Revision 2.2  2002-10-09 23:25:36+05:30  Cprogrammer
 * added mailing list insert
 * fixed memory leaks
 *
 * Revision 2.1  2002-09-30 23:56:24+05:30  Cprogrammer
 * function to insert filters
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_insert.c,v 2.14 2017-05-01 20:18:52+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_insert(char *emailid, char *filter_name, int header_name, int comparision, char *keyword, char *folder, int bounce_action, 
	char *faddr)
{
	int             err, filter_no;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	if(comparision == 5 || comparision == 6)
		filter_no = comparision - 5;
	else
	if((filter_no = vfilter_filterNo(emailid)) == -1)
	{
		fprintf(stderr, "failed to obtain filter No\n");
		return(-1);
	}
	snprintf(SqlBuf, sizeof(SqlBuf), "insert low_priority into vfilter \
			(emailid, filter_no, filter_name, header_name, comparision, keyword, destination, bounce_action) \
			values (\"%s\", %d, \"%s\", %d, %d, \"%s\", \"%s\", \"%d%s\")",
			emailid, filter_no, filter_name, header_name, comparision, keyword, folder, bounce_action,
			(bounce_action == 2 || bounce_action == 3) ? faddr : "");
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return(-1);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				mysql_perror("vfilter_insert: %s", SqlBuf);
				return (-1);
			}
		} else
		{
			mysql_perror("vfilter_insert: %s", SqlBuf);
			return (-1);
		}
	}
	if((err = mysql_affected_rows(&mysql[1])) == -1)
	{
		mysql_perror("mysql_affected_rows");
		return(-1);
	}
	if(!verbose)
		return (err > 0 ? 0 : 1);
	if(err)
		printf("Added filter No %d for %s\n", filter_no, emailid);
	else
		fprintf(stderr, "Filter No %d failed for %s\n", filter_no, emailid);
	return (err > 0 ? 0 : 1);
}
#endif

void
getversion_vfilter_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
