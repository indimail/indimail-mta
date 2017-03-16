/*
 * $Log: vfilter_mlistOpt.c,v $
 * Revision 2.5  2008-09-08 09:57:26+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.4  2008-05-28 17:41:00+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:38:20+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-10-15 11:44:11+05:30  Cprogrammer
 * allow setting of mailing_list options based on comparision (5 or 6) rather than on filter_no
 *
 * Revision 2.1  2002-10-14 21:11:39+05:30  Cprogrammer
 * function to set the mailing list action (none, intelligent, specfied mailing list) for a filter
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_mlistOpt.c,v 2.5 2008-09-08 09:57:26+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
vfilter_mlistOpt(char *emailid, int option)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, sizeof(SqlBuf), 
		"update low_priority vfilter set mailing_list=%d where emailid=\"%s\" and comparision=5 or comparision=6", 
		option, emailid);
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
	if(!verbose)
		return (err > 0 ? 0 : 1);
	if(err)
		printf("Updated %d filter(s) with option %d for email [%s]\n", err, option, emailid);
	else
		fprintf(stderr, "No filter with comparision=5 or comparision=6 for %s, or no filter to update\n", emailid);
	return (err > 0 ? 0 : 1);
}
#endif

void
getversion_vfilter_mlistOpt_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
