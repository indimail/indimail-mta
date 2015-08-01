/*
 * $Log: logmysql.c,v $
 * Revision 2.4  2008-09-08 09:48:16+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.3  2008-05-28 16:36:52+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.2  2002-10-27 21:23:12+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.1  2002-08-05 00:18:05+05:30  Cprogrammer
 * added mysql_escape()
 *
 * Revision 1.8  2002-08-03 04:32:09+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.7  2002-02-23 20:23:35+05:30  Cprogrammer
 * corrected bug with ER_NO_SUCH_TABLE check
 *
 * Revision 1.6  2001-12-21 02:21:26+05:30  Cprogrammer
 * create table when errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.5  2001-12-03 04:17:18+05:30  Cprogrammer
 * changed insert to insert low_priority
 *
 * Revision 1.4  2001-11-24 12:19:27+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 10:55:20+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:23:09+05:30  Cprogrammer
 * vauth_open for distibuted arch
 *
 * Revision 1.1  2001-10-24 18:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: logmysql.c,v 2.4 2008-09-08 09:48:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef ENABLE_MYSQL_LOGGING
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

int
logmysql(int verror, char *TheUser, char *TheDomain, char *ThePass, char *TheName, char *IpAddr, char *LogLine)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (vauth_open((char *) 0))
		return (-1);
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"insert low_priority into vlog set user=\"%s\",passwd=\"%s\",domain=\"%s\", \
		logon=\"%s\",remoteip=\"%s\",message=\"%s\",error=%i", 
		TheUser, ThePass, TheDomain, TheName, IpAddr, LogLine, verror);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if(create_table(ON_LOCAL, "vlog", VLOG_TABLE_LAYOUT))
				return(-1);
			if (!mysql_query(&mysql[1], SqlBuf))
				return (0);
		} 
		mysql_perror("logmysql: %s", SqlBuf);
		return(-1);
	}
	return (0);
}
#endif /*- #ifdef ENABLE_MYSQL_LOGGING */

void
getversion_logmysql_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
