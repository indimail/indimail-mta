/*
 * $Log: vclear_open_smtp.c,v $
 * Revision 2.7  2010-05-05 14:39:57+05:30  Cprogrammer
 * added connect_all argument to vclear_open_smtp
 * to process all mda hosts
 *
 * Revision 2.6  2008-05-28 16:40:12+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2005-12-29 22:52:00+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.4  2004-10-27 14:50:53+05:30  Cprogrammer
 * close mysql before exit
 *
 * Revision 2.3  2002-10-27 21:35:30+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-07-07 22:28:37+05:30  Cprogrammer
 * changed name of variable clear_minutes to clear_seconds
 *
 * Revision 2.1  2002-05-13 12:36:31+05:30  Cprogrammer
 * added code to centrally clear relay table
 *
 * Revision 1.10  2002-02-23 20:25:48+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.9  2001-12-22 18:15:46+05:30  Cprogrammer
 * create table only if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.8  2001-12-11 11:34:22+05:30  Cprogrammer
 * removed open_relay_db()
 *
 * Revision 1.7  2001-11-30 00:14:53+05:30  Cprogrammer
 * removed relay_table assignment
 *
 * Revision 1.6  2001-11-29 20:58:06+05:30  Cprogrammer
 * change for separate connection to a relay table, allowing relay table to lie on a diff database
 *
 * Revision 1.5  2001-11-24 22:20:32+05:30  Cprogrammer
 * int changed to long
 *
 * Revision 1.4  2001-11-24 12:21:24+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:59:56+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:27:33+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vclear_open_smtp.c,v 2.7 2010-05-05 14:39:57+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <mysqld_error.h>

#ifdef POP_AUTH_OPEN_RELAY
int
vclear_open_smtp(time_t clear_seconds, int connect_all)
{
	char            SqlBuf[SQL_BUF_SIZE];
	time_t          delete_time;
	char           *relay_table;
	int             err;
#ifdef CLUSTERED_SITE
	DBINFO        **ptr;
	MYSQL         **mysqlptr;
#endif

	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	delete_time = time(0) - clear_seconds;
	err = 0;
#ifndef CLUSTERED_SITE
	if (vauth_open((char *)0))
		return(1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where timestamp <= %ld",
		relay_table, delete_time);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
			return(0);
		}
		fprintf(stderr, "vclear_open_smtp: %s\n", mysql_error(&mysql[1]));
		return(1);
	}
#else
	if (!connect_all)
	{
		if (vauth_open((char *)0))
			return(1);
		snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where timestamp <= %ld",
			relay_table, delete_time);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			{
				create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
				return(0);
			}
			fprintf(stderr, "vclear_open_smtp: %s\n", mysql_error(&mysql[1]));
			return(1);
		}
		return (0);
	}
	if(OpenDatabases())
		return(1);
	for (mysqlptr = MdaMysql, ptr = RelayHosts;(*ptr);mysqlptr++, ptr++)
	{
		if((*ptr)->fd == -1)
		{
			fprintf(stderr, "mysql_real_connect %s %s %s %s %s %d fd %d: %s\n", 
				(*ptr)->domain, (*ptr)->server, (*ptr)->mdahost, (*ptr)->database, (*ptr)->user,
				(*ptr)->port, (*ptr)->fd, mysql_error((*mysqlptr)));
			continue;
		}
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from %s where timestamp <= %ld", relay_table, delete_time);
		if (mysql_query((*mysqlptr), SqlBuf))
		{
			if(mysql_errno((*mysqlptr)) == ER_NO_SUCH_TABLE)
			{
				vauth_init(1, *mysqlptr);
				create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
				mysql_close(*mysqlptr);
				is_open = 0;
				continue;
			}
			fprintf(stderr, "mysql_query: %s %s %s %s %s %d fd %d: %s: %s\n", 
				(*ptr)->domain, (*ptr)->server, (*ptr)->mdahost, (*ptr)->database, (*ptr)->user,
				(*ptr)->port, (*ptr)->fd, SqlBuf, mysql_error((*mysqlptr)));
			err = 1;
			continue;
		}
	}
#endif
	return(err);
}
#endif

void
getversion_vclear_open_smtp_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
