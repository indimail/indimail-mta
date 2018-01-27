/*
 * $Log: Login_Tasks.c,v $
 * Revision 2.30  2013-02-21 22:39:14+05:30  Cprogrammer
 * fixed typo (postauth->migrateuser) in error message
 *
 * Revision 2.29  2010-07-04 14:36:40+05:30  Cprogrammer
 * replaced open_smtp_relay() with vopen_smtp_relay()
 *
 * Revision 2.28  2010-05-06 13:25:57+05:30  Cprogrammer
 * fixed argument 'user' getting modified inside Login_Tasks()
 *
 * Revision 2.27  2009-10-14 20:43:17+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.26  2009-10-09 20:20:32+05:30  Cprogrammer
 * use defined CONSTANTS for vget_lastauth
 *
 * Revision 2.25  2009-09-23 15:00:06+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.24  2009-06-04 16:28:59+05:30  Cprogrammer
 * fixed syntax error
 *
 * Revision 2.23  2009-06-04 16:27:23+05:30  Cprogrammer
 * code optimized
 *
 * Revision 2.22  2008-11-20 22:13:44+05:30  Cprogrammer
 * set LAST_PASSWORD_CHANGE environment variable
 * return status of POSTAUTH, MIGRATEUSER commands
 *
 * Revision 2.21  2008-06-24 21:48:22+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.20  2008-06-13 10:13:45+05:30  Cprogrammer
 * fixed compilation warning due to ENABLE_AUTH_LOGGING turned off
 *
 * Revision 2.19  2008-06-13 09:49:33+05:30  Cprogrammer
 * fixed compilation errors with ENABLE_AUTH_LOGGING was not defined
 *
 * Revision 2.18  2007-12-22 00:16:35+05:30  Cprogrammer
 * changed location of BulkMail flag to Maildir
 *
 * Revision 2.17  2005-12-29 22:46:22+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.16  2005-07-04 21:08:52+05:30  Cprogrammer
 * send mail quota warning only for users not having NOQUOTA as the quota
 *
 * Revision 2.15  2004-07-12 22:46:31+05:30  Cprogrammer
 * replaced system() with runcmmd()
 *
 * Revision 2.14  2004-07-02 18:06:01+05:30  Cprogrammer
 * renamed .BulkMail to BulkMail, .QuotaWarn to QuotaWarn
 *
 * Revision 2.13  2003-11-03 10:41:49+05:30  Cprogrammer
 * do not display error if interval is less than MIN_LOGIN_INTERVAL
 *
 * Revision 2.12  2003-10-01 02:11:58+05:30  Cprogrammer
 * DOS prevention code MIN_LOGIN_INTERVAL
 *
 * Revision 2.11  2003-08-12 09:22:49+05:30  Cprogrammer
 * added program to be run post authentication
 *
 * Revision 2.10  2003-03-30 23:36:31+05:30  Cprogrammer
 * Subject contained junk for new users
 *
 * Revision 2.9  2003-03-27 20:37:16+05:30  Cprogrammer
 * added inactivation Time in subject for activation mail
 *
 * Revision 2.8  2003-03-24 19:16:41+05:30  Cprogrammer
 * set lastauth and other stuff if user is inactive
 *
 * Revision 2.7  2003-02-01 15:35:17+05:30  Cprogrammer
 * removed update_flag argument to user_over_quota()
 *
 * Revision 2.6  2003-01-17 00:14:05+05:30  Cprogrammer
 * create maildirsize/.current_size on activation
 *
 * Revision 2.5  2003-01-13 23:34:35+05:30  Cprogrammer
 * allow activation of users even if NOLASTAUTH is set
 *
 * Revision 2.4  2002-10-23 21:00:32+05:30  Cprogrammer
 * migrateuser was not being set correctly
 *
 * Revision 2.3  2002-08-01 23:23:01+05:30  Cprogrammer
 * changed case of user and domain to lower to prevent upper case entries in lastauth and relay table
 * send activation mail for user who becomes active
 *
 * Revision 2.2  2002-05-16 01:08:12+05:30  Cprogrammer
 * Added welcome mail delivery code
 *
 * Revision 2.1  2002-04-13 12:18:40+05:30  Cprogrammer
 * Remove BOUNCE_MAIL flag if a overquota user has released disk space
 *
 * Revision 1.11  2002-04-03 01:39:13+05:30  Cprogrammer
 * added authvchkpw - imap/pop3 code to Login_Tasks
 *
 * Revision 1.10  2002-03-03 11:47:52+05:30  Cprogrammer
 * added function RemoteBulkMail() (removed from bulk_mail()) to avoid
 * multiple invocations
 * Changed order of flags in open() for SNAP OS Bug
 *
 * Revision 1.9  2002-03-03 11:16:47+05:30  Cprogrammer
 * removed updation of .domain file on login
 *
 * Revision 1.8  2002-03-02 00:52:19+05:30  Cprogrammer
 * added variable to hold the fully qualified email (fqemail)
 *
 * Revision 1.7  2002-02-25 13:52:37+05:30  Cprogrammer
 * changed order of flags for SNAP OS Bug
 *
 * Revision 1.6  2002-02-24 03:22:17+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.5  2001-12-08 12:33:05+05:30  Cprogrammer
 * correction in copying bulk mail
 *
 * Revision 1.4  2001-11-28 23:43:49+05:30  Cprogrammer
 * .BulkMail flag created after all bulk mails are copied
 *
 * Revision 1.3  2001-11-24 12:17:00+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:17+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef	lint
static char     sccsid[] = "$Id: Login_Tasks.c,v 2.30 2013-02-21 22:39:14+05:30 Cprogrammer Stab mbhangui $";
#endif

int
Login_Tasks(pw, User, ServiceType)
	struct passwd  *pw;
	const char     *User;
	char           *ServiceType;
{
	char           *domain, *ptr, *migrateflag, *migrateuser, *postauth;
	char            fqemail[MAX_BUFF], Maildir[MAX_BUFF], tmpbuf[MAX_BUFF];
	char            pwbuf[MAX_BUFF], last_pass_change[MAX_BUFF], user[MAX_BUFF];
	struct stat     statbuf;
	int             status, flag = 0;
#ifdef ENABLE_AUTH_LOGGING
	time_t          inact_time, tmval1, tmval2, min_login_interval;
	char            Subject[MAX_BUFF];
#ifdef USE_MAILDIRQUOTA	
	mdir_t          size_limit, count_limit;
#endif
#endif

	if (!pw)
		return(1);
	strncpy(user, User, MAX_BUFF);
	lowerit((char *) user);
	lowerit(pw->pw_name);
	if (!(ptr = strchr(user, '@')))
	{
		getEnvConfigStr(&domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		lowerit(domain);
		snprintf(fqemail, MAX_BUFF, "%s@%s", user, domain);
	} else
	{
		domain = ptr + 1;
		scopy(fqemail, user, MAX_BUFF);
		*ptr = 0;
	}
	create_flag = 1;
	if (access(pw->pw_dir, F_OK))
	{
		vmake_maildir(pw->pw_dir, indimailuid, indimailgid, domain);
#ifdef ENABLE_AUTH_LOGGING
		if ((inact_time = vget_lastauth(pw, domain, INACT_TIME, 0)))
		{
			struct tm *tm;
			int    year;
			tm = localtime(&inact_time);
			year = tm->tm_year + 1900;
			snprintf(Subject, MAX_BUFF, "Your IndiMail Account was de-activated on %02d-%02d-%02d", tm->tm_mday, tm->tm_mon + 1, year);
			if ((ptr = strrchr(Subject, '\n')))
				*ptr = 0;
		} else
			*Subject = 0;
		SendWelcomeMail(pw->pw_dir, (char *) user, domain, (is_inactive && inact_time) ? 1 : 0, Subject);
#else
		SendWelcomeMail(pw->pw_dir, (char *) user, domain, 0, "");
#endif
	}
	snprintf(Maildir, MAX_BUFF, "%s/Maildir", pw->pw_dir);
#ifdef ENABLE_AUTH_LOGGING
	if (is_inactive)
	{
		vauth_active(pw, domain, FROM_INACTIVE_TO_ACTIVE);
#ifdef USE_MAILDIRQUOTA	
		if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
		{
			fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
			return (1);
		}
		(void) recalc_quota(Maildir, 0, size_limit, count_limit, 2);
#else
		(void) recalc_quota(Maildir, 2);
#endif
	} else
	if (getenv("NOLASTAUTHLOGGING") || getenv("NOLASTAUTH"))
		return(0);
#endif /*- ENABLE_AUTH_LOGGING */
#ifdef POP_AUTH_OPEN_RELAY
	/*- open the relay to pop3/imap users */
	if (!getenv("NORELAY") && (pw->pw_gid & NO_RELAY) == 0 && getenv("OPEN_SMTP")
			&& vopen_smtp_relay(pw->pw_name, domain))
		update_rules(1); /*- update tcp.smtp.cdb */
