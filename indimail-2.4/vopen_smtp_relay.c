/*
 * $Log: vopen_smtp_relay.c,v $
 * Revision 2.9  2014-01-02 23:55:19+05:30  Cprogrammer
 * set delayed MySQL inserts if delayed_insert variable is non-zero
 *
 * Revision 2.8  2008-09-08 09:58:15+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.7  2008-05-28 17:41:48+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.6  2005-12-29 22:54:34+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.5  2004-05-22 22:33:48+05:30  Cprogrammer
 * renamed ip_addr to ipaddr
 *
 * Revision 2.4  2002-10-27 21:42:51+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-08-05 01:15:32+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.2  2002-08-03 04:39:10+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-06-21 20:39:38+05:30  Cprogrammer
 * update relay table with the real_domain when email address has alias domain
 *
 * Revision 1.10  2002-02-23 20:33:31+05:30  Cprogrammer
 * corrected code for ERR_NO_SUCH_TABLE check
 *
 * Revision 1.9  2001-12-22 18:18:06+05:30  Cprogrammer
 * create table only if mysql_errno is ER_NO_SUCH_TABLE
 *
 * Revision 1.8  2001-12-11 11:35:15+05:30  Cprogrammer
 * removed open_relay_db()
 *
 * Revision 1.7  2001-11-30 00:15:33+05:30  Cprogrammer
 * removed relay_table assignment
 *
 * Revision 1.6  2001-11-29 20:59:27+05:30  Cprogrammer
 * code change for initiating separate msyql connection enabling relay to be on a diff db
 *
 * Revision 1.5  2001-11-24 22:20:47+05:30  Cprogrammer
 * int changed to long
 *
 * Revision 1.4  2001-11-24 12:22:09+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:01:00+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:03+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vopen_smtp_relay.c,v 2.9 2014-01-02 23:55:19+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <mysqld_error.h>

#ifdef POP_AUTH_OPEN_RELAY
/*
 * Gets ipaddr from Env variable TCPREMOTEIP
 * Inserts ipaddr into relay table.
 */
int
vopen_smtp_relay(char *user, char *domain)
{
	char            SqlBuf[SQL_BUF_SIZE];
	char           *ipaddr, *relay_table, *real_domain;
	time_t          mytime;

	mytime = time(0);
	if (!(ipaddr = (char *) getenv("TCPREMOTEIP")))
		return(0);
	/*
	 * courier-imap mangles TCPREMOTEIP 
	 */
	if (ipaddr[0] == ':')
	{
		ipaddr += 2;
		while (*ipaddr != ':')
			++ipaddr;
		++ipaddr;
	}
	if (skip_relay(ipaddr))
		return(0);
	if (vauth_open((char *)0))
		return(0);
	getEnvConfigStr(&relay_table, "RELAY_TABLE", RELAY_DEFAULT_TABLE);
	if (!(real_domain = vget_real_domain(domain)))
		real_domain = domain;
	snprintf(SqlBuf, SQL_BUF_SIZE, delayed_insert ?
		"replace delayed into %s ( email, ipaddr, timestamp ) values ( \"%s@%s\", \"%s\", %ld )" :
		"replace into %s ( email, ipaddr, timestamp ) values ( \"%s@%s\", \"%s\", %ld )",
		relay_table, user, real_domain, ipaddr, mytime);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, relay_table, RELAY_TABLE_LAYOUT))
				return(0);
			if (!mysql_query(&mysql[1], SqlBuf))
				return(1);
		} 
		fprintf(stderr, "vopen_smtp_relay: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
		return(0);
	}
	return(1);
}
#endif /*- #ifdef POP_AUTH_OPEN_RELAY */

void
getversion_vopen_smtp_relay_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
