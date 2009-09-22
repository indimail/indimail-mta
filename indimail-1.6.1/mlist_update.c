/*
 * $Log: mlist_update.c,v $
 * Revision 2.5  2008-09-08 09:50:35+05:30  Cprogrammer
 * remvoed mysql_escape
 *
 * Revision 2.4  2008-05-28 16:37:19+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:28:43+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-10-11 20:02:06+05:30  Cprogrammer
 * changed function interface to update a specified mailing_list
 *
 * Revision 2.1  2002-10-09 23:31:56+05:30  Cprogrammer
 * function to update mailing lists
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mlist_update.c,v 2.5 2008-09-08 09:50:35+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
mlist_update(char *emailid, char *old_list, char *new_list)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, sizeof(SqlBuf), \
		"update low_priority mailing_list set mailing_list=\"%s\" where emailid=\"%s\" and mailing_list=\"%s\"",
		new_list, emailid, old_list);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, "mailing_list", MAILING_LIST_TABLE_LAYOUT);
			return(1);
		}
		mysql_perror("mlist_update: %s", SqlBuf);
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
		printf("Updated mailing list [%s]->[%s] for email [%s]\n", old_list, new_list, emailid);
	else
		fprintf(stderr, "No mailing list %s for %s or no mailing list to update\n", old_list, emailid);
	return ((err > 0) ? 0 : 1);
}
#endif

void
getversion_mlist_update_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
