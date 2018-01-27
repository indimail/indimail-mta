/*
 * $Log: vauth_active.c,v $
 * Revision 2.10  2016-01-12 14:27:08+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.9  2009-10-14 20:46:19+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.8  2008-09-08 09:55:25+05:30  Cprogrammer
 * removed mysql_escape
 * changes for using mysql_real_escape_string
 *
 * Revision 2.7  2008-06-24 21:59:43+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.6  2008-05-28 16:38:43+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.5  2002-08-11 00:36:47+05:30  Cprogrammer
 * update fstab counters
 *
 * Revision 2.4  2002-08-05 01:05:55+05:30  Cprogrammer
 * added mysql_escape for username
 *
 * Revision 2.3  2002-08-03 04:34:08+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.2  2002-08-03 03:09:29+05:30  Cprogrammer
 * conditional deletion of imap and pop3 lastauth record
 *
 * Revision 2.1  2002-08-03 02:35:05+05:30  Cprogrammer
 * check_quota() not to be called when making a user inactive
 *
 * Revision 1.6  2002-04-03 13:31:42+05:30  Cprogrammer
 * set is_inactive when changing the inactive status of a user
 *
 * Revision 1.5  2002-02-24 04:12:54+05:30  Cprogrammer
 * added code for MAILDROP Maildir quota
 *
 * Revision 1.4  2001-12-08 00:39:07+05:30  Cprogrammer
 * vauth_active to remove only imap and pop3 entries from lastauth
 *
 * Revision 1.3  2001-11-24 12:20:45+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-24 01:45:43+05:30  Cprogrammer
 * no rows found in lastauth not to be considered as an error
 *
 * Revision 1.1  2001-11-19 12:20:18+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vauth_active.c,v 2.10 2016-01-12 14:27:08+05:30 Cprogrammer Stab mbhangui $";
#endif


#ifdef ENABLE_AUTH_LOGGING
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <sys/socket.h>

int
vauth_active(struct passwd *pw, char *domain, int type)
{
	char           *table1 = NULL, *table2 = NULL, *local_ip;
	int             row_count;
	mdir_t          quota;
	char            SqlBuf[SQL_BUF_SIZE], Dir[MAX_BUFF];

	userNotFound = 0;
	if(site_size == LARGE_SITE || !domain || !*domain || vauth_open((char *) 0))
		return(1);
	if(type != FROM_INACTIVE_TO_ACTIVE && type != FROM_ACTIVE_TO_INACTIVE)
		return(1);
	if(type == FROM_INACTIVE_TO_ACTIVE)
	{
		table1  = inactive_table;
		table2 = default_table;
	} else
	if(type == FROM_ACTIVE_TO_INACTIVE)
	{
		table2  = inactive_table;
		table1 = default_table;
		snprintf(SqlBuf, SQL_BUF_SIZE, "CREATE TABLE IF NOT EXISTS %s ( %s )", inactive_table, SMALL_TABLE_LAYOUT);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_active: %s", SqlBuf);
			return (1);
		}
	}
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"insert low_priority into %s select high_priority * from %s where pw_name=\"%s\" and pw_domain=\"%s\"",
		table2, table1, pw->pw_name, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_moverecord: %s", SqlBuf);
		return (1);
	}
	row_count = mysql_affected_rows(&mysql[1]);
	if(row_count == -1 || !row_count)
		return(1);
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from %s where pw_name=\"%s\" and pw_domain=\"%s\"",
			table1, pw->pw_name, domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		mysql_perror("vauth_moverecord: %s", SqlBuf);
		return (1);
	}
	row_count = mysql_affected_rows(&mysql[1]);
#ifdef DELETE_AUTH_RECORD
	if(type == FROM_ACTIVE_TO_INACTIVE)
	{
		snprintf(SqlBuf, SQL_BUF_SIZE, 
			"delete low_priority from lastauth where user = \"%s\" and domain = \"%s\" \
			and (service = \"pop3\" or service=\"imap\")", 
			pw->pw_name, domain);
		if (mysql_query(&mysql[1], SqlBuf))
		{
			mysql_perror("vauth_active-lastauth: %s", SqlBuf);
			return(1);
		}
	}
#endif
	if(row_count == -1 || !row_count)
		return(1);
	if(type == FROM_ACTIVE_TO_INACTIVE)
		is_inactive = 1;
	else
	if(type == FROM_INACTIVE_TO_ACTIVE)
		is_inactive = 0;
	snprintf(Dir, MAX_BUFF, "%s/Maildir", pw->pw_dir);
#ifdef USE_MAILDIRQUOTA
	if ((quota = parse_quota(pw->pw_shell, 0)) == -1)
	{
		fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
		return (-1);
	}
	vset_lastauth(pw->pw_name, domain, ((type == FROM_ACTIVE_TO_INACTIVE) ? "INAC" : "ACTI"), GetIpaddr(), 
			pw->pw_gecos, (type == FROM_ACTIVE_TO_INACTIVE) ? 0 : check_quota(Dir, 0));
#else
	quota = atol(pw->pw_shell);
	vset_lastauth(pw->pw_name, domain, ((type == FROM_ACTIVE_TO_INACTIVE) ? "INAC" : "ACTI"), GetIpaddr(), 
			pw->pw_gecos, (type == FROM_ACTIVE_TO_INACTIVE) ? 0 : check_quota(Dir));
#endif
	if(!(local_ip = get_local_ip(PF_INET)))
	{
		fprintf(stderr, "vauth_active: get_local_ip: %s\n", strerror(errno));
		return(-1);
	}
	if(type == FROM_ACTIVE_TO_INACTIVE)
		fstabChangeCounter(pw->pw_dir, local_ip, -1, 0 - quota);
	else
	if(type == FROM_INACTIVE_TO_ACTIVE)
		fstabChangeCounter(pw->pw_dir, local_ip, 1, quota);
	return(0);
}
#endif

void
getversion_vauth_active_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
