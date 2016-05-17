/*
 * $Log: userinfo.c,v $
 * Revision 2.40  2016-05-17 17:09:39+05:30  mbhangui
 * use control directory set by configure
 *
 * Revision 2.39  2016-01-21 12:57:58+05:30  Cprogrammer
 * use localip if MdaServer returns null
 *
 * Revision 2.38  2016-01-19 00:33:23+05:30  Cprogrammer
 * datatype for DisplayFilter was missing
 *
 * Revision 2.37  2016-01-12 14:27:02+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.36  2013-06-10 16:06:09+05:30  Cprogrammer
 * set maildir in the correct block
 *
 * Revision 2.35  2013-04-29 22:50:44+05:30  Cprogrammer
 * fixed display of users with quota = NOQUOTA
 *
 * Revision 2.34  2012-04-22 17:17:17+05:30  Cprogrammer
 * display quota in Gb if quota > 1 Gb
 *
 * Revision 2.33  2011-02-11 23:01:46+05:30  Cprogrammer
 * fix for displaying quota, counts > 2Gb
 *
 * Revision 2.32  2010-06-14 21:25:31+05:30  Cprogrammer
 * show mailbox path as remote if user account is on remote machine
 *
 * Revision 2.31  2010-05-29 16:52:33+05:30  Cprogrammer
 * do not calculate quota for remote mailstore
 *
 * Revision 2.30  2009-10-14 20:45:45+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.29  2009-10-09 20:20:40+05:30  Cprogrammer
 * use defined CONSTANTS for vget_lastauth
 *
 * Revision 2.28  2009-09-26 00:02:02+05:30  Cprogrammer
 * display status as missing if home directory is absent
 *
 * Revision 2.27  2008-11-13 21:01:53+05:30  Cprogrammer
 * added display of password hash
 *
 * Revision 2.26  2008-07-13 19:48:51+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.25  2008-06-24 21:59:21+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.24  2008-06-13 10:32:22+05:30  Cprogrammer
 * fixed compilation warnings if ENABLE_AUTH_LOGGING was not defined
 *
 * Revision 2.23  2005-12-29 22:50:53+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.22  2005-12-21 09:52:10+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.21  2004-09-20 19:54:58+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.20  2004-07-02 18:10:14+05:30  Cprogrammer
 * renamed .vfilter to vfilter, .deliveryCount to deliveryCount
 * Added display of Total Mail size delivered in a day
 *
 * Revision 2.19  2003-10-24 00:44:56+05:30  Cprogrammer
 * added NO_SMTP and V_OVERRIDE
 *
 * Revision 2.18  2003-10-23 13:51:25+05:30  Cprogrammer
 * DisplayMail to be explicitely specified
 *
 * Revision 2.17  2003-06-04 23:45:58+05:30  Cprogrammer
 * added display of mail delivery stats
 *
 * Revision 2.16  2003-02-27 23:56:08+05:30  Cprogrammer
 * changes for MAD
 *
 * Revision 2.15  2003-02-08 21:22:10+05:30  Cprogrammer
 * display last imap, last pop3 and activation date by default
 *
 * Revision 2.14  2003-01-17 01:06:00+05:30  Cprogrammer
 * added code to work on relay servers
 *
 * Revision 2.13  2002-12-01 18:52:34+05:30  Cprogrammer
 * added unread, unseen message count
 *
 * Revision 2.12  2002-11-26 20:33:01+05:30  Cprogrammer
 * more meaningful text for mail store id
 *
 * Revision 2.11  2002-11-18 12:41:22+05:30  Cprogrammer
 * change for raw display option in vfilter_display
 *
 * Revision 2.10  2002-11-13 13:34:00+05:30  Cprogrammer
 * added filter_name in vfilter
 *
 * Revision 2.9  2002-10-24 01:14:48+05:30  Cprogrammer
 * corrected placement of #ifdef VFILTER
 *
 * Revision 2.8  2002-10-14 20:58:01+05:30  Cprogrammer
 * added variable for storing forwarding address in vfilter_display()
 *
 * Revision 2.7  2002-10-12 22:59:50+05:30  Cprogrammer
 * correction in display of Sql Database for clustered and non-clustered environment
 * corrected compilation errors in non-clustered environment
 *
 * Revision 2.6  2002-10-11 20:03:05+05:30  Cprogrammer
 * added option to display filters
 *
 * Revision 2.5  2002-08-05 00:32:42+05:30  Cprogrammer
 * corrected display of ip addresses
 *
 * Revision 2.4  2002-08-03 23:22:14+05:30  Cprogrammer
 * added POP3 and IMAP authentication times
 *
 * Revision 2.3  2002-08-03 00:38:19+05:30  Cprogrammer
 * corrected problem with wrong display of Sql Database IP
 *
 * Revision 2.2  2002-06-26 03:18:10+05:30  Cprogrammer
 * correction for unused variable if USE_MAILDIRQUOTA was not defined
 *
 * Revision 2.1  2002-05-12 01:21:59+05:30  Cprogrammer
 * code beautified at line 233
 *
 * Revision 1.26  2002-03-29 23:31:50+05:30  Cprogrammer
 * added option to display hostid
 *
 * Revision 1.25  2002-03-28 23:57:18+05:30  Cprogrammer
 * added display of Mysql Database IP address/hostname
 *
 * Revision 1.24  2002-03-24 19:13:38+05:30  Cprogrammer
 * recalculate quota by default
 *
 * Revision 1.23  2002-03-03 15:41:43+05:30  Cprogrammer
 * added display of quota in Mb
 *
 * Revision 1.22  2002-02-25 13:55:19+05:30  Cprogrammer
 * conditional compilation for CLUSTERED_SITE
 *
 * Revision 1.21  2002-02-24 03:26:15+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.20  2002-02-23 23:54:20+05:30  Cprogrammer
 * increased space alloated for printing folder names
 * code to display ip address for auth, passwd change, etc
 *
 * Revision 1.19  2001-12-24 15:00:45+05:30  Cprogrammer
 * display mail stats only if maildir is present
 *
 * Revision 1.18  2001-12-24 00:57:33+05:30  Cprogrammer
 * extra spaces put in printf statements
 *
 * Revision 1.17  2001-12-22 21:03:06+05:30  Cprogrammer
 * added option to display folders and mails
 *
 * Revision 1.16  2001-12-09 02:16:23+05:30  Cprogrammer
 * display if the user is on local or remote
 *
 * Revision 1.15  2001-12-09 00:59:17+05:30  Cprogrammer
 * added display of passwd change, inactivation and activation dates
 *
 * Revision 1.14  2001-11-29 20:55:45+05:30  Cprogrammer
 * conditional compilation for distributed arch
 *
 * Revision 1.13  2001-11-29 14:35:04+05:30  Cprogrammer
 * removed vclose()
 *
 * Revision 1.12  2001-11-29 13:18:06+05:30  Cprogrammer
 * bug fix where error message was not getting displayed for non-existent local user
 *
 * Revision 1.11  2001-11-28 23:39:42+05:30  Cprogrammer
 * display information whether the user is distributed or local
 *
 * Revision 1.10  2001-11-28 23:01:28+05:30  Cprogrammer
 * code change for distributed architecture
 *
 * Revision 1.9  2001-11-24 20:37:23+05:30  Cprogrammer
 * more refined printing of inactive days
 *
 * Revision 1.8  2001-11-24 12:20:20+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.7  2001-11-24 01:44:30+05:30  Cprogrammer
 * Days inactive computation on either the add date or auth date
 *
 * Revision 1.6  2001-11-24 00:47:53+05:30  Cprogrammer
 * Corrected display of inactive days
 *
 * Revision 1.5  2001-11-23 20:55:43+05:30  Cprogrammer
 * Added addition time, display of authentication table
 *
 * Revision 1.4  2001-11-23 00:13:09+05:30  Cprogrammer
 * a null domain caused core dump. added a null value check for real_domain
 *
 * Revision 1.3  2001-11-20 10:56:15+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:24:39+05:30  Cprogrammer
 * added code to display the mailstore host
 *
 * Revision 1.1  2001-10-24 18:15:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <pwd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/socket.h>

#ifndef	lint
static char     sccsid[] = "$Id: userinfo.c,v 2.40 2016-05-17 17:09:39+05:30 mbhangui Exp $";
#endif

extern char *strptime(const char *, const char *, struct tm *);

int
vuserinfo(Email, User, Domain, DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, 
		  DisplayQuota, DisplayLastAuth, DisplayMail, DisplayFilter, DisplayAll)
	char           *Email, *User, *Domain;
	int             DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, DisplayQuota, 
	                DisplayLastAuth, DisplayMail, DisplayFilter, DisplayAll;
{
	struct passwd  *mypw;
	char           *ptr, *real_domain, *mailstore, *qmaildir, *controldir, *passwd_hash;
	FILE           *fp;
#ifdef CLUSTERED_SITE
	int             is_dist = 0;
#endif
	int             islocal = 1;
	char            maildir[MAX_BUFF], tmpbuf[MAX_BUFF];
	uid_t           uid;
	gid_t           gid;
	mdir_t          mcount;
#ifdef USE_MAILDIRQUOTA
	mdir_t          cur_size;
#endif
#ifdef ENABLE_AUTH_LOGGING
	char            timeBuf[MAX_BUFF];
	mdir_t          delivery_count, delivery_size;
	char            ipaddr[7][18];
	time_t          add_time, auth_time, pwdchg_time, inact_time, act_time, pop3_time, imap_time, delivery_time;
	struct tm       tm;
#endif

	real_domain = (char *) 0;
	if ((Domain && *Domain) && !(real_domain = vget_real_domain(Domain)))
	{
		getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
		if (*controldir == '/')
			snprintf(tmpbuf, MAX_BUFF, "%s/rcpthosts", controldir);
		else {
			getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
			snprintf(tmpbuf, MAX_BUFF, "%s/%s/rcpthosts", qmaildir, controldir);
		}
		if (!(fp = fopen(tmpbuf, "r")))
		{
			fprintf(stderr, "%s: No such domain\n", Domain);
			return (1);
		}
		for(;;)
		{
			if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			{
				fclose(fp);
				fprintf(stderr, "%s: No such domain\n", Domain);
				return (1);
			}
			if ((ptr = strchr(tmpbuf, '\n')) || (ptr = strchr(tmpbuf, '#')))
				*ptr = 0;
			for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			if (!strncmp(tmpbuf, Domain, MAX_BUFF))
			{
				real_domain = Domain;
				break;
			}
		}
		fclose(fp);
	}
#ifdef CLUSTERED_SITE
	if ((is_dist = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n", Domain);
		return (1);
	}
	if (real_domain && *real_domain)
	{
		if (is_dist == 1)
		{
			snprintf(tmpbuf, MAX_BUFF, "%s@%s", User, real_domain);
			if ((mailstore = findhost(tmpbuf, 1)) != (char *) 0)
			{
				if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
					*ptr = 0;
				for(;*mailstore && *mailstore != ':';mailstore++);
				mailstore++;
			} else
			{
				if (userNotFound)
					fprintf(stderr, "%s@%s: No such user\n", User, Domain);
				else
					fprintf(stderr, "Internal System Error\n");
				return(1);
			}
		} else
		{
			if (vauth_open((char *) 0))
			{
				fprintf(stderr, "vauth_open failed\n");
				return(1);
			}
			mailstore = MdaServer(mysql_host, real_domain);
			if (!mailstore && (!strncmp(mysql_host, "localhost", 10) || !strncmp(mysql_host, "127.0.0.1", 10)))
			{
				if ((ptr = get_local_ip(PF_INET))) {
					if (!(mailstore = MdaServer(ptr, real_domain)))
						mailstore = ptr;
				}
			}
			if (!mailstore)
				mailstore = "unknown";
		}
	} else
		mailstore = "localhost";
	islocal = islocalif(mailstore);
#else
	mailstore = "localhost";
#endif
	if (!(mypw = vauth_getpw(User, real_domain)))
	{
		if (!real_domain && !isvirtualdomain(User) && vget_assign(User, tmpbuf, MAX_BUFF, &uid, &gid))
		{
			if (DisplayName || DisplayAll)
				printf("name          : %s@localhost\n", User);
			if (DisplayUid || DisplayAll)
				printf("uid           : %d\n", (int) uid);
			if (DisplayGid || DisplayAll)
				printf("gid           : %d\n", (int) gid);
			if (DisplayDir || DisplayAll)
				printf("dir           : %s%s\n", tmpbuf,
					access(tmpbuf, F_OK) && errno == ENOENT ? (islocal ? " (missing)" : " (remote)") : "");
			return(0);
		} else
		if (valiasinfo(User, real_domain))
			return (0);
		if (Domain && *Domain)
			fprintf(stderr, "%s@%s: No such user\n", User, Domain);
		else
			fprintf(stderr, "%s: No such user\n", User);
		return (1);
	}
	if (mypw->pw_passwd[0] == '$' && mypw->pw_passwd[2] == '$')
	{
		switch(mypw->pw_passwd[1])
		{
			case '1':
				passwd_hash = "MD5";
				break;
			case '5':
				passwd_hash = "SHA256";
				break;
			case '6':
				passwd_hash = "SHA512";
				break;
			default:
				passwd_hash = "$?$";
				break;
		}
	} else
		passwd_hash = "DES";
	if (DisplayName || DisplayAll)
		printf("name          : %s@%s\n", mypw->pw_name, (Domain && *Domain) ? real_domain : "localhost");
	if (DisplayPasswd || DisplayAll)
		printf("passwd        : %s (%s)\n", mypw->pw_passwd, passwd_hash);
	if (DisplayUid || DisplayAll)
		printf("uid           : %d\n", (int) mypw->pw_uid);
	if (DisplayGid || DisplayAll)
	{
		printf("gid           : %d\n", (int) mypw->pw_gid);
		if (mypw->pw_gid == 0)
			printf("                -all services available\n");
		if (mypw->pw_gid & NO_PASSWD_CHNG)
			printf("                -password can not be changed by user\n");
		if (mypw->pw_gid & NO_POP)
			printf("                -pop access closed\n");
		if (mypw->pw_gid & NO_WEBMAIL)
			printf("                -webmail access closed\n");
		if (mypw->pw_gid & NO_IMAP)
			printf("                -imap access closed\n");
		if (mypw->pw_gid & NO_SMTP)
			printf("                -smtp access closed\n");
		if (mypw->pw_gid & BOUNCE_MAIL)
			printf("                -mail will be bounced back to sender\n");
		if (mypw->pw_gid & NO_RELAY)
			printf("                -user not allowed to relay mail\n");
		if (mypw->pw_gid & NO_DIALUP)
			printf("                -no dialup flag has been set\n");
		if (mypw->pw_gid & QA_ADMIN)
			printf("                -has qmailadmin administrator privileges\n");
		if (mypw->pw_gid & V_OVERRIDE)
			printf("                -has domain limit skip privileges\n");
		if (mypw->pw_gid & V_USER0)
			printf("                -user flag 0 is set\n");
		if (mypw->pw_gid & V_USER1)
			printf("                -user flag 1 is set\n");
		if (mypw->pw_gid & V_USER2)
			printf("                -user flag 2 is set\n");
		if (mypw->pw_gid & V_USER3)
			printf("                -user flag 3 is set\n");
	}
	if (DisplayComment || DisplayAll)
		printf("gecos         : %s\n", mypw->pw_gecos);
	if (DisplayDir || DisplayAll)
		printf("dir           : %s%s\n", mypw->pw_dir,
			access(mypw->pw_dir, F_OK) && errno == ENOENT ? (islocal ? " (missing)" : " (remote)") : "");
	if (DisplayQuota || DisplayAll)
	{
		mdir_t          dquota;
#ifdef USE_MAILDIRQUOTA	
		mdir_t          size_limit, count_limit;
#endif

		if (!strncmp(mypw->pw_shell, "NOQUOTA", 8)) {
			printf("quota         : unlimited\n");
		} else {
			dquota = parse_quota(mypw->pw_shell, 0)/1048576;
			printf("quota         : %s [%-4.2f %s]\n", mypw->pw_shell,
				dquota < 1024 ? (float) dquota : (float) (dquota/1024), dquota < 1024 ? "Mb" : "Gb");
		}
		if (islocal)
		{
			snprintf(maildir, MAX_BUFF, "%s/Maildir", mypw->pw_dir);
#ifdef USE_MAILDIRQUOTA	
			if ((size_limit = parse_quota(mypw->pw_shell, &count_limit)) == -1)
				cur_size = mcount = -1;
			else
				cur_size = recalc_quota(maildir, &mcount, size_limit, count_limit, 2);
			printf("curr quota    : %"PRIu64"S,%"PRIu64"C\n",
				cur_size == -1 ? 0 : cur_size, mcount == -1 ? 0 : mcount);
#else
			printf("curr quota    : %"PRIu64"\n", recalc_quota(maildir, 2));
#endif
		} else
			printf("curr quota    : unknown (remote)\n");
	}
	if (DisplayAll)
	{
#ifdef CLUSTERED_SITE
		printf("Mail Store IP : %s (%s - %s)\n", mailstore,
			(is_dist ? "Clustered" : "NonClustered"), 
			islocal ? "local" : "remote");
		if (is_dist)
			ptr = vauth_gethostid(mailstore);
		else
			ptr = "non-clustered domain";
		printf("Mail Store ID : %s\n", ptr ? ptr : "no id present");
#else
		printf("Mail store    : %s\n", mailstore);
#endif
#ifdef CLUSTERED_SITE
		if (!(ptr = SqlServer(mailstore, real_domain)))
		{
			if (!is_dist)
				ptr = mysql_host;
			else
				ptr = "unknown";
		}
		printf("Sql Database  : %s\n", ptr);
#else
		printf("Sql Database  : %s\n", mysql_host);
#endif
	}
#ifdef ENABLE_AUTH_LOGGING
	if (Domain && *Domain && DisplayAll)
	{
		if (is_inactive)
			printf("Table Name    : %s\n", inactive_table);
		else
			printf("Table Name    : %s\n", default_table);
	}
	if (Domain && *Domain && (DisplayLastAuth || DisplayAll))
	{
		auth_time = vget_lastauth(mypw, Domain, AUTH_TIME, ipaddr[0]);
		add_time  = vget_lastauth(mypw, Domain, CREAT_TIME, ipaddr[1]);
		pwdchg_time = vget_lastauth(mypw, Domain, PASS_TIME, ipaddr[2]);
		act_time = vget_lastauth(mypw, Domain, ACTIV_TIME, ipaddr[3]);
		inact_time = vget_lastauth(mypw, Domain, INACT_TIME, ipaddr[4]);
		pop3_time = vget_lastauth(mypw, Domain, POP3_TIME, ipaddr[5]);
		imap_time = vget_lastauth(mypw, Domain, IMAP_TIME, ipaddr[6]);
		delivery_count = 0;
		delivery_time = 0;
		delivery_size = 0;
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/deliveryCount", mypw->pw_dir);
		if ((fp = fopen(tmpbuf, "r")))
		{
			for(;;)
			{
				if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
					break;
				sscanf(tmpbuf, "%s %"SCNu64" %"SCNu64, timeBuf, &delivery_count, &delivery_size);
			}
			fclose(fp);
			strptime(timeBuf, "%d-%m-%Y:%H:%M:%S", &tm);
			delivery_time = mktime(&tm);
		}
		if (auth_time > 0 || add_time > 0)
		{
			ptr = (relay_select(Email, ipaddr[5]) || relay_select(Email, ipaddr[6]) ? "YES" : "NO");
			printf("Relay Allowed : %s\n", ptr);
			printf("Days inact    : %s\n", no_of_days((time(0) - (auth_time ? auth_time : add_time))));
		} else
		{
			printf("Relay Allowed : No\n");
			if (add_time)
				printf("Days inact    : %s\n", no_of_days((time(0) - (auth_time ? auth_time : add_time))));
			else
				printf("Days inact    : Unknown\n");
		}
		if (!add_time)
			printf("Added   On    : Unknown\n");
		else
		if (add_time > 0)
			printf("Added   On    : (%15s) %s", ipaddr[1], asctime(localtime(&add_time)));
		else
			printf("Added   On    : ????\n");
		if (!auth_time)
		{
			printf("last  auth    : Not yet logged in\n");
			printf("last  IMAP    : Not yet logged in\n");
			printf("last  POP3    : Not yet logged in\n");
		}
		else
		if (pop3_time > 0 || imap_time > 0)
		{
			printf("last  auth    : (%15s) %s", ipaddr[0], asctime(localtime(&auth_time)));
			if (pop3_time > 0)
				printf("last  POP3    : (%15s) %s", ipaddr[5], asctime(localtime(&pop3_time)));
			else
				printf("last  POP3    : Not yet logged in\n");
			if (imap_time > 0)
				printf("last  IMAP    : (%15s) %s", ipaddr[6], asctime(localtime(&imap_time)));
			else
				printf("last  IMAP    : Not yet logged in\n");
		} else
		if (auth_time < 0)
			printf("last  auth    : ????\n");
		if (!pwdchg_time)
			printf("PassChange    : Not yet Changed\n");
		else
		if (pwdchg_time > 0)
			printf("PassChange    : (%15s) %s", ipaddr[2], asctime(localtime(&pwdchg_time)));
		else
			printf("PassChange    : ????\n");
		if (!inact_time)
			printf("Inact Date    : Not yet Inactivated\n");
		else
		if (inact_time > 0)
			printf("Inact Date    : (%15s) %s", ipaddr[4], asctime(localtime(&inact_time)));
		else
			printf("Inact Date    : ????\n");
		if (inact_time > 0)
		{
			if (!act_time)
				printf("Activ Date    : Not yet Activated\n");
			else
			if (act_time > 0)
				printf("Activ Date    : (%15s) %s", ipaddr[3], asctime(localtime(&act_time)));
			else
				printf("Activ Date    : ????\n");
		} else
		{
			if (is_inactive)
				printf("Activ Date    : Not yet Activated\n");
			else
			{
				if (!add_time)
					printf("Activ Date    : Unknown\n");
				else
				if (add_time > 0)
					printf("Activ Date    : (%15s) %s", ipaddr[1], asctime(localtime(&add_time)));
				else
					printf("Activ Date    : ????\n");
			}
		}
		if (delivery_time > 0)
		{
			printf("Delivery Time :                   %s", asctime(localtime(&delivery_time)));
			printf("              : (%"PRIu64" Mails, %"PRIu64" Bytes [%"PRIu64" Kb])\n", delivery_count, delivery_size, delivery_size/1024);
		} else
			printf("Delivery Time : No Mails Delivered yet / Per Day Limit not configured\n");
	}
#endif
	if (DisplayFilter || DisplayAll)
	{
#ifdef VFILTER
		int             header_name, comparision, bounce_action, filter_no;
		char            keyword[MAX_BUFF], folder[MAX_BUFF], forward[AUTH_SIZE], filter_name[MAX_BUFF];
		char         **mailing_list;
#endif

		valiasinfo(mypw->pw_name, Domain);
#ifdef VFILTER
		snprintf(maildir, MAX_BUFF, "%s/Maildir/vfilter", mypw->pw_dir);
		if (!access(maildir, F_OK))
		{
			printf("Filters       :\n");
			vfilter_display(Email, 0, &filter_no, filter_name, &header_name, &comparision, keyword, folder, &bounce_action, forward, &mailing_list);
		}
#endif
	}
	if (DisplayMail)
	{
		mdir_t total, ttotal, tcount, unread, unseen;

		ttotal = tcount = mcount = total = unread = unseen = 0;
		snprintf(maildir, MAX_BUFF, "%s/Maildir", mypw->pw_dir);
		if (!access(maildir, F_OK))
		{
			printf("-----------------------------------------------------------------------------------\n");
			printf("Folder                          Size     Mails   TrashSize TrashMails Unread Unseen\n");
			printf("-----------------------------------------------------------------------------------\n");
			ScanDir(stdout, maildir, 1, &total, &mcount, &ttotal, &tcount, (mdir_t *) &unread,
				&unseen);
			printf("-----------------------------------------------------------------------------------\n");
			printf("Total                     %10"PRIu64"  %8"PRIu64"  %10"PRIu64"  %8"PRIu64" %6"PRIu64" %6"PRIu64"\n",
				total, mcount, ttotal, tcount, unread, unseen);
		}
	}
	return (0);
}

void
getversion_userinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
