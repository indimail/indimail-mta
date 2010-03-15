/*
 * $Log: cntrl_clearaddflag.c,v $
 * Revision 2.6  2009-02-06 11:36:57+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.5  2008-11-06 15:37:27+05:30  Cprogrammer
 * added cache_reset option
 *
 * Revision 2.4  2008-09-08 09:31:47+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 17:38:28+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2003-01-03 02:45:15+05:30  Cprogrammer
 * replaced vcreate_cntrl_table() with create_table()
 *
 * Revision 2.1  2002-08-05 00:09:57+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 1.7  2002-08-03 04:25:50+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.6  2002-03-29 22:08:00+05:30  Cprogrammer
 * add hostid instead of ip address
 *
 * Revision 1.5  2002-02-23 20:21:33+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.4  2001-12-21 02:19:34+05:30  Cprogrammer
 * check for ER_NO_SUCH_TABLE before creating a table
 *
 * Revision 1.3  2001-12-08 23:50:04+05:30  Cprogrammer
 * removed addition of port in host column of hostcntrl table
 *
 * Revision 1.2  2001-11-30 00:11:40+05:30  Cprogrammer
 * used variable cntrl_table for hostcntrl table
 *
 * Revision 1.1  2001-11-29 20:49:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: cntrl_clearaddflag.c,v 2.6 2009-02-06 11:36:57+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <mysqld_error.h>

/*
 * -1 - Mysql Error (or) Assignment Error , so insert was a failure
 *  1 - Add in hostcntrl was a success
 */

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

int
cntrl_clearaddflag(char *user, char *domain, char *passwd)
{
	int             ret;
	static int      is_present;
	static char     User[MAX_BUFF], Domain[MAX_BUFF];
	char            SqlBuf[SQL_BUF_SIZE];
	char 		   *hostid;

	if (!user || !*user || !domain || !*domain || !passwd || !*passwd)
		return (-1);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE") && is_present != -1 && !strncmp(user, User, MAX_BUFF) 
		&& !strncmp(domain, Domain, MAX_BUFF))
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
	if ((ret = is_user_present(user, domain)) == -1)
	{
		(void) fprintf(stderr, "cntrl_clearaddflag: Auth Db Error\n");
		is_present = -1;
		return (-1);
	}	
	if (ret == 1)
	{
		is_present = 0;
		return(0);
	}
	if (!(hostid = get_local_hostid()))
	{
		(void) fprintf(stderr, "cntrl_clearaddflag: Unable to get Local IPAddress: %s\n", strerror(errno));
		is_present = -1;
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "insert low_priority into %s (pw_name, pw_domain, pw_passwd, host, timestamp) \
		values(\"%s\",\"%s\",\"%s\",\"%s\", FROM_UNIXTIME(%lu))", 
		cntrl_table, user, domain, passwd, hostid, time(0));
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
			{
				fprintf(stderr, "cntrl_clearaddflag: create_table: %s: %s\n", cntrl_table, mysql_error(&mysql[0]));
				return(-1);
			}
			if (!mysql_query(&mysql[0], SqlBuf))
				return(is_present = (vauth_updateflag(user, domain, 1) ? 0 : 1));
		} 
		fprintf(stderr, "cntrl_clearaddflag: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return(is_present = -1);
	}
	return(is_present = (vauth_updateflag(user, domain, 1) ? 0 : 1));
}

#ifdef QUERY_CACHE
void
cntrl_clearaddflag_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif
#endif

void
getversion_cntrl_clearaddflag_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
