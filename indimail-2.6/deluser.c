/*
 * $Log: deluser.c,v $
 * Revision 2.31  2019-03-16 19:26:43+05:30  Cprogrammer
 * removed mailing_list code
 *
 * Revision 2.30  2018-11-21 14:36:00+05:30  Cprogrammer
 * change for fstabChangeCounter
 *
 * Revision 2.29  2018-09-11 10:31:13+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.28  2016-05-18 12:43:34+05:30  Cprogrammer
 * added dir argument to del_user_assign()
 *
 * Revision 2.27  2016-01-12 14:26:36+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.26  2010-03-02 08:17:32+05:30  Cprogrammer
 * changed Username xxx@yyy does not exist to xxx@yyy: No such user
 *
 * Revision 2.25  2009-10-14 20:42:48+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.24  2009-10-09 20:19:55+05:30  Cprogrammer
 * use defined CONSTANTS for vget_lastauth
 *
 * Revision 2.23  2008-09-14 19:42:42+05:30  Cprogrammer
 * removed setgid, setuid calls
 *
 * Revision 2.22  2008-08-02 09:06:53+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.21  2008-06-24 21:47:44+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.20  2008-06-13 10:12:40+05:30  Cprogrammer
 * compile userquota code only if ENABLE_AUTH_LOGGING defined
 *
 * Revision 2.19  2008-06-13 08:48:27+05:30  Cprogrammer
 * conditional compilation for ENABLE_AUTH_LOGGING, VALIAS
 *
 * Revision 2.18  2008-05-28 16:35:14+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.17  2004-11-16 11:10:46+05:30  Cprogrammer
 * remove all entries from lastauth when deleting user
 *
 * Revision 2.16  2004-07-02 20:42:00+05:30  Cprogrammer
 * remove forwarding to the user in valias
 *
 * Revision 2.15  2004-06-24 19:59:20+05:30  Cprogrammer
 * correction for adding local users
 *
 * Revision 2.14  2004-05-19 20:00:08+05:30  Cprogrammer
 * new logic for replacing '.' with ':'
 *
 * Revision 2.13  2004-05-17 14:00:57+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.12  2004-04-20 10:53:12+05:30  Cprogrammer
 * replace '.' with ':' for .qmail files
 *
 * Revision 2.11  2003-12-08 20:22:01+05:30  Cprogrammer
 * create lastauth table if it does not exist
 *
 * Revision 2.10  2003-07-02 18:23:51+05:30  Cprogrammer
 * correction of logic of treating missing table
 *
 * Revision 2.9  2003-06-19 17:11:41+05:30  Cprogrammer
 * remove entries from table only if remove_db is not null
 *
 * Revision 2.8  2003-06-18 23:09:15+05:30  Cprogrammer
 * added deletion of records from lastauth, vfilter, mailinglist on user inactivation
 *
 * Revision 2.7  2003-03-30 23:35:20+05:30  Cprogrammer
 * allow users starting with a numeral
 *
 * Revision 2.6  2003-01-12 21:35:50+05:30  Cprogrammer
 * change uid only if uid is root or neither indimail uid or id in assign file
 *
 * Revision 2.5  2002-10-18 01:17:22+05:30  Cprogrammer
 * use the array rfc_ids[] to check for mandatory RFC821 ids
 *
 * Revision 2.4  2002-08-11 16:46:38+05:30  Cprogrammer
 * update fstab only if user has been activated once
 *
 * Revision 2.3  2002-08-11 00:56:53+05:30  Cprogrammer
 * added call to fstabChangeCounter()
 *
 * Revision 2.2  2002-06-21 20:50:24+05:30  Cprogrammer
 * prevent postmaster and abuse ids from getting disabled
 *
 * Revision 2.1  2002-05-06 22:14:24+05:30  Cprogrammer
 * minor change in error format
 *
 * Revision 1.13  2001-12-22 18:06:46+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.12  2001-12-11 11:30:49+05:30  Cprogrammer
 * open connection to master for updates
 *
 * Revision 1.11  2001-12-09 02:17:20+05:30  Cprogrammer
 * use islocalif() to check if mailstore is local ip or not
 *
 * Revision 1.10  2001-12-08 00:33:33+05:30  Cprogrammer
 * vauth_active first argument changed to passwd structure
 *
 * Revision 1.9  2001-12-02 20:19:44+05:30  Cprogrammer
 * conditional compilation for mysql
 *
 * Revision 1.8  2001-12-02 18:41:49+05:30  Cprogrammer
 * additional argument Dir passed to dec_dir_control
 * enabling it to decrement filesytem specific users
 *
 * Revision 1.7  2001-11-29 20:51:06+05:30  Cprogrammer
 * conditional compilation for distributed arch
 *
 * Revision 1.6  2001-11-29 13:17:56+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.5  2001-11-28 22:57:42+05:30  Cprogrammer
 * 1. Change because of vdelfiles() function change
 * 2. Distributed architecture code added
 *
 * Revision 1.4  2001-11-24 20:36:44+05:30  Cprogrammer
 * dir_control not to be modified when making a user inactive
 *
 * Revision 1.3  2001-11-24 12:18:55+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:20+05:30  Cprogrammer
 * Added option for making users inactive
 *
 * Revision 1.1  2001-10-24 18:14:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mysqld_error.h>
#ifndef USE_MAILDIRQUOTA
#include <stdlib.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: deluser.c,v 2.31 2019-03-16 19:26:43+05:30 Cprogrammer Exp mbhangui $";
#endif

/*-
 * Function to remove users, depending on the value of remove_db
 * 0 - Only Remove the directory
 * 1 - Remove the directory and entry from indimail database
 * 2 - Remove the directory and move the entry from indimail to indibak
 -*/
