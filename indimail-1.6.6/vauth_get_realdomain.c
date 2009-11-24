/*
 * $Log: vauth_get_realdomain.c,v $
 * Revision 2.5  2008-11-06 15:38:31+05:30  Cprogrammer
 * added cache reset option
 *
 * Revision 2.4  2008-05-28 16:39:22+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2002-10-27 21:34:08+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-03 04:35:04+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-07-04 22:10:53+05:30  Cprogrammer
 * code to cache results
 *
 * Revision 1.2  2002-02-24 22:07:38+05:30  Cprogrammer
 * create table aliasdomain if query fails and errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.1  2001-12-21 01:24:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_get_realdomain.c,v 2.5 2008-11-06 15:38:31+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

char *
vauth_get_realdomain(char *aliasdomain)
{
	static char     buf[MAX_BUFF], prevDomainVal[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE") && *prevDomainVal && 
		!strncmp(prevDomainVal, aliasdomain, MAX_BUFF))
	{
		if (*buf)
			return(buf);
		else
			return((char *) 0);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	if (open_central_db((char *) 0))
		return((char *) 0);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority domain from aliasdomain where  alias=\"%s\"", aliasdomain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "aliasdomain", ALIASDOMAIN_TABLE_LAYOUT);
			return ((char *) 0);
		}
		(void) fprintf(stderr, "vauth_get_realdomain: mysql_query: %s\n", mysql_error(&mysql[0]));
		return((char *) 0);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		(void) fprintf(stderr, "vauth_get_realdomain: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return ((char *) 0);
	}
	if (mysql_num_rows(res) == 0)
	{
		mysql_free_result(res);
		scopy(prevDomainVal, aliasdomain, MAX_BUFF);
		return ((char *) 0);
	}
	row = mysql_fetch_row(res);
	scopy(buf, row[0], MAX_BUFF);
	mysql_free_result(res);
	scopy(prevDomainVal, aliasdomain, MAX_BUFF);
	return(buf);
}

#ifdef QUERY_CACHE
void
vauth_get_realdomain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif

void
getversion_vauth_get_realdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
