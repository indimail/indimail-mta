/*
 * $Log: vquota_select.c,v $
 * Revision 2.3  2009-10-14 20:48:06+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.2  2008-06-24 22:02:32+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.1  2008-05-28 17:42:07+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 1.6  2002-08-03 04:39:17+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.5  2001-12-23 00:49:55+05:30  Cprogrammer
 * error checking to prevent core dump
 *
 * Revision 1.4  2001-11-24 12:22:20+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:09+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:24+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:44+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vquota_select.c,v 2.3 2009-10-14 20:48:06+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef ENABLE_AUTH_LOGGING
#include <string.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>

int
vquota_select(char *user, char *domain, mdir_t *quota, time_t *timestamp, int len)
{
	char            SqlBuf[SQL_BUF_SIZE];
	static MYSQL_RES *quota_res;
	MYSQL_ROW       row;

	if(!quota_res)
	{
		if (vauth_open((char *) 0))
			return (-1);
		if(!*domain)
			snprintf(SqlBuf, SQL_BUF_SIZE, 
				"select high_priority user, domain, quota, \
				UNIX_TIMESTAMP(timestamp) from userquota where \
				quota != 0 and UNIX_TIMESTAMP(timestamp) < %ld", *timestamp);
		else
			snprintf(SqlBuf, SQL_BUF_SIZE,
				"select high_priority user, domain, quota, \
				UNIX_TIMESTAMP(timestamp) from userquota where \
				quota != 0 and UNIX_TIMESTAMP(timestamp) < %ld \
				and domain=\"%s\"", *timestamp, domain);
		if (mysql_query(&mysql[1], SqlBuf))
			return(-1);
		if(!(quota_res = mysql_store_result(&mysql[1])))
			return(0);
	} 
	if ((row = mysql_fetch_row(quota_res)))
	{
		scopy(user, row[0], len);
		scopy(domain, row[1], len);
		*quota = strtoll(row[2], 0, 0);
		*timestamp = atol(row[3]);
		return (1);
	}
	mysql_free_result(quota_res);
	quota_res = (MYSQL_RES *) 0;
	return (0);
}
#endif

void
getversion_vquota_select_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
