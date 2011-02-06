/*
 * $Log: valiasCount.c,v $
 * Revision 2.6  2008-09-08 09:54:38+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 16:38:16+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2004-01-06 17:17:45+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.3  2003-12-07 00:23:02+05:30  Cprogrammer
 * changed return type to long
 * mysql result was not being freed
 *
 * Revision 2.2  2003-01-03 02:27:30+05:30  Cprogrammer
 * return zero count for syntax error
 *
 * Revision 2.1  2002-12-29 18:53:37+05:30  Cprogrammer
 * function to return alias count for a user
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: valiasCount.c,v 2.6 2008-09-08 09:54:38+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

long
valiasCount(char *alias, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *real_domain;
	int             err;
	long            row_count;
	MYSQL_ROW       row;
	MYSQL_RES      *select_res;

	if(!domain || !*domain)
		return(-1);
	if (vauth_open((char *) 0))
		return(-1);
	if(!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(SqlBuf, sizeof(SqlBuf), 
		"select count(*) from valias where alias=\"%s\" and domain=\"%s\"", 
		alias, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		err = mysql_errno(&mysql[1]);
		mysql_perror("valiasCount: %s", SqlBuf);
		if(err == ER_NO_SUCH_TABLE && create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
			mysql_perror("valiasCount: create_table valias: %s", SqlBuf);
		if(err == ER_NO_SUCH_TABLE || err == ER_SYNTAX_ERROR)
			return(0);
		return(-1);
	}
	if(!(select_res = mysql_store_result(&mysql[1])))
		return(-1);
	if ((row = mysql_fetch_row(select_res)))
	{
		row_count = atol(row[0]);
		mysql_free_result(select_res);
		return (row_count);
	}
	mysql_free_result(select_res);
	return(0);
}
#endif

void
getversion_valiasCount_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
