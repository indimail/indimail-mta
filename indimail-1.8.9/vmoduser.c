/*
 * $Log: vmoduser.c,v $
 * Revision 2.27  2014-04-18 17:33:19+05:30  Cprogrammer
 * added option -D to create folder.dateformat in user's Maildir
 *
 * Revision 2.26  2013-04-29 22:52:48+05:30  Cprogrammer
 * fixed setting quota = NOQUOTA
 *
 * Revision 2.25  2011-11-09 19:46:24+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.24  2011-02-11 22:57:47+05:30  Cprogrammer
 * fix for specifying 'k', 'K', 'm', 'M' units in quota
 *
 * Revision 2.23  2009-12-02 11:05:04+05:30  Cprogrammer
 * use .domain_limits in domain directory to turn on domain limits
 *
 * Revision 2.22  2009-12-01 16:29:00+05:30  Cprogrammer
 * added checking of domain limit for user quota
 *
 * Revision 2.21  2009-10-14 20:47:54+05:30  Cprogrammer
 * check return status of parse_quota()
 * use strtoll() instead of atol()
 *
 * Revision 2.20  2009-09-30 00:23:12+05:30  Cprogrammer
 * use setuid so that quota and vacation operation succeed
 *
 * Revision 2.19  2009-01-27 18:16:28+05:30  Cprogrammer
 * added toggle operation for gid field int bits
 *
 * Revision 2.18  2008-11-06 15:06:28+05:30  Cprogrammer
 * BUG - Fix for quota setting problem when full maildirquota was specified
 *
 * Revision 2.17  2008-08-02 09:10:29+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.16  2008-06-24 22:02:19+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.15  2008-06-13 10:55:43+05:30  Cprogrammer
 * compile active->inactive toggle only if ENABLE_AUTH_LOGGING defined
 *
 * Revision 2.14  2008-05-28 17:41:35+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.13  2008-05-28 15:33:42+05:30  Cprogrammer
 * removed ldap, cdb code
 *
 * Revision 2.12  2004-07-03 23:55:40+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.11  2003-10-26 00:09:25+05:30  Cprogrammer
 * Preserve mail count limit in quota field when changing quota
 *
 * Revision 2.10  2003-10-24 22:42:19+05:30  Cprogrammer
 * added usage for setting smtp access and domain limit override access
 *
 * Revision 2.9  2003-10-24 00:45:23+05:30  Cprogrammer
 * added option to set NO_SMTP and V_OVERRIDE
 *
 * Revision 2.8  2003-05-30 00:01:21+05:30  Cprogrammer
 * bypass ldap to prevent wrong setting of pw_gid
 *
 * Revision 2.7  2003-01-12 21:33:51+05:30  Cprogrammer
 * set passwd change in lastauth when changing passwords
 *
 * Revision 2.6  2002-12-02 02:31:33+05:30  Cprogrammer
 * allow quota to be increased/decreased by prefixing quota with '+' or '-' sign
 *
 * Revision 2.5  2002-11-28 00:48:25+05:30  Cprogrammer
 * option to set passwd added
 *
 * Revision 2.4  2002-07-03 01:20:56+05:30  Cprogrammer
 * avoid overwriting static location returned by vauth_getpw()
 *
 * Revision 2.3  2002-05-12 01:23:49+05:30  Cprogrammer
 * added option to add autoresponder for a user
 *
 * Revision 2.2  2002-05-11 15:25:54+05:30  Cprogrammer
 * changed display of error messages to stderr
 *
 * Revision 2.1  2002-05-10 10:09:29+05:30  Cprogrammer
 * removed comments
 *
 * Revision 1.18  2002-04-03 01:45:26+05:30  Cprogrammer
 * use uid/gid from assign file
 *
 * Revision 1.17  2002-03-20 01:36:19+05:30  Cprogrammer
 * corrected bug with overwriting static location of pw structure returned by vauth_getpw inside vauth_setpw
 *
 * Revision 1.16  2002-02-24 22:47:02+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.15  2002-02-24 03:32:09+05:30  Cprogrammer
 * added MAILDROP Maildir quota code
 * update .current_size/maildirsize only after successful vauth_getpw()
 *
 * Revision 1.14  2001-12-27 01:30:55+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.13  2001-12-19 20:40:10+05:30  Cprogrammer
 * err message if user does not exist
 *
 * Revision 1.12  2001-12-08 17:46:05+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.11  2001-12-08 00:37:00+05:30  Cprogrammer
 * changed first argument of vauth_active to passwd structure
 *
 * Revision 1.10  2001-12-02 20:23:26+05:30  Cprogrammer
 * conditional compilation of mysql specific code
 *
 * Revision 1.9  2001-11-28 23:10:47+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.8  2001-11-24 21:23:26+05:30  Cprogrammer
 * Creation of Maildir when user is made active
 *
 * Revision 1.7  2001-11-24 12:22:07+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.6  2001-11-24 01:46:36+05:30  Cprogrammer
 * option to make user inactive or active
 *
 * Revision 1.5  2001-11-23 20:56:53+05:30  Cprogrammer
 * used real_domain instead of domain in vauth_getpw and vauth_setpw
 *
 * Revision 1.4  2001-11-23 00:15:33+05:30  Cprogrammer
 * return from code if real_domain is null
 *
 * Revision 1.3  2001-11-20 11:00:57+05:30  Cprogrammer
 * added getversion_vmoduser_c()
 *
 * Revision 1.2  2001-11-14 19:28:58+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:41+05:30  Cprogrammer
 * Initial revision
 */
