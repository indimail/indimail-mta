/*
 * $Log: mlist_insert.c,v $
 * Revision 2.7  2008-09-08 09:50:29+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.6  2008-05-28 16:37:17+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2002-10-27 21:55:04+05:30  Cprogrammer
 * removed unecessary mysql_perror() statement
 *
 * Revision 2.4  2002-10-27 21:28:18+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-10-11 20:01:31+05:30  Cprogrammer
 * added code to create table mailing_list if not present
 *
 * Revision 2.2  2002-10-10 01:15:39+05:30  Cprogrammer
 * added return status
 *
 * Revision 2.1  2002-10-09 23:31:41+05:30  Cprogrammer
 * function to insert mailing lists
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: mlist_insert.c,v 2.7 2008-09-08 09:50:29+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

int
mlist_insert(char *emailid, int filter_no, char **mailing_list)
{
	int             err, terr;
	char            SqlBuf[SQL_BUF_SIZE];
	char          **tmp_ptr;

	if (vauth_open((char *) 0))
		return (-1);
	for(err = 0, tmp_ptr = mailing_list;tmp_ptr && *tmp_ptr;tmp_ptr++)
	{
		snprintf(SqlBuf, sizeof(SqlBuf), "insert low_priority into mailing_list (emailid, filter_no, mailing_list) \
			values (\"%s\", %d, \"%s\")", emailid, filter_no, *tmp_ptr);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				if(create_table(ON_LOCAL, "mailing_list", MAILING_LIST_TABLE_LAYOUT))
					return (-1);
				if (mysql_query(&mysql[1], SqlBuf))
				{
					mysql_perror("mlist_insert: %s", SqlBuf);
					return (-1);
				}
			} else
			{
				mysql_perror("mlist_insert: %s", SqlBuf);
				return (-1);
			}
		}
		if(!err)
			terr = err = mysql_affected_rows(&mysql[1]);
		else
			terr = mysql_affected_rows(&mysql[1]);
		if(terr == -1)
		{
			mysql_perror("mysql_affected_rows");
			return(-1);
		}
	}
	if(!verbose)
		return (err > 0 ? 0 : 1);
	if(err)
		printf("Added mailing list No %d for %s\n", filter_no, emailid);
	else
		fprintf(stderr, "Mailing List No %d failed for %s\n", filter_no, emailid);
	return (err > 0 ? 0 : 1);
}
#endif

void
getversion_mlist_insert_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
