/*
 * $Log: vpriv_select.c,v $
 * Revision 2.6  2010-03-07 11:27:56+05:30  Cprogrammer
 * use open_central_db instead of open_master
 *
 * Revision 2.5  2008-09-08 09:58:35+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.4  2008-06-13 10:59:31+05:30  Cprogrammer
 * compile vpriv_select() only if CLUSTERED_SITE defined
 *
 * Revision 2.3  2008-05-28 17:42:01+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-09-16 12:36:09+05:30  Cprogrammer
 * added option to select all entries
 *
 * Revision 2.1  2003-09-14 01:59:13+05:30  Cprogrammer
 * function to retrieve privileges
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vpriv_select.c,v 2.6 2010-03-07 11:27:56+05:30 Cprogrammer Stab mbhangui $";
#endif
#ifdef CLUSTERED_SITE
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

char           *
vpriv_select(char **user, char **program)
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!select_res)
	{
		if (open_central_db(0))
			return ((char *) 0);
		if (program && *program)
		{
			if (user && *user && **user)
				snprintf(SqlBuf, SQL_BUF_SIZE,
					"select high_priority user,program,cmdswitches from vpriv where user=\"%s\" and program=\"%s\"",
					*user, *program);
			else
				snprintf(SqlBuf, SQL_BUF_SIZE,
					"select high_priority user,program,cmdswitches from vpriv where program=\"%s\"", *program);
		} else
		{
			if (user && *user && **user)
				snprintf(SqlBuf, SQL_BUF_SIZE,
					"select high_priority user,program,cmdswitches from vpriv where user=\"%s\"", *user);
			else
				snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority user,program,cmdswitches from vpriv");
		}
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_MASTER, "vpriv", PRIV_CMD_LAYOUT);
				return ((char *) 0);
			}
			fprintf(stderr, "vpriv_select: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return ((char *) 0);
		}
		if (!(select_res = mysql_store_result(&mysql[0])))
			return ((char *) 0);
	}
	if ((row = mysql_fetch_row(select_res)))
	{
		*user = row[0];
		*program = row[1];
		return (row[2]);
	}
	mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif

void
getversion_vpriv_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
