/*
 * $Log: valias_select_all.c,v $
 * Revision 2.6  2008-09-08 09:54:58+05:30  Cprogrammer
 * formatting of long lines
 *
 * Revision 2.5  2008-05-28 16:38:23+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.4  2004-01-06 17:18:17+05:30  Cprogrammer
 * use real domain
 *
 * Revision 2.3  2003-07-02 18:33:17+05:30  Cprogrammer
 * mysql_query() not needed if table not present
 *
 * Revision 2.2  2003-06-19 00:00:15+05:30  Cprogrammer
 * added option to select all entries by specifying null value of domain
 *
 * Revision 2.1  2002-10-27 21:32:19+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.8  2002-02-23 20:24:32+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.7  2001-12-23 00:47:55+05:30  Cprogrammer
 * removed function valias_select_all_next
 *
 * Revision 1.6  2001-12-21 02:22:47+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.5  2001-12-09 23:56:16+05:30  Cprogrammer
 * removed unecessary sprintf statement
 *
 * Revision 1.4  2001-11-24 12:20:40+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:56:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:26:03+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: valias_select_all.c,v 2.6 2008-09-08 09:54:58+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VALIAS
#include <mysqld_error.h>

char           *
valias_select_all(char *alias, char *domain, int len)
{
	int             err;
	char            SqlBuf[SQL_BUF_SIZE];
	char           *real_domain;
	MYSQL_ROW       row;
	static MYSQL_RES *res;

	if(!res)
	{
		if ((err = vauth_open((char *) 0)) != 0)
			return ((char *) 0);
		if(domain && *domain)
		{
			if(!(real_domain = vget_real_domain(domain)))
				real_domain = domain;
			snprintf(SqlBuf, SQL_BUF_SIZE,
				"select high_priority alias, domain, valias_line from valias where domain = \"%s\"",
				real_domain);
		} else
			snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority alias, domain, valias_line from valias");
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT);
				return ((char *) 0);
			}
			mysql_perror("valias_select_all: %s", SqlBuf);
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
		return (row[2]);
	}
	mysql_free_result(res);
	res = (MYSQL_RES *) 0;
	return ((char *) 0);
}
#endif

void
getversion_valias_select_all_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
