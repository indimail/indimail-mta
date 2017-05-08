/*
 * $Log: vget_lastauth.c,v $
 * Revision 2.8  2009-10-09 20:20:50+05:30  Cprogrammer
 * use defined CONSTANTS for vget_lastauth
 *
 * Revision 2.7  2008-09-08 09:57:54+05:30  Cprogrammer
 * removed mysql_escape
 * changes for using mysql_real_escape_string on sql queries
 *
 * Revision 2.6  2008-05-28 15:33:14+05:30  Cprogrammer
 * removed cdb code
 *
 * Revision 2.5  2002-12-03 03:04:10+05:30  Cprogrammer
 * added missing break
 *
 * Revision 2.4  2002-12-02 01:49:30+05:30  Cprogrammer
 * included service 'webm' for authentication
 *
 * Revision 2.3  2002-10-27 21:40:53+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.2  2002-08-05 01:14:26+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.1  2002-08-03 04:38:40+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.10  2002-02-23 23:57:02+05:30  Cprogrammer
 * added code to return the ip address
 *
 * Revision 1.9  2001-12-22 18:17:39+05:30  Cprogrammer
 * create table only if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.8  2001-12-08 00:36:13+05:30  Cprogrammer
 * added new cases , for passwd changes, activation and inactivation timestamps
 *
 * Revision 1.7  2001-12-02 20:23:02+05:30  Cprogrammer
 * added dummy argument to make function consistent with mysql version
 *
 * Revision 1.6  2001-11-24 12:21:58+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.5  2001-11-23 20:56:22+05:30  Cprogrammer
 * added option to retrieve both add and authentication records
 *
 * Revision 1.4  2001-11-23 00:15:05+05:30  Cprogrammer
 * return -1 on error since 0 is a valid time value
 *
 * Revision 1.3  2001-11-20 11:00:47+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:28:53+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vget_lastauth.c,v 2.8 2009-10-09 20:20:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

time_t
vget_lastauth(struct passwd *pw, char *domain, int type, char *ipaddr)
{
	int             err;
	time_t          tmval1, tmval2;
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if(ipaddr)
		*ipaddr = 0;
	if ((err = vauth_open((char *) 0)) != 0)
		return (-1);
	switch (type)
	{
		case AUTH_TIME: /*- Last Authentication */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and (service = \"pop3\" or service=\"imap\" or service=\"webm\")", pw->pw_name, domain);
		break;
		case CREAT_TIME: /*- User Creation Date */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service=\"add\"", pw->pw_name, domain);
		break;
		case PASS_TIME: /*- Last password change */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service=\"pass\"", pw->pw_name, domain);
		break;
		case ACTIV_TIME: /*- Activation date */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service=\"ACTI\"", pw->pw_name, domain);
		break;
		case INACT_TIME: /*- Inactivation date */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service=\"INAC\"", pw->pw_name, domain);
		break;
		case POP3_TIME: /*- Last POP3 access */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service = \"pop3\"", pw->pw_name, domain);
		break;
		case IMAP_TIME: /*- Last IMAP access */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service = \"imap\"", pw->pw_name, domain);
		break;
		case WEBM_TIME: /*- Last WEB access */
			snprintf(SqlBuf, SQL_BUF_SIZE, 
			"select high_priority UNIX_TIMESTAMP(timestamp), remote_ip from lastauth where user=\"%s\" and domain=\"%s\" \
			and service = \"webm\"", pw->pw_name, domain);
		break;
	}
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if(mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
			create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT);
		else
			mysql_perror("vget_lastauth: mysql_query: %s", SqlBuf);
		return(0);
	}
	res = mysql_store_result(&mysql[1]);
	tmval1 = 0;
	while ((row = mysql_fetch_row(res)))
	{
		tmval2 = atol(row[0]);
		if(tmval2 > tmval1)
		{
			tmval1 = tmval2;
			if(ipaddr)
				scopy(ipaddr, row[1], 18);
		}
	}
	mysql_free_result(res);
	return (tmval1);
}
#endif

void
getversion_vget_lastauth_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