int
vdeluser(char *user, char *domain, int remove_db)
{
	struct passwd  *passent;
	char            Dir[MAX_BUFF], TmpBuf[MAX_BUFF + 9], SqlBuf[SQL_BUF_SIZE];
	char           *real_domain, *ptr;
	char            ch;
	mdir_t          quota;
#ifdef CLUSTERED_SITE
	char           *mailstore;
	int             err;
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;

	if (!user || !*user || !isalnum((int) *user)) {
		error_stack(stderr, "Illegal Username\n");
		return (-1);
	}
	if (remove_db == 2) {
		for(i = 0;rfc_ids[i];i++) {
			if (!strncmp(user, rfc_ids[i], slen(rfc_ids[i]) + 1)) {
				error_stack(stderr, "RFC Ids cannot be made inactive\n");
				return (-1);
			}
		}
	} else {
		for(i = 0;rfc_ids[i];i++) {
			if (!strncmp(user, rfc_ids[i], slen(rfc_ids[i]) + 1)) {
				printf("Are You sure removing RFC Id %s (y/n) - ", user);
				ch = getchar();
				if (ch != 'y' && ch != 'Y')
					return(-1);
				break;
			}
		}
	}
	umask(INDIMAIL_UMASK);
	lowerit(user);
	if (domain && *domain)
		lowerit(domain);
	if (domain && *domain) {
		if (!(real_domain = vget_real_domain(domain))) {
			error_stack(stderr, "Domain %s does not exist\n", domain);
			return (-1);
		} else
		if (!vget_assign(real_domain, Dir, MAX_BUFF, &uid, &gid)) {
			error_stack(stderr, "Domain %s does not exist\n", real_domain);
			return (-1);
		}
#ifdef CLUSTERED_SITE
		if ((err = is_distributed_domain(real_domain)) == -1) {
			error_stack(stderr, "Unable to verify %s as a distributed domain\n", real_domain);
			return (-1);
		} else
		if (err == 1) {
			if (open_master()) {
				error_stack(stderr, "vdeluser: Failed to Open Master Db\n");
				return (-1);
			}
			snprintf(TmpBuf, MAX_BUFF, "%s@%s", user, real_domain);
			if ((mailstore = findhost(TmpBuf, 0)) != (char *) 0) {
				if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
					*ptr = 0;
				for(;*mailstore && *mailstore != ':';mailstore++);
				mailstore++;
			} else {
				if (userNotFound)
					error_stack(stderr, "%s@%s: No such user\n", user, real_domain);
				else
					error_stack(stderr, "Error connecting to db\n");
				return (-1);
			}
			if (!islocalif(mailstore)) {
				error_stack(stderr, "%s@%s not local (mailstore %s)\n",
					user, real_domain, mailstore);
				return(-1);
			}
		}
#endif
		if (!(passent = vauth_getpw(user, real_domain))) {
			if (userNotFound)
				error_stack(stderr, "%s@%s: No such user\n", user, real_domain);
			else
				error_stack(stderr, "Error connecting to db\n");
			return (-1);
		}
		scopy(Dir, passent->pw_dir, MAX_BUFF);
		switch(remove_db)
		{
			case 1: /*- Delete User */
				if (vauth_deluser(user, real_domain)) {
					error_stack(stderr, "vdeluser: Failed to remove user %s@%s\n",
						user, real_domain);
					return (-1);
				}
#ifdef USE_MAILDIRQUOTA
				quota = parse_quota(passent->pw_shell, 0);
#else
				quota = atol(passent->pw_shell);
#endif
				if (quota == -1) {
					fprintf(stderr, "vdeluser: parse_quota: %s: %s\n", passent->pw_shell, strerror(errno));
					return (-1);
				}
#ifdef ENABLE_AUTH_LOGGING
				if (vget_lastauth(passent, real_domain, ACTIV_TIME, 0))
					fstabChangeCounter(passent->pw_dir, 0, -1, 0 - quota);
				snprintf(SqlBuf, SQL_BUF_SIZE, 
					"delete low_priority from lastauth where user=\"%s\" and domain=\"%s\"",
					user, real_domain);
				if (mysql_query(&mysql[1], SqlBuf)) {
					if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
						if (create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT))
							return(-1);
					} else {
						fprintf(stderr, "vdeluser: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
						return(-1);
					}
				}
#endif
				break;
			case 2: /*- Make user inactive */
#ifdef ENABLE_AUTH_LOGGING
				if (vauth_active(passent, real_domain, FROM_ACTIVE_TO_INACTIVE)) {
					error_stack(stderr, "vdeluser: Failed to mark user %s@%s as inactive\n", 
						user, real_domain);
					return (-1);
				} 
#else
				error_stack(stderr,
					"vdeluser: cannot mark user %s@%s as inactive.\nIndiMail not configured with ENABLE_AUTH_LOGGING\n", 
					user, real_domain);
				return (-1);
#endif
				break;
		}
		if (remove_db) {
#ifdef VFILTER
			snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from vfilter where emailid=\"%s@%s\"",
				user, real_domain);
			if (mysql_query(&mysql[1], SqlBuf)) {
				if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
						return(-1);
				} else {
					fprintf(stderr, "vdeluser: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
					return (-1);
				}
			}
#endif
#ifdef ENABLE_AUTH_LOGGING
			snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from userquota where user=\"%s\" and domain=\"%s\"", 
				user, real_domain);
			if (mysql_query(&mysql[1], SqlBuf)) {
				if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
						return(-1);
				} else {
					fprintf(stderr, "vdeluser: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
					return (-1);
				}
			}