#endif
#ifdef ENABLE_AUTH_LOGGING
	getEnvConfigStr(&ptr, "MIN_LOGIN_INTERVAL", "0");
	min_login_interval = atoi(ptr);
	if (min_login_interval)
	{
		if ((tmval2 = time(0)) - (tmval1 = vget_lastauth(pw, domain, AUTH_TIME, 0)) < min_login_interval)
		{
			fprintf(stderr, "ERR: Login interval %ld < %ld, user=%s, domain=%s\n", 
				tmval2 - tmval1, min_login_interval, pw->pw_name, domain);
			return(2);
		}
	}
	tmval1 = vget_lastauth(pw, domain, PASS_TIME, 0);
	snprintf(last_pass_change, sizeof(last_pass_change), "LAST_PASSWORD_CHANGE=%ld", tmval1);
	putenv(last_pass_change);
#ifdef USE_MAILDIRQUOTA
	if ((ptr = (char *) getenv("TCPREMOTEIP")) || (ptr = GetIpaddr()))
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, ptr, pw->pw_gecos, check_quota(Maildir, 0));
	else
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, "unknown", pw->pw_gecos, check_quota(Maildir, 0));
#else
	if ((ptr = (char *) getenv("TCPREMOTEIP")) || (ptr = GetIpaddr()))
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, ptr, pw->pw_gecos, check_quota(Maildir));
	else
		vset_lastauth(pw->pw_name, domain, (char *) ServiceType, "unknown", pw->pw_gecos, check_quota(Maildir));
