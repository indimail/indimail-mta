/*
 * $Log: vupdate_rules.c,v $
 * Revision 2.5  2009-02-18 21:36:16+05:30  Cprogrammer
 * check write for error
 *
 * Revision 2.4  2008-05-28 17:42:58+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.3  2005-12-29 22:53:49+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-05-22 22:34:34+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 2.1  2002-10-27 21:46:15+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.9  2002-02-23 20:27:44+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.8  2001-12-22 18:19:39+05:30  Cprogrammer
 * create table on if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.7  2001-12-11 11:36:34+05:30  Cprogrammer
 * removed open_relay_db()
 *
 * Revision 1.6  2001-11-30 00:15:52+05:30  Cprogrammer
 * removed relay_table assignment
 *
 * Revision 1.5  2001-11-29 21:00:41+05:30  Cprogrammer
 * code change for relay table to be on a different server
 *
 * Revision 1.4  2001-11-24 12:22:38+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:22+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:58+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: vupdate_rules.c,v 2.5 2009-02-18 21:36:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#include <mysqld_error.h>

int
vupdate_rules(int fdm)
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_ROW       row;
	MYSQL_RES      *res;
	char           *relay_table;

	if (vauth_open((char *)0))
		return(1);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority ipaddr from %s", relay_table);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
				return(1);
			if (mysql_query(&mysql[1], SqlBuf))
			{
				fprintf(stderr, "vupdate_rules: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
				return(1);
			}
		} else
		{
			fprintf(stderr, "vupdate_rules: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
			return(1);
		}
	}
	if (!(res = mysql_store_result(&mysql[1])))
	{
		fprintf(stderr, "vupdate_rules: mysql_store_result: %s\n", mysql_error(&mysql[1]));
		return(1);
	}
	while ((row = mysql_fetch_row(res)))
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, "%s:allow,RELAYCLIENT=\"\"\n", row[0]);
		if (write(fdm, SqlBuf, slen(SqlBuf)) == -1)
		{
			fprintf(stderr, "vupdate_rules: write: %s\n", strerror(errno));
			mysql_free_result(res);
			return(1);
		}
	}
	mysql_free_result(res);
	return(0);
}
#endif

void
getversion_vupdate_rules_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