#include "indimail.h"
#include <stdio.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vmoduser.c,v 2.27 2014-04-18 17:33:19+05:30 Cprogrammer Stab mbhangui $";
#endif

char            Email[MAX_BUFF];
char            User[MAX_BUFF];
char            Domain[MAX_BUFF];
char            Gecos[MAX_BUFF];
char            Passwd[MAX_BUFF];
char            DateFormat[MAX_BUFF];
char            Quota[MAX_BUFF];
char            vacation_file[MAX_BUFF];

int             GidFlag = 0;
int             QuotaFlag = 0;
int             toggle = 0;
#ifdef ENABLE_AUTH_LOGGING
int             active_inactive = 0;
#endif
int             ClearFlags;
int             set_vacation = 0;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err, fd;
	uid_t           uid;
#ifdef ENABLE_AUTH_LOGGING
	gid_t           gid;
#endif
	struct passwd   PwTmp;
	struct passwd  *pw;
	char            tmpbuf[MAX_BUFF], tmpQuota[MAX_BUFF];
	char           *real_domain, *ptr;
	mdir_t          quota = 0;
#ifdef USE_MAILDIRQUOTA
	mdir_t          size_limit, count_limit, mailcount;
#endif
#ifdef ENABLE_DOMAIN_LIMITS
	char            TheDir[MAX_BUFF];
	struct vlimits  limits;
	int             domain_limits;
#endif

	if (get_options(argc, argv))
		return(1);
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	if (uid != 0 && uid != indimailuid)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if (QuotaFlag == 1 || set_vacation)
	{
		if (uid && setuid(0))
		{
			error_stack(stderr, "setuid-root: %s\n", strerror(errno));
			return(1);
		}
	} else
	if (setuid(uid))
	{
		error_stack(stderr, "setuid-%d: %s\n", uid, strerror(errno));
		return(1);
	}
	if (set_vacation)
		return(add_vacation(Email, vacation_file));
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", Email);
		return(1);
	}
	real_domain = (char *) 0;
	if (!(real_domain = vget_real_domain(Domain)))
	{
		error_stack(stderr, "%s: No such domain\n", Domain);
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!vget_assign(real_domain, TheDir, MAX_BUFF, 0, 0))
	{
		error_stack(stderr, "%s: domain does not exist\n", real_domain);
		return (1);
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.domain_limits", TheDir);
	domain_limits = ((access(tmpbuf, F_OK) && !getenv("DOMAIN_LIMITS")) ? 0 : 1);
#endif
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
	{
		error_stack(stderr, "Unable to verify %s as a distributed domain\n", real_domain);
		return(1);
	} else
	if (err == 1)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s@%s", User, real_domain);
		if (!findhost(tmpbuf, 1))
		{
			error_stack(stderr, "no such user %s@%s\n", User, real_domain);
			return(1);
		}
	}
