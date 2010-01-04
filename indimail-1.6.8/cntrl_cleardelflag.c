/*
 * $Log: cntrl_cleardelflag.c,v $
 * Revision 2.7  2009-02-06 11:37:01+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.6  2008-11-10 11:56:45+05:30  Cprogrammer
 * conditional compilation of QUERY_CACHE code
 *
 * Revision 2.5  2008-11-06 15:39:03+05:30  Cprogrammer
 * added cache reset option
 *
 * Revision 2.4  2008-09-08 09:32:03+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 17:39:15+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-01-03 02:45:23+05:30  Cprogrammer
 * replaced vcreate_cntrl_table() with create_table()
 *
 * Revision 2.1  2002-08-05 00:11:15+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 1.6  2002-08-03 04:26:25+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.5  2002-02-23 20:22:07+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.4  2001-12-22 18:06:16+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.3  2001-12-21 02:19:58+05:30  Cprogrammer
 * added open_master() and create table on when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.2  2001-11-30 00:11:56+05:30  Cprogrammer
 * used variable cntrl_table for hostcntrl table
 *
 * Revision 1.1  2001-11-29 20:50:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: cntrl_cleardelflag.c,v 2.7 2009-02-06 11:37:01+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

/*
 * -1 - Mysql Error (or) Assignment Error , so insert was a failure
 *  1 - Delete from Location DB was a success
 */
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

int
cntrl_cleardelflag(char *user, char *domain)
{
	int             ret;
	static char     User[MAX_BUFF], Domain[MAX_BUFF];
	static int      is_present;
	char            SqlBuf[SQL_BUF_SIZE];

	if (!user || !*user || !domain || !*domain)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE") && is_present != -1 && 
		!strncmp(user, User, MAX_BUFF) && !strncmp(domain, Domain, MAX_BUFF))
		return(is_present);
	else
	{
		scopy(User, user, MAX_BUFF);
		scopy(Domain, domain, MAX_BUFF);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#else
	scopy(User, user, MAX_BUFF);
	scopy(Domain, domain, MAX_BUFF);
#endif
	if (open_master())
	{
		(void) fprintf(stderr, "cntrl_cleardelflag: Failed to open Master Db: %s\n", mysql_error(&mysql[0]));
		return(is_present = -1);
	}
	if ((ret = is_user_present(user, domain)) == -1)
	{
		(void) fprintf(stderr, "cntrl_cleardelflag: Auth Db Error\n");
		return(is_present = -1);
	}	
	if (ret == 1)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_name = \"%s\" and pw_domain = \"%s\"", 
				cntrl_table, user, domain);
		if (mysql_query(&mysql[0], SqlBuf))
		{
			if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
				create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT);
			else
			{
				(void) fprintf(stderr, "cntrl_cleardelflag: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return(is_present = -1);
			}
		}
	}
	return(is_present = (vauth_updateflag(user, domain, -1) ? 0 : 1));
}

#ifdef QUERY_CACHE
void
cntrl_cleardelflag_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif /*- #ifdef CLUSTERED_SITE */

void
getversion_cntrl_cleardelflag_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
