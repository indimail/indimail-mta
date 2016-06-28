/*
 * $Log: valias_select.c,v $
 * Revision 2.9  2014-07-03 00:09:13+05:30  Cprogrammer
 * added option to track all alias which deliver to an address using valias_track()
 *
 * Revision 2.8  2008-09-08 09:55:08+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.7  2008-05-28 15:44:12+05:30  Cprogrammer
 * removed leftover cdb code
 *
 * Revision 2.6  2008-05-28 15:22:02+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.5  2004-05-19 20:06:18+05:30  Cprogrammer
 * new logic for replacing '.' with ':'
 *
 * Revision 2.4  2004-01-06 17:18:21+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.3  2003-07-02 18:34:10+05:30  Cprogrammer
 * mysql_query() not needed if table not present
 *
 * Revision 2.2  2002-10-27 21:32:40+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-05 01:01:57+05:30  Cprogrammer
 * added mysql_escape for alias
 *
 * Revision 1.10  2002-08-03 04:33:37+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.9  2002-02-23 20:24:28+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.8  2001-12-23 00:47:41+05:30  Cprogrammer
 * removed function valias_select_next
 *
 * Revision 1.7  2001-12-22 18:09:43+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.6  2001-11-30 00:33:20+05:30  Cprogrammer
 * removed '\n'
 *
 * Revision 1.5  2001-11-24 12:20:36+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.4  2001-11-23 00:14:18+05:30  Cprogrammer
 * added check and return null when domain is null
 *
 * Revision 1.3  2001-11-20 10:56:31+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:25:58+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: valias_select.c,v 2.9 2014-07-03 00:09:13+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VALIAS
#include <stdlib.h>
#include <mysqld_error.h>

char           *
valias_select(char *alias, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *real_domain;
	MYSQL_ROW       row;
	static MYSQL_RES *select_res;

	if(!domain || !*domain)
		return((char *) 0);
	if(!select_res)
	{
		if (vauth_open((char *) 0))
			return ((char *) 0);
		if(!(real_domain = vget_real_domain(domain)))
			real_domain = domain;
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority valias_line from valias where alias=\"%s\" and domain=\"%s\"", 
			alias, real_domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return ((char *) 0);
			}
			mysql_perror("valias_select: %s", SqlBuf);
			return ((char *) 0);
		}
		if(!(select_res = mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = mysql_fetch_row(select_res)))
		return (row[0]);
	mysql_free_result(select_res);
	select_res = (MYSQL_RES *) 0;
	return ((char *) 0);
}

char           *
valias_track(char *alias, char *domain, char *valias_line, int len)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	static MYSQL_RES *res;

	if(!res)
	{
		if ((err = vauth_open((char *) 0)) != 0)
			return ((char *) 0);
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority alias, domain from valias where valias_line = \"%s\"",
			valias_line);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return ((char *) 0);
			}
			mysql_perror("valias_track: %s", SqlBuf);
			return ((char *) 0);
		}
		if(!(res = mysql_store_result(&mysql[1])))
			return ((char *) 0);
	}
	if ((row = mysql_fetch_row(res)))
	{
		if(alias)
			scopy(alias, (row[0]), len);
		if(domain && !*domain)
			scopy(domain, (row[1]), len);
		return (row[1]);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif /*- #ifdef VALIAS */

void
getversion_valias_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