#endif /*- USE_MAILDIRQUOTA */
#endif /*- ENABLE_AUTH_LOGGING */
	if ((postauth = (char *) getenv("POSTAUTH")) && !access(postauth, X_OK))
	{
		snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=%s:%s:%d:%d:%s:%s:%s:%d", 
			fqemail,
			pw->pw_passwd,
			pw->pw_uid,
			pw->pw_gid,
			pw->pw_gecos,
			pw->pw_dir,
			pw->pw_shell, is_inactive);
		putenv(pwbuf);
		snprintf(tmpbuf, MAX_BUFF, "%s %s %s", postauth, fqemail, pw->pw_dir);
		if ((status = runcmmd(tmpbuf, 0)))
		{
			fprintf(stderr, "%s %s %s [status=%d]\n", postauth, fqemail, pw->pw_dir, status);
			return (status);
		}
	}
	getEnvConfigStr(&migrateuser, "MIGRATEUSER", MIGRATEUSER);
	getEnvConfigStr(&migrateflag, "MIGRATEFLAG", MIGRATEFLAG);
	snprintf(tmpbuf, MAX_BUFF, "%s/%s", pw->pw_dir, migrateflag);
	if (access(tmpbuf, F_OK) && !access(migrateuser, X_OK))
	{
		snprintf(pwbuf, sizeof(pwbuf), "PWSTRUCT=%s:%s:%d:%d:%s:%s:%s:%d", 
			fqemail,
			pw->pw_passwd,
			pw->pw_uid,
			pw->pw_gid,
			pw->pw_gecos,
			pw->pw_dir,
			pw->pw_shell, is_inactive);
		putenv(pwbuf);
		snprintf(tmpbuf, MAX_BUFF, "%s %s %s", migrateuser, fqemail, pw->pw_dir);
		if ((status = runcmmd(tmpbuf, 0)))
		{
			fprintf(stderr, "%s %s %s [status=%d]\n", migrateuser, fqemail, pw->pw_dir, status);
			return (status);
		}
	}
	/* - Copy Bulk Mails from Local BULK_MAILDIR directory */
	if (!bulk_mail(fqemail, domain, pw->pw_dir))
		flag = 1;
	if (!bulk_mail(fqemail, pw->pw_gecos, pw->pw_dir) || flag)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/BulkMail", pw->pw_dir);
		close(open(tmpbuf, O_CREAT | O_TRUNC , 0644));
	}
	RemoteBulkMail(fqemail, domain, pw->pw_dir);
	/*- 
	 * If the age of file QuotaWarn is more than a week send warning
	 * to user if overquota
	 */
	if (strncmp(pw->pw_shell, "NOQUOTA", 7))
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/QuotaWarn", pw->pw_dir);
		if ((stat(tmpbuf, &statbuf) ? time(0) : time(0) - statbuf.st_mtime) > 7 * 86400)
			MailQuotaWarn(pw->pw_name, domain, Maildir, pw->pw_shell);
	}
	/*- Remove Bounce Flag if user is under qutoa */
	if (pw->pw_gid & BOUNCE_MAIL && !user_over_quota(Maildir, pw->pw_shell, 0))
		vset_lastdeliver(pw->pw_name, domain, 0);
	return(0);
}

void
getversion_Login_Tasks_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
