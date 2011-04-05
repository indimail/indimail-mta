/*
 * $Log: vsmtp_select.c,v $
 * Revision 2.3  2008-05-28 17:42:51+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:45:36+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-07 18:41:27+05:30  Cprogrammer
 * return null if table does not exist
 *
 * Revision 1.7  2002-08-03 04:40:19+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.6  2002-02-23 20:27:37+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.5  2001-12-23 00:50:21+05:30  Cprogrammer
 * removed function vsmtp_select_next
 *
 * Revision 1.4  2001-12-22 18:19:13+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.3  2001-12-12 13:45:22+05:30  Cprogrammer
 * change for mda ip address
 *
 * Revision 1.2  2001-12-11 11:36:15+05:30  Cprogrammer
 * change in open_central_db()
 *
 * Revision 1.1  2001-12-09 23:49:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: vsmtp_select.c,v 2.3 2008-05-28 17:42:51+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <mysqld_error.h>

char           *
vsmtp_select(char *domain, int *Port)
{
	static char     tmpbuf[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	static MYSQL_RES *res;
	MYSQL_ROW       row;

	if(!res)
	{
		*Port = -1;
		if(open_central_db(0))
			return ((char *) 0);
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority host, src_host, port from smtp_port where  domain = \"%s\"", domain);
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if(mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_MASTER, "smtp_port", SMTP_TABLE_LAYOUT);
				return ((char *) 0);
			} else
			{
				fprintf(stderr, "vsmtp_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return ((char *) 0);
			}
		}
		if(!(res = mysql_store_result(&mysql[0])))
			return ((char *) 0);
	} 
	if ((row = mysql_fetch_row(res)))
	{
		*Port = atoi(row[2]);
		snprintf(tmpbuf, MAX_BUFF, "%-15s %s", row[1], row[0]);
		return (tmpbuf);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	*Port = -1;
	return ((char *) 0);
}

#endif

void
getversion_vsmtp_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
