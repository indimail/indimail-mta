/*
 * $Log: addusercntrl.c,v $
 * Revision 2.8  2016-05-17 15:38:35+05:30  Cprogrammer
 * fixed comments
 *
 * Revision 2.7  2009-02-06 11:35:35+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.6  2008-09-08 09:20:12+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.5  2008-05-28 16:33:16+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.4  2004-05-17 01:06:56+05:30  Cprogrammer
 * added force flag to bypass distributed domain check
 *
 * Revision 2.3  2004-05-17 00:46:48+05:30  Cprogrammer
 * added hostid argument to addusercntrl()
 *
 * Revision 2.2  2003-01-03 02:44:26+05:30  Cprogrammer
 * replaced vcreate_cntrl_table() with create_table()
 *
 * Revision 2.1  2002-08-05 00:02:28+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 1.13  2002-08-03 04:25:38+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 1.12  2002-03-29 22:07:50+05:30  Cprogrammer
 * add hostid instead of ip address
 *
 * Revision 1.11  2002-03-19 20:37:35+05:30  Cprogrammer
 * removed erroneous comment
 *
 * Revision 1.10  2002-02-23 20:20:29+05:30  Cprogrammer
 * moved code for deleting from local indimail to adduser(). This code was executed when dup entry
 * was found in hostcntrl
 *
 * Revision 1.9  2001-12-26 23:27:40+05:30  Cprogrammer
 * ER_DUP_ENTRY check to be skipped for alias entries
 *
 * Revision 1.8  2001-12-22 18:06:01+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.7  2001-12-14 13:50:08+05:30  Cprogrammer
 * prevent race condition where user gets added in indimail
 * and another process adds entry in hostcntrl
 *
 * Revision 1.6  2001-12-12 13:42:42+05:30  Cprogrammer
 * changed open_central_db to open_master for updates
 *
 * Revision 1.5  2001-12-11 11:30:19+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.4  2001-12-08 23:49:12+05:30  Cprogrammer
 * removed addition of port in host column of hostcntrl table
 *
 * Revision 1.3  2001-11-30 00:10:54+05:30  Cprogrammer
 * used variable cntrl_table for hostcntrl table
 *
 * Revision 1.2  2001-11-29 00:31:02+05:30  Cprogrammer
 * replaced snprintf with fprintf to print error messages on stdout
 *
 * Revision 1.1  2001-11-28 22:46:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: addusercntrl.c,v 2.8 2016-05-17 15:38:35+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysqld_error.h>

/* 
 * To add an entry into the Location DB.
 *  2 - User Exists
 *  1 - Mysql Error (or) Assignment Error
 *  0 - Success
 */

int
addusercntrl(char *user, char *domain, char *hostid, char *pass, int force)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if (!user || !*user || !domain || !*domain || !pass || !*pass)
		return (1);
	/*
	 *  Check if Domain is distributed or not, by checking table hostcntrl
	 */
	if (!force)
	{
		if ((err = is_distributed_domain(domain)) == -1)
			return(1);
		if (!err)
			return(0);
	}
	if (open_master())
	{
		fprintf(stderr, "addusercntrl: Failed to open Master Db\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"insert low_priority into %s (pw_name, pw_domain, pw_passwd, host, timestamp) \
		 values(\"%s\", \"%s\", \"%s\", \"%s\", FROM_UNIXTIME(%lu))",
		 cntrl_table, user, domain, pass, hostid, time(0));
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if ((err = mysql_errno(&mysql[0])) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, cntrl_table, CNTRL_TABLE_LAYOUT))
				return(1);
			if (!mysql_query(&mysql[0], SqlBuf))
				return(0);
		}
		fprintf(stderr, "addusercntrl: %s: %s", SqlBuf, mysql_error(&mysql[0]));
		if (err == ER_DUP_ENTRY)
			return(2);
		return(1);
	}
	return (0);
}
#endif

void
getversion_addusercntrl_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
