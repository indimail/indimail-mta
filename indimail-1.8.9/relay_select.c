/*
 * $Log: relay_select.c,v $
 * Revision 2.7  2009-03-05 09:30:06+05:30  Cprogrammer
 * do not treat first time table missing as error
 *
 * Revision 2.6  2008-06-13 10:00:29+05:30  Cprogrammer
 * compile relay_select() only if POP_AUTH_OPEN_RELAY defined
 *
 * Revision 2.5  2008-05-28 16:37:34+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2005-12-29 22:49:19+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-12-28 17:36:05+05:30  Cprogrammer
 * added missing display of error message
 *
 * Revision 2.2  2004-05-22 22:29:48+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 2.1  2002-10-27 21:28:57+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 1.2  2002-08-03 04:32:39+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.1  2002-04-08 21:54:55+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: relay_select.c,v 2.7 2009-03-05 09:30:06+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"

#include <string.h>
#include <stdlib.h>
#include <mysqld_error.h>

#ifdef POP_AUTH_OPEN_RELAY
int
relay_select(char *email, char *remoteip)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ptr, *relay_table;
	int             len;
	long            timeout;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (vauth_open((char *) 0))
		return (-1);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	getEnvConfigStr(&ptr, "RELAY_CLEAR_MINUTES", RELAY_CLEAR_MINUTES);
	timeout = atoi(ptr) * 60;
	snprintf(SqlBuf, SQL_BUF_SIZE, "select email FROM %s WHERE ipaddr=\"%s\" AND timestamp>(UNIX_TIMESTAMP()-%ld)",
			 relay_table, remoteip, timeout);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT);
		else
			mysql_perror("relay_select: %s", SqlBuf);
		return (0);
	} 
	res = mysql_store_result(&mysql[1]);
	for (len = slen(email);(row = mysql_fetch_row(res));)
	{
		if (!strncasecmp(row[0], email, len))
		{
			mysql_free_result(res);
			return(1);
		}
	}
	mysql_free_result(res);
	return(0);
}
#endif

void
getversion_relay_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
