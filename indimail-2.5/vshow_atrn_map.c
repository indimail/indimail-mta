/*
 * $Log: vshow_atrn_map.c,v $
 * Revision 2.5  2009-02-27 08:39:59+05:30  Cprogrammer
 * show all atrn domain access if both email and domain are not specified
 *
 * Revision 2.4  2008-09-08 09:59:14+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 17:42:41+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-10-06 00:00:35+05:30  Cprogrammer
 * changed arguments to char **
 * option to select maps by specifying atrn domain on command line
 *
 * Revision 2.1  2003-07-04 11:30:44+05:30  Cprogrammer
 * show atrn MAPS for ODMR
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: vshow_atrn_map.c,v 2.5 2009-02-27 08:39:59+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <mysqld_error.h>

char           *
vshow_atrn_map(char **user, char **domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if (!select_res)
	{
		if (vauth_open((char *) 0))
			return ((char *) 0);
		if (user && *user)
			snprintf(SqlBuf, SQL_BUF_SIZE,
				"select high_priority domain_list from atrn_map where pw_name=\"%s\" and pw_domain=\"%s\"",
				*user, *domain);
		else
		if (domain && *domain)
			snprintf(SqlBuf, SQL_BUF_SIZE,
				"select high_priority pw_name,pw_domain,domain_list from atrn_map where domain_list=\"%s\"", *domain);
		else
		if (!domain || !*domain || !**domain)
			snprintf(SqlBuf, SQL_BUF_SIZE,
				"select high_priority pw_name,pw_domain,domain_list from atrn_map");
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "atrn_map", ATRN_MAP_LAYOUT);
				return ((char *) 0);
			}
			mysql_perror("vshow_atrn_map: %s", SqlBuf);
			return ((char *) 0);
		}
		if (!(select_res = mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = mysql_fetch_row(select_res)))
	{
		if (user && *user)
			return (row[0]);
		else
		{
			*user = row[0];
			*domain = row[1];
			return(row[2]);
		}
	}
	mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}

void
getversion_vshow_atrn_map_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