#endif
#ifdef VALIAS
			/*- Remove forwardings to this email address */
			snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from valias where valias_line=\"&%s@%s\"", 
				user, real_domain);
			if (mysql_query(&mysql[1], SqlBuf)) {
				if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE) {
					if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
						return(-1);
				} else {
					fprintf(stderr, "vdeluser: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
					return (-1);
				}
			}
			if (valias_delete(user, real_domain, (char *) 0)) {
				error_stack(stderr, "vdeluser: Failed to remove aliases for user %s@%s\n",
					user, real_domain);
				return (-1);
			}
#endif
		}
	} else {
		if (!vget_assign(user, Dir, MAX_BUFF, &uid, &gid)) {
			error_stack(stderr, "%s: No such user\n", user);
			return (-1);
		}
		if (remove_db && del_user_assign(user, Dir))
			return (-1);
		real_domain = NULL;
	}
	if (remove_db == 1)
		dec_dir_control(Dir, user, real_domain, -1, -1);
	/*
	 * remove the users directory from the file system 
	 * and check for error
	 */
	if (vdelfiles(Dir, user, real_domain)) {
		error_stack(stderr, "Failed to remove Dir %s: %s\n", Dir, strerror(errno));
		return (-1);
	}
	snprintf(TmpBuf, sizeof(TmpBuf), "%s/.qmail-%s", Dir, user);
	/* replace all dots with ':' */
	for(ptr = TmpBuf + slen(Dir) + 8;*ptr;ptr++) {
		if (*ptr == '.')
			*ptr = ':';
	}
	if (!access(TmpBuf, F_OK)) {
		if (verbose)
			printf("Removing file %s\n", TmpBuf);
		unlink(TmpBuf);
	}
	return (0);
}

void
getversion_deluser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
