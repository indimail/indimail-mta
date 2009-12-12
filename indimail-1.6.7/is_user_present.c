/*
 * $Log: is_user_present.c,v $
 * Revision 2.6  2008-11-06 15:38:00+05:30  Cprogrammer
 * added cache_reset option
 *
 * Revision 2.5  2008-09-08 09:45:10+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.4  2008-05-28 16:36:09+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2003-07-02 18:24:24+05:30  Cprogrammer
 * mysql_query() not needed if table not present
 *
 * Revision 2.2  2003-01-03 02:45:30+05:30  Cprogrammer
 * replaced vcreate_cntrl_table() with create_table()
 *
 * Revision 2.1  2002-08-05 00:16:58+05:30  Cprogrammer
 * added mysql_escape for user
 *
 * Revision 1.13  2002-08-03 04:31:52+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.12  2002-03-27 13:21:43+05:30  Cprogrammer
 * set userNotFound to zero at start
 *
 * Revision 1.11  2002-02-23 20:23:28+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check.
 *
 * Revision 1.10  2001-12-21 02:21:07+05:30  Cprogrammer
 * create table if errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.9  2001-12-21 00:34:47+05:30  Cprogrammer
 * use real domain in case domain passed is an alias domain
 *
 * Revision 1.8  2001-12-11 11:31:29+05:30  Cprogrammer
 * change in open_central_db()
 *
 * Revision 1.7  2001-11-30 00:13:25+05:30  Cprogrammer
 * used variable cntrl_table for hostcntrl table
 *
 * Revision 1.6  2001-11-29 14:34:54+05:30  Cprogrammer
 * replaced strncpy with scopy
 *
 * Revision 1.5  2001-11-29 00:32:09+05:30  Cprogrammer
 * added checks for vcreate_cntrl_table()
 *
 * Revision 1.4  2001-11-28 23:00:40+05:30  Cprogrammer
 * improved efficiency by caching previous results
 *
 * Revision 1.3  2001-11-24 12:19:19+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-23 00:12:46+05:30  Cprogrammer
 * Corrected misleading error string
 *
 * Revision 1.1  2001-11-22 22:53:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: is_user_present.c,v 2.6 2008-11-06 15:38:00+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

/*
 * -1 - Mysql Error (or) Assignment Error
 *  0 - User Not Present
 *  1 - User Present
 */

#ifdef CLUSTERED_SITE
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

int
is_user_present(char *user, char *domain)
{
	int             ret;
	static char     User[MAX_BUFF], Domain[MAX_BUFF], SqlBuf[SQL_BUF_SIZE];
	static int      is_present;
	char           *real_domain;
	MYSQL_RES      *res;

	if (!user || !*user || !domain || !*domain)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE") && is_present != -1 && 
		!strncmp(user, User, MAX_BUFF) && !strncmp(domain, Domain, MAX_BUFF))
		return(is_present);
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	real_domain = (char *) 0;
	userNotFound = is_present = 0;
	if (!(real_domain = vget_real_domain(domain)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		return(is_present = -1);
	}
	if (open_central_db(0))
		return(is_present = -1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority host from %s where  pw_name=\"%s\" and pw_domain=\"%s\"", 
		cntrl_table, user, real_domain);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			userNotFound = 1;
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
				return(is_present = -1);
			return(is_present = 0);
		} else
			return(is_present = -1);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		(void) fprintf(stderr, "is_user_present: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return(is_present = -1);
	}
	ret = mysql_num_rows(res);
	mysql_free_result(res);
	scopy(User, user, MAX_BUFF);
	scopy(Domain, domain, MAX_BUFF);
	if (!ret)
		userNotFound = 1;
	return(is_present = ret);
}

#ifdef QUERY_CACHE
void
is_user_present_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif

void
getversion_is_user_present_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