#endif
	if (!(pw = vauth_getpw(User, real_domain)))
	{
		error_stack(stderr, "no such user %s@%s\n", User, real_domain);
		return(1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!(pw->pw_gid & V_OVERRIDE) && domain_limits)
	{
		if (vget_limits(real_domain, &limits))
		{
			error_stack(stderr, "Unable to get domain limits for %s\n", real_domain);
			return(1);
		}
		if (QuotaFlag && (limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY))
		{
			error_stack(stderr, "quota modification not allowed for %s\n", Email);
			return(1);
		}
	}
#endif
	PwTmp = *pw; /*- structure copy */
	pw = &PwTmp;
	if (*Gecos)
		pw->pw_gecos = Gecos;
	if (*Passwd)
		pw->pw_passwd = Passwd;
	if (ClearFlags == 1)
		pw->pw_gid = 0;
	else
	if (GidFlag != 0)
	{
		if (toggle)
			pw->pw_gid ^= GidFlag;
		else
			pw->pw_gid |= GidFlag;
	}
	if (QuotaFlag == 1)
	{
		if (!strncmp(Quota, "NOQUOTA", 8))
			pw->pw_shell = "NOQUOTA";
		else {
			if ((*Quota == '+') || (*Quota == '-'))
				snprintf(tmpQuota, sizeof(tmpQuota), "%"PRId64"", parse_quota(pw->pw_shell, 0) + parse_quota(Quota, 0));
			else
				snprintf(tmpQuota, sizeof(tmpQuota), "%"PRId64"", parse_quota(Quota, 0));
			if (!(ptr = strchr(tmpQuota, ',')))
			{
				if ((ptr = strchr(pw->pw_shell, ',')))
					scat(tmpQuota, ptr, sizeof(tmpQuota));
			}
			pw->pw_shell = tmpQuota;
		}
	}
	err = 0;
#ifdef ENABLE_AUTH_LOGGING
	if (active_inactive == 1)
	{
		if (is_inactive)
		{
			err = vauth_active(pw, real_domain, FROM_INACTIVE_TO_ACTIVE);
			if (!vget_assign(real_domain, 0, 0, &uid, &gid))
			{
				if (indimailuid == -1 || indimailgid == -1)
					GetIndiId(&indimailuid, &indimailgid);
				uid = indimailuid;
				gid = indimailgid;
			}
			vmake_maildir(pw->pw_dir, uid, gid, real_domain);
		} else
			err = vdeluser(User, real_domain, 2);
	}
#endif
	if ((*Gecos || *Passwd || ClearFlags || GidFlag || QuotaFlag) && (err = vauth_setpw(pw, real_domain)))
		error_stack(stderr, "vauth_setpw failed\n");
	if (!err && QuotaFlag == 1)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir", pw->pw_dir);
#ifdef USE_MAILDIRQUOTA
		if ((size_limit = parse_quota(pw->pw_shell, &count_limit)) == -1)
		{
			error_stack(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
			return (1);
		}
		quota = recalc_quota(tmpbuf, &mailcount, size_limit, count_limit, 2);
#else
		quota = recalc_quota(tmpbuf, 2);
#endif
	}
	if (*Passwd)
	{
		if (!quota)
		{
			snprintf(tmpbuf, MAX_BUFF, "%s/Maildir", pw->pw_dir);
#ifdef USE_MAILDIRQUOTA
			quota = check_quota(tmpbuf, 0);
#else
			quota = check_quota(tmpbuf);
#endif
		}
#ifdef ENABLE_AUTH_LOGGING
		vset_lastauth(pw->pw_name, real_domain, "pass", GetIpaddr(), pw->pw_gecos, quota);
#endif
	}
	if (*DateFormat) {
		snprintf(tmpbuf, MAX_BUFF, "%s/Maildir/folder.dateformat", pw->pw_dir);
		if ((fd = open(tmpbuf, O_CREAT|O_TRUNC|O_WRONLY, INDIMAIL_QMAIL_MODE)) == -1) {
			error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
			return (1);
		}
		if (!vget_assign(real_domain, 0, 0, &uid, &gid)) {
			if (indimailuid == -1 || indimailgid == -1)
				GetIndiId(&indimailuid, &indimailgid);
			uid = indimailuid;
			gid = indimailgid;
		}
		if (fchown(fd, uid, gid))
		{
			error_stack(stderr, "fchown: %s: (uid %d: gid %d): %s\n", tmpbuf, uid, gid, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (filewrt(fd, "%s\n", DateFormat) == -1) {
			error_stack(stderr, "write: %s: %s\n", tmpbuf, strerror(errno));
			return (1);
		}
		close(fd);
	}
	vclose();
	return(err);
}

void
usage()
{
	fprintf(stderr, "usage: vmoduser [options] email_addr\n");
	fprintf(stderr, "options: -V                       (print version number)\n");
	fprintf(stderr, "         -v                       (verbose)\n");
#ifdef ENABLE_AUTH_LOGGING
	fprintf(stderr, "         -n                       (Inactive<->Active Toggle)\n");
#endif
	fprintf(stderr, "         -q quota                 (set quota to quota bytes, +/- to increase/decrease curr value)\n");
	fprintf(stderr, "         -c comment               (set the comment/gecos field)\n");
	fprintf(stderr, "         -P passwd                (set the password field)\n");
	fprintf(stderr, "         -e encrypted_passwd      (set the encrypted password field)\n");
	fprintf(stderr, "         -D date format           (Delivery to a Date Folder)\n");
	fprintf(stderr, "         -l vacation_message_file (sets up Auto Responder)\n");
	fprintf(stderr, "                                  (some special values for vacation_message_file)\n");
	fprintf(stderr, "                                  ('-' to remove vacation, '+' to take mesage from stdin)\n");
	fprintf(stderr, "the following options are bit flags in the gid int field\n");
	fprintf(stderr, "         -t ( toggle bit flags in the gid int field for below operations )\n");
	fprintf(stderr, "         -u ( set no dialup flag )\n");
	fprintf(stderr, "         -d ( set no password changing flag )\n");
	fprintf(stderr, "         -p ( set no pop access flag )\n");
	fprintf(stderr, "         -w ( set no web mail access flag )\n");
	fprintf(stderr, "         -s ( set no smtp access flag )\n");
	fprintf(stderr, "         -i ( set no imap access flag )\n");
	fprintf(stderr, "         -b ( set bounce mail flag )\n");
	fprintf(stderr, "         -r ( set no external relay flag )\n");
	fprintf(stderr, "         -o ( set domain limit override privileges)\n");
	fprintf(stderr, "         -a ( grant administrator privileges)\n");
	fprintf(stderr, "         -0 ( set V_USER0 flag )\n");
	fprintf(stderr, "         -1 ( set V_USER1 flag )\n");
	fprintf(stderr, "         -2 ( set V_USER2 flag )\n");
	fprintf(stderr, "         -3 ( set V_USER3 flag )\n");
	fprintf(stderr, "         -x ( clear all flags )\n");
}

int
get_options(int argc, char **argv)
{
	int             c, errflag;

	memset(User, 0, MAX_BUFF);
	memset(Email, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(Gecos, 0, MAX_BUFF);
	memset(Passwd, 0, MAX_BUFF);
	memset(DateFormat, 0, MAX_BUFF);
	memset(Quota, 0, MAX_BUFF);
	toggle = ClearFlags = 0;
	QuotaFlag = 0;
	errflag = 0;
#ifdef ENABLE_AUTH_LOGGING
	while ((c = getopt(argc, argv, "avutnxD:c:q:dpwisobr0123he:l:P:")) != -1) 
#else
	while ((c = getopt(argc, argv, "avuxD:c:q:dpwisobr0123he:l:P:")) != -1) 
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 't':
			toggle = 1;
			break;
#ifdef ENABLE_AUTH_LOGGING
		case 'n':
			active_inactive = 1;
#endif
			break;
		case 'x':
			ClearFlags = 1;
			break;
		case 'e':
			encrypt_flag = 1;
			/*- flow through */
		case 'P':
			mkpasswd3(optarg, Passwd, MAX_BUFF);
			break;
		case 'D':
			scopy(DateFormat, optarg, MAX_BUFF);
			break;
		case 'l':
			scopy(vacation_file, optarg, MAX_BUFF);
			set_vacation = 1;
			break;
		case 'c':
			scopy(Gecos, optarg, MAX_BUFF);
			break;
		case 'q':
			QuotaFlag = 1;
			scopy(Quota, optarg, MAX_BUFF);
			break;
		case 'd':
			GidFlag |= NO_PASSWD_CHNG;
			break;
		case 'p':
			GidFlag |= NO_POP;
			break;
		case 's':
			GidFlag |= NO_SMTP;
			break;
		case 'o':
			GidFlag |= V_OVERRIDE;
			break;
		case 'w':
			GidFlag |= NO_WEBMAIL;
			break;
		case 'i':
			GidFlag |= NO_IMAP;
			break;
		case 'b':
			GidFlag |= BOUNCE_MAIL;
			break;
		case 'r':
			GidFlag |= NO_RELAY;
			break;
		case 'u':
			GidFlag |= NO_DIALUP;
			break;
		case '0':
			GidFlag |= V_USER0;
			break;
		case '1':
			GidFlag |= V_USER1;
			break;
		case '2':
			GidFlag |= V_USER2;
			break;
		case '3':
			GidFlag |= V_USER3;
			break;
		case 'a':
			GidFlag |= QA_ADMIN;
			break;
		case 'h':
			usage();
			return(1);
		default:
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
		scopy(Email, argv[optind++], MAX_BUFF);
	if (errflag || !*Email)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vmoduser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
