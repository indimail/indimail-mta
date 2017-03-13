/*
 * $Log: vdelivermail.c,v $
 * Revision 2.64  2017-03-13 14:12:43+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.63  2017-01-09 19:38:54+05:30  Cprogrammer
 * initialize user, domain, bounce, userext variables
 *
 * Revision 2.62  2016-05-25 09:08:03+05:30  Cprogrammer
 * use LIBEXECDIR for overquota.sh
 *
 * Revision 2.61  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.60  2015-12-17 17:14:32+05:30  Cprogrammer
 * mimic behaviour of dot-qmail. skip valias lines if program exits 99
 *
 * Revision 2.59  2014-01-13 08:04:58+05:30  Cprogrammer
 * discard bounces if DISCARD_BOUNCE env variable is set
 *
 * Revision 2.58  2011-06-23 20:01:57+05:30  Cprogrammer
 * fixed setting of NOALIAS
 *
 * Revision 2.57  2011-06-22 22:41:10+05:30  Cprogrammer
 * skip alias if vdelivermail is recursively called
 *
 * Revision 2.56  2011-06-02 20:25:41+05:30  Cprogrammer
 * defer overquota mails when deliver_mail() returns -5
 *
 * Revision 2.55  2010-07-14 22:16:21+05:30  Cprogrammer
 * display only MAILCOUNT_LIMIT, MAILSIZE_LIMIT quota message when recordMailcount() returns
 * overquota
 *
 * Revision 2.54  2010-05-28 14:11:24+05:30  Cprogrammer
 * use QMTP as default
 *
 * Revision 2.53  2010-04-24 15:21:45+05:30  Cprogrammer
 * set env variable SMTPROUTE or QMTPROUTE depending on value of ROUTES env variable
 *
 * Revision 2.52  2010-03-24 10:15:18+05:30  Cprogrammer
 * moved overquota.sh to libexec
 *
 * Revision 2.51  2009-10-14 20:46:47+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.50  2009-09-23 15:00:29+05:30  Cprogrammer
 * change for new runcmmd
 *
 * Revision 2.49  2009-02-06 11:41:07+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.48  2008-07-17 22:57:06+05:30  Cprogrammer
 * port for Darwin
 *
 * Revision 2.47  2008-06-24 22:02:07+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.46  2008-06-13 10:42:43+05:30  Cprogrammer
 * fixed compilation error if ENABLE_AUTH_LOGGING not defined
 *
 * Revision 2.45  2008-05-28 15:31:50+05:30  Cprogrammer
 * mysql module default
 *
 * Revision 2.44  2006-08-29 17:44:46+05:30  Cprogrammer
 * Use absolute path for filename referenced by HOLDOVERQUOTA if it contains '/'
 *
 * Revision 2.43  2006-08-02 14:54:26+05:30  Cprogrammer
 * BUG - Missed process_dir in doAlias()
 *
 * Revision 2.42  2006-08-01 14:27:21+05:30  Cprogrammer
 * added common function process_dir() to create directories for .qmail, valias and mail delivery
 *
 * Revision 2.41  2006-07-27 14:37:56+05:30  Cprogrammer
 * create directory specified by MAILDIRFOLDER when called from .qmail
 *
 * Revision 2.40  2006-03-02 20:41:57+05:30  Cprogrammer
 * corrected order of MailQuotaSize, MailQuotaCount
 *
 * Revision 2.39  2005-12-29 18:51:18+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.38  2005-12-21 09:51:28+05:30  Cprogrammer
 * prevent mail delivery to group IDs
 * make gcc 4 happy
 *
 * Revision 2.37  2005-07-04 21:10:05+05:30  Cprogrammer
 * turn on extension address only if QMAIL_EXT is defined
 *
 * Revision 2.36  2005-06-03 09:04:54+05:30  Cprogrammer
 * continue with maildir delivery after valias/.qmail processing
 *
 * Revision 2.35  2005-04-02 20:16:04+05:30  Cprogrammer
 * hold overquota messages if BOUNCE_MAIL is set
 *
 * Revision 2.34  2005-04-02 18:09:19+05:30  Cprogrammer
 * display error message for looping mails
 * for multiple alias deliver, and continue instead of exiting on first error
 *
 * Revision 2.33  2005-02-09 22:57:23+05:30  Cprogrammer
 * combine case -1, -4 for return values of deliver_mail()
 *
 * Revision 2.32  2005-02-09 22:40:18+05:30  Cprogrammer
 * added missing cases
 *
 * Revision 2.31  2005-01-05 22:45:12+05:30  Cprogrammer
 * defer mail for insufficient quota
 *
 * Revision 2.30  2004-09-20 19:55:27+05:30  Cprogrammer
 * add entry in lastauth for mails delivered to a group id
 *
 * Revision 2.29  2004-07-17 14:35:35+05:30  Cprogrammer
 * qqeh argument to qmail-local
 *
 * Revision 2.28  2004-07-05 19:22:35+05:30  Cprogrammer
 * fixed bug where blank lines in blackholedsender(s) caused match for all senders
 *
 * Revision 2.27  2004-06-20 01:05:33+05:30  Cprogrammer
 * added case for MAILCOUNT_LIMIT exceeded
 *
 * Revision 2.26  2004-06-19 23:39:35+05:30  Cprogrammer
 * new quota definition - AUTO for automatically figuring out quota while delivering
 *
 * Revision 2.25  2004-06-09 17:09:11+05:30  Cprogrammer
 * fixed problem with valias requiring entry in indimail table
 *
 * Revision 2.24  2004-05-17 14:02:25+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.23  2004-05-03 22:06:15+05:30  Cprogrammer
 * unset SPAMFILTER to avoid using notification queue
 *
 * Revision 2.22  2004-01-10 09:47:19+05:30  Cprogrammer
 * check return status of deliver_mail
 *
 * Revision 2.21  2003-10-01 02:10:15+05:30  Cprogrammer
 * exit with temporary error if syntax of IP in .qmail-default is not in
 * SMTPROUTES format
 *
 * Revision 2.20  2003-09-04 19:51:44+05:30  Cprogrammer
 * corrected logic for determining address is a maildir or ip address
 *
 * Revision 2.19  2003-07-06 14:17:25+05:30  Cprogrammer
 * option in .qmail-default to forward mail to different host
 *
 * Revision 2.18  2003-06-08 19:25:10+05:30  Cprogrammer
 * added blackhole function using control files blackholedsender and blackholedpatterns
 *
 * Revision 2.17  2003-01-22 16:03:35+05:30  Cprogrammer
 * create maildirfolder for folders automatically created
 *
 * Revision 2.16  2002-12-13 19:10:37+05:30  Cprogrammer
 * added environment variable MAKE_SEEKABLE to turn on/off seekable stdin
 *
 * Revision 2.15  2002-11-30 09:38:19+05:30  Cprogrammer
 * bounce only if userNotFound is set
 *
 * Revision 2.14  2002-11-28 00:47:44+05:30  Cprogrammer
 * compilation correction for non-clustered domains
 *
 * Revision 2.13  2002-11-24 15:50:38+05:30  Cprogrammer
 * use MAILDIRFOLDER if defined for delivering mail
 *
 * Revision 2.12  2002-11-21 14:36:09+05:30  Cprogrammer
 * corrected bug  - only 4 bytes was being copied to Bounce variable
 *
 * Revision 2.11  2002-11-21 00:58:57+05:30  Cprogrammer
 * added code to deliver mail for local users
 *
 * Revision 2.10  2002-10-12 23:07:14+05:30  Cprogrammer
 * corrected compilation problems in non-clustered environment
 *
 * Revision 2.9  2002-10-12 21:15:10+05:30  Cprogrammer
 * moved deliver_mail to libindimail
 * restructured code
 *
 * Revision 2.8  2002-10-12 02:38:52+05:30  Cprogrammer
 * added X-Filter header
 * added environment variable MAILDIRFOLDER to deliver to a user specified folder
 *
 * Revision 2.7  2002-09-04 12:57:09+05:30  Cprogrammer
 * moved maildir_to_email() to a different source file
 *
 * Revision 2.6  2002-08-25 22:35:55+05:30  Cprogrammer
 * made control dir configurable.
 *
 * Revision 2.5  2002-08-01 11:31:03+05:30  Cprogrammer
 * added code for mail alert
 *
 * Revision 2.4  2002-07-12 01:23:50+05:30  Cprogrammer
 * added code to make stdin seekable.
 * added error code if lseek fails
 *
 * Revision 2.3  2002-05-21 09:43:22+05:30  Cprogrammer
 * added overquota message generation to the user
 *
 * Revision 2.2  2002-05-09 00:37:42+05:30  Cprogrammer
 * same logic for user_not_found applied for inactive users
 *
 * Revision 2.1  2002-04-17 13:40:14+05:30  Cprogrammer
 * changed get_message_size() to use fstat() instead of read() to calculate size
 *
 * Revision 1.20  2002-04-10 10:30:03+05:30  Cprogrammer
 * unbuffer stdout
 *
 * Revision 1.19  2002-04-10 01:16:25+05:30  Cprogrammer
 * flush stdout on exit and don't flush other open file descriptors on exit
 *
 * Revision 1.18  2002-04-09 14:35:27+05:30  Cprogrammer
 * prevent looping of mails by doing hostid check after findhost()
 *
 * Revision 1.17  2002-04-06 23:30:16+05:30  Cprogrammer
 * set SMTPROUTE for qmail-remote
 *
 * Revision 1.16  2002-03-24 19:15:24+05:30  Cprogrammer
 * changed QMAIL_INACTIVE to ALLOW_INACTIVE
 * absense of a directory not to be treated as inactive
 *
 * Revision 1.15  2002-03-18 19:05:31+05:30  Cprogrammer
 * exit(111) if we can't figure out if domain is distributed
 *
 * Revision 1.14  2002-03-03 17:39:14+05:30  Cprogrammer
 * changed error messages
 *
 * Revision 1.13  2002-03-03 17:14:20+05:30  Cprogrammer
 * replaced strcat with scat
 *
 * Revision 1.12  2002-02-25 13:55:34+05:30  Cprogrammer
 * added Received header for vdelivermail
 *
 * Revision 1.11  2002-02-24 22:08:15+05:30  Cprogrammer
 * remove quotes '"' from username to prevent mysql syntax error
 *
 * Revision 1.10  2002-02-24 03:27:29+05:30  Cprogrammer
 * Change for incorporating MAILDROP Maildir Quota
 *
 * Revision 1.9  2002-02-23 20:32:36+05:30  Cprogrammer
 * added code to provide qmail_local functionality
 * change in logic to bounce mails for inactive users
 *
 * Revision 1.8  2001-12-11 11:34:45+05:30  Cprogrammer
 * removed TMDA code
 *
 * Revision 1.7  2001-12-01 23:14:55+05:30  Cprogrammer
 * added code for TMDA
 *
 * Revision 1.6  2001-12-01 11:16:07+05:30  Cprogrammer
 * ignore SIGPIPE
 *
 * Revision 1.5  2001-12-01 02:16:18+05:30  Cprogrammer
 * added better wait handling
 *
 * Revision 1.4  2001-11-24 22:32:59+05:30  Cprogrammer
 * replaced strcmp with strncmp
 *
 * Revision 1.3  2001-11-24 12:21:47+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:16+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:34+05:30  Cprogrammer
 * Initial revision
 *
 */

/*- include files */
#include "indimail.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef	lint
static char     sccsid[] = "$Id: vdelivermail.c,v 2.64 2017-03-13 14:12:43+05:30 Cprogrammer Exp mbhangui $";
#endif

/*- Globals */
char            ReturnPathEnv[AUTH_SIZE];

/*- Function Prototypes */
static void     get_arguments(int, char **, char *, char *, char *, char *);
static int      processMail(struct passwd *, char *, char *, mdir_t);
static int      doAlias(char *, char *, char *, mdir_t);
static int      reject_mail(char *, char *, int, mdir_t, char *);
#ifdef VALIAS
static int      process_valias(char *, char *, mdir_t);
#endif
static char    *process_dir(char *, uid_t, gid_t);
static int      bhfcheck(char *);
static void     vdl_exit(int);

void
vdl_exit(int err)
{
	vclose();
	fflush(stdout);
	if (getenv("DISCARD_BOUNCE") && err == 100)
		_exit (0);
	_exit(err);
}

/* 
 * The email message comes in on file descriptor 0 - stanard in
 * The user to deliver the email to is in the EXT environment variable
 * The domain to deliver the email to is in the HOST environment variable
 *
 * Deliver Exit Codes
 *	exit(0)   - exit successfully and have qmail delete the email 
 *	exit(100) - Bounce Back the email
 *	exit(111) - email remains in the queue.
 */

int
main(int argc, char **argv)
{
	char            TheUser[AUTH_SIZE], TheDomain[AUTH_SIZE], Bounce[MAX_BUFF];
	int             MsgSize;
	struct passwd  *pw;
	char           *ptr, *addr;
#ifdef QMAIL_EXT
	char            TheUserExt[AUTH_SIZE];
#endif
#ifdef CLUSTERED_SITE
	int             ret;
	char           *ip, remoteip[MAX_BUFF], *local_hostid, *remote_hostid;
	char            Email[MAX_BUFF], route[256];
#endif
#ifdef MAKE_SEEKABLE
	char           *str;
#endif

	setbuf(stdout, 0);
	if (GetIndiId(&indimailuid, &indimailgid))
		_exit(111);
	/*
	 * get the arguments to the program and setup things 
	 */
	*TheUser = *TheDomain = *Bounce = 0;
#ifdef QMAIL_EXT
	*TheUserExt = 0;
	get_arguments(argc, argv, TheUser, TheDomain, TheUserExt, Bounce);
#else
	get_arguments(argc, argv, TheUser, TheDomain, 0, Bounce);
#endif
	signal(SIGPIPE, SIG_IGN);
#ifdef MAKE_SEEKABLE
	if ((str = getenv("MAKE_SEEKABLE")) && *str != '0' && makeseekable(stdin))
	{
		fprintf(stderr, "makeseekable: system error: %s\n", strerror(errno));
		_exit(111);
	}
#endif
	unsetenv("SPAMFILTER");
	/*- if we don't know the message size then read it */
	if (!(MsgSize = get_message_size()))
	{
		fprintf(stderr, "Discarding 0 size message\n");
		_exit(0);
	}
	if (!(addr = (char *) getenv("SENDER")))
	{
		fprintf(stderr, "No SENDER environment variable\n");
		_exit(100);
	}
	if (bhfcheck(addr))
	{
		fprintf(stderr, "Discarding BlackHoled Address %s\n", addr);
		_exit(0);
	}
	/*
	 * get the user from indimail database 
	 */
	if (!(pw = ((ptr = (char *) getenv("PWSTRUCT"))) ?  strToPw(ptr, slen(ptr) + 1) : vauth_getpw(TheUser, TheDomain)))
	{
		if (!userNotFound)
		{
			fprintf(stderr, "Temporary Authentication error\n");
			vdl_exit(111);
		} 
#ifdef CLUSTERED_SITE
		/*- Set SMTPROUTE for qmail-remote */
		if ((ret = is_distributed_domain(TheDomain)) == -1)
		{
			fprintf(stderr, "is_distributed_domain: Error\n");
			vdl_exit(111);
		} else
		if (ret == 1)
		{
			snprintf(Email, sizeof(Email), "%s@%s", TheUser, TheDomain);
			if ((ip = findhost(Email, 0)))
			{
				for (ptr = ip;*ptr && *ptr != ':';ptr++);
				scopy(remoteip, ptr + 1, MAX_BUFF);
				if ((ptr = strchr(remoteip, ':')))
					*ptr = 0;
				else
				{
					fprintf(stderr, "findhost: Invalid route %s\n", ip);
					vdl_exit(111);
				}
				if (!(local_hostid = get_local_hostid()))
				{
					fprintf(stderr, "Unable to get hostid\n");
					vdl_exit(111);
				}
				if (!(remote_hostid = vauth_gethostid(remoteip)))
				{
					fprintf(stderr, "unable to get hostid for %s\n", remoteip);
					vdl_exit(111);
				} else /* avoid looping of mails */
				if (strncmp(remote_hostid, local_hostid, MAX_BUFF))
				{
					if ((ptr = getenv("ROUTES")) && (*ptr && !memcmp(ptr, "smtp", 4)))
						ptr = "SMTP";
					else
						ptr = "QMTP";
					snprintf(route, sizeof(route), "%cMTPROUTE=%s", *ptr, ip);
					putenv(route);
					switch (qmail_remote(TheUser, TheDomain))
					{
						case -1:
							fprintf(stderr, "%s@%s has insufficient quota. indimail (#5.1.4)", TheUser, TheDomain);
							vdl_exit(100);
							break;
						case -2:
							fprintf(stderr, "system error: %s\n", strerror(errno));
							vdl_exit(111);
							break;
						case 0:
							vdl_exit(0);
							break;
						case 100:
							vdl_exit(100);
							break;
						case 111:
							vdl_exit(111);
							break;
					} /*- switch() */
				}
			} else
			if (!userNotFound)
			{
				fprintf(stderr, "Temporary Authentication error\n");
				vdl_exit(111);
			}
		} /*- if (is_distributed()) */
#endif
#ifdef QMAIL_EXT
		/*
		 * try and find user that matches the QmailEXT address if no user found, 
		 * and the QmailEXT address is different, meaning there was an extension 
		 */
		if (getenv("QMAIL_EXT") && strncmp(TheUser, TheUserExt, AUTH_SIZE))
			pw = vauth_getpw(TheUserExt, TheDomain);
#endif
	} /*- if (!(pw = vauth_getpw(TheUser, TheDomain))) */
	if (pw)
	{
		/*-
		 * Mail delivery has happened successfully
		 * Do not check the return status of 
		 * vset_lastauth * as that will result
		 * in duplicate mails in case
		 * of vset_lastauth() error
		 */
#ifdef ENABLE_AUTH_LOGGING
		if (getenv("ALLOW_INACTIVE"))
		{
			processMail(pw, pw->pw_name, TheDomain, MsgSize);
			if (!strncmp (pw->pw_gecos, "MailGroup ", 10)) /*- Prevent groups getting inactive */
				vset_lastauth(TheUser, TheDomain, "deli", "", pw->pw_gecos + 10, 0);
		} else
		{
			if (is_inactive)
				reject_mail(pw->pw_name, TheDomain, 0, MsgSize, Bounce);
			else
			{
				processMail(pw, pw->pw_name, TheDomain, MsgSize);
				if (!strncmp (pw->pw_gecos, "MailGroup ", 10)) /*- Prevent groups getting inactive */
					vset_lastauth(TheUser, TheDomain, "deli", "", pw->pw_gecos + 10, 0);
			}
		}
#else
		if (is_inactive)
			reject_mail(pw->pw_name, TheDomain, 0, MsgSize, Bounce);
		else
			processMail(pw, pw->pw_name, TheDomain, MsgSize);
#endif
	} else
	{
#ifdef VALIAS
		/*- process valiases if configured */
		if (!process_valias(TheUser, TheDomain, MsgSize))
			reject_mail(TheUser, TheDomain, 1, MsgSize, Bounce);
#else
		reject_mail(TheUser, TheDomain, 1, MsgSize, Bounce);
#endif
	}
	vclose();
	exit(0);
}

/* 
 * Get the command line arguments and the environment variables.
 * Force addresses to be lower case and set the default domain
 */
static void
get_arguments(int argc, char **argv, char *user, char *domain, char *user_ext, char *bounce)
{
	char           *tmpstr;
	int             local;
#ifdef QMAIL_EXT
	int             i;
#endif

	local = 0;
	if (argc == 3)
	{
		scopy(bounce, argv[2], MAX_BUFF);
		/*- get the last parameter in the .qmail-default file */
		if (!(tmpstr = getenv("EXT")))
		{
			fprintf(stderr, "No EXT environment variable\n");
			vdl_exit(100);
		} else
		{
			if (*tmpstr)
				scopy(user, tmpstr, AUTH_SIZE);
			else
			if (!(tmpstr = getenv("LOCAL")))
			{
				fprintf(stderr, "No LOCAL environment variable\n");
				vdl_exit(100);
			} else
			{
				scopy(user, tmpstr, AUTH_SIZE);
				local = 1;
			}
		}
		if (local)
			scopy(domain, ((tmpstr = (char *) getenv("DEFAULT_DOMAIN")) ? tmpstr : DEFAULT_DOMAIN), AUTH_SIZE);
		else
		if (!(tmpstr = getenv("HOST")))
		{
			fprintf(stderr, "No HOST environment variable\n");
			vdl_exit(100);
		} else
			scopy(domain, tmpstr, AUTH_SIZE);
		if (remove_quotes(user))
		{
			fprintf(stderr, "Invalid user %s\n", user);
			vdl_exit(100);
		}
	} else
	if (argc == 11) /*- qmail-local */
	{
		scopy(user, argv[6], AUTH_SIZE);
		scopy(domain, argv[7], AUTH_SIZE);
		snprintf(ReturnPathEnv, AUTH_SIZE, "RPLINE=Return-Path: <%s>\n", argv[8]);
		if (putenv(ReturnPathEnv) == -1)
		{
			fprintf(stderr, "vdelivermail: putenv: %s\n", strerror(ENOMEM));
			vdl_exit(111);
		}
	} else
	{
		fprintf(stderr, "vdelivermail: Invalid number of arguments\n");
		vdl_exit(111);
	}
	lowerit(user);
	lowerit(domain);
#ifdef QMAIL_EXT
	if (getenv("QMAIL_EXT"))
	{
		for (i = 0;user[i] && user[i] != '-';i++)
			user_ext[i] = user[i];
		user_ext[i] = 0;
	}
#endif
	if (!(tmpstr = vget_real_domain(domain)))
	{
		fprintf(stderr, "%s: No such domain\n", domain);
		if (userNotFound)
			vdl_exit(100);
		else
			vdl_exit(111);
	} else
		scopy(domain, tmpstr, AUTH_SIZE);
}

static int
processMail(struct passwd *pw, char *user, char *domain, mdir_t MsgSize)
{
	char            TheDir[AUTH_SIZE], tmpbuf[AUTH_SIZE];
	char           *ptr, *maildirfolder;
	int             ret;
	mdir_t          cur_size, mail_size_limit, cur_count = 0;
#ifdef USE_MAILDIRQUOTA
	mdir_t          mail_count_limit;
#endif
	mdir_t          MailQuotaSize, MailQuotaCount;
	uid_t           uid;
	gid_t           gid;

	if (!vget_assign(domain, NULL, AUTH_SIZE, &uid, &gid))
	{
		fprintf(stderr, "%s: domain does not exist\n", domain);
		vdl_exit(111);
	}
	/*
	 * check if the account is locked and their email
	 * should be bounced back
	 */
	if (pw->pw_gid & BOUNCE_MAIL)
	{
		snprintf(tmpbuf, AUTH_SIZE, "%s/Maildir", pw->pw_dir);
#ifdef USE_MAILDIRQUOTA
		if ((mail_size_limit = parse_quota(pw->pw_shell, &mail_count_limit)) == -1)
		{
			fprintf(stderr, "parse_quota: %s: %s\n", pw->pw_shell, strerror(errno));
			return (-1);
		}
		cur_size = recalc_quota(tmpbuf, &cur_count, mail_size_limit, mail_count_limit, 0);
#else
		mail_size_limit = atol(pw->pw_shell);
		cur_size = recalc_quota(tmpbuf, 0);
#endif
		/* Remove bounce flag if a message size of 1024000 can be delivered to the user */
		if (strncmp(pw->pw_shell, "NOQUOTA", 7) && ((cur_size + 1024000) < mail_size_limit))
			vset_lastdeliver(user, domain, 0);
		else
		{
			getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
			if (!access(ptr, X_OK))
			{
				/*
				 * Call overquota command with 5 arguments
				 */
				if ((maildirfolder = (char *) getenv("MAILDIRFOLDER")))
					snprintf(TheDir, sizeof(TheDir), "%s %s/Maildir/%s %"PRIu64" %"PRIu64" %"PRIu64" %s",
						ptr, pw->pw_dir, maildirfolder, MsgSize, cur_size, cur_count, pw->pw_shell);
				else
					snprintf(TheDir, sizeof(TheDir), "%s %s/Maildir %"PRIu64" %"PRIu64" %"PRIu64" %s",
						ptr, pw->pw_dir, MsgSize, cur_size, cur_count, pw->pw_shell);
				runcmmd(TheDir, 0);
			}
			fprintf(stderr, "Account %s@%s locked/overquota %"PRIu64"/%"PRIu64". indimail (#5.1.1)", user, domain, cur_size, mail_size_limit);
			getEnvConfigStr(&ptr, "HOLDOVERQUOTA", "holdoverquota");
			if (ptr && *ptr == '/')
				scopy(TheDir, ptr, sizeof(TheDir));
			else
				snprintf(TheDir, sizeof(TheDir), "%s/%s", tmpbuf, ptr);
			if (!access(TheDir, F_OK))
				vdl_exit(111);
			vdl_exit(100);
		}
	}
	/*
	 * check for a .qmail file in Maildir or valias table
	 * If either exists, then carry out delivery instructions
	 * and skip Maildir delivery
	 */
	ptr = (char *) getenv("EXT");
	if (ptr && *ptr && doAlias(pw->pw_dir, user, domain, MsgSize) == 1)
		vdl_exit(0);
	if (!strncmp(pw->pw_gecos, "MailGroup ", 10))
	{
		fprintf(stderr, "Mail delivery to group is avoided");
		return(0);
	}
	snprintf(TheDir, AUTH_SIZE, "%s/Maildir/", pw->pw_dir);
	ptr = process_dir(TheDir, uid, gid);
	ret = deliver_mail(ptr, MsgSize, pw->pw_shell, uid, gid, domain,
		&MailQuotaCount, &MailQuotaSize);
	if (ret == -1 || ret == -4)
	{
		if (ret == -1)
			fprintf(stderr, "%s@%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
				user, domain, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
		vdl_exit(100);
	} else
	if (ret == -2)
	{
		fprintf(stderr, "temporary system error: %s\n", strerror(errno));
		vdl_exit(111);
	} else
	if (ret == -3) /* mail is looping */
	{
		fprintf(stderr, "%s is looping\n", TheDir);
		vdl_exit(100);
	}
	else
	if (ret == -5) /*- Defer Overquota mails */
	{
		fprintf(stderr, "%s@%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
			user, domain, MsgSize, CurBytes, CurCount, MailQuotaSize, MailQuotaCount);
		vdl_exit(111);
	}
	return(0);
}

/*
 * Check if the indimail user has a .qmail file in their directory
 * and foward to each email address, Maildir or program 
 *  that is found there in that file
 *
 * Return:  1 if we found and delivered email
 *       :  0 if not found
 *       : -1 if no user .qmail file 
 */
int
doAlias(char *dir, char *user, char *domain, mdir_t MsgSize)
{
	static char     qmail_line[BUFF_SIZE];
	char            tmpbuf[BUFF_SIZE];
	mdir_t          MailQuotaSize, MailQuotaCount;
	FILE           *fs;
	char           *ptr;
	int             ret = 0;

	if ((ptr =getenv("NOALIAS")) && *ptr > '0')
		return (0);
#ifdef VALIAS
	/*- process valiases if configured */
	if (process_valias(user, domain, MsgSize))
		return(1);
#endif
	/*- format the file name */
	snprintf(tmpbuf, BUFF_SIZE, "%s/.qmail", dir);
	if ((fs = fopen(tmpbuf, "r")) == NULL)
		/*- no file, so just return */
		return (-1);
	/*- format a simple loop checker name */
	snprintf(tmpbuf, BUFF_SIZE, "%s@%s", user, domain);
	while (fgets(qmail_line, BUFF_SIZE, fs) != NULL)
	{
		if ((ptr = strrchr(qmail_line, '\n')) != NULL)
			*ptr = 0;
		/*
		 * simple loop check, if they are sending it to themselves
		 * then skip this line
		 */
		if (!strncmp(qmail_line, tmpbuf, BUFF_SIZE))
			continue;
		if (qmail_line[0] == '/')
			ptr = process_dir(qmail_line, indimailuid, indimailgid);
		else
			ptr = qmail_line;
		putenv("NOALIAS=2");
		ret = deliver_mail(ptr, MsgSize, "AUTO", indimailgid, indimailgid, domain,
			&MailQuotaSize, &MailQuotaCount);
		unsetenv("NOALIAS");
		if (ret == 99)
			break;
		if (ret == -1 || ret == -4)
		{
			if (ret == -1)
				fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
					qmail_line, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
			vdl_exit(100);
		} else
		if (ret == -2)
			vdl_exit(111);
		else
		if (ret == -3) /* mail is looping */
		{
			fprintf(stderr, "%s is looping\n", qmail_line);
			vdl_exit(100);
		} else
		if (ret == -5) /*- Defer Overquota mails */
		{
			fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
				qmail_line, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
			vdl_exit(111);
		}
	}
	fclose(fs);
	/*-
	 * A Blank .qmail file will result in mails getting blackholed
	 */
	return (1);
}

int
reject_mail(char *user, char *domain, int status, mdir_t MsgSize, char *bounce)
{
	int             ret, i;
	mdir_t          MailQuotaSize, MailQuotaCount;
	char            route[256];
	char           *ptr;

	/*
	 * the indimail user does not exist. Follow the rest of 
	 * the directions in the .qmail-default file
	 *
	 * If they want to delete email for non existent users
	 * then just exit 0. Qmail will delete the email for us
	 */
	if (!strncmp(bounce, DELETE_ALL, AUTH_SIZE))
		vdl_exit(0);
	else
	if (!strncmp(bounce, BOUNCE_ALL, AUTH_SIZE))
	{
		if (status == 0)
			fprintf(stderr, "Account %s@%s is inactive. indimail(#5.1.3)", user, domain);
		else
		if (status == 1)
			fprintf(stderr, "No Account %s@%s here by that name. indimail (#5.1.5)", user, domain);
		vdl_exit(100);
	}
	/*
	 * Last case: the last parameter is a Maildir, an email address, ipaddress or hostname
	 */
	if (status == 0)
		fprintf(stderr, "Account %s@%s is inactive delivering to %s. indimail(#5.1.7) ", user, domain, bounce);
	else
	if (status == 1)
		fprintf(stderr, "No Account %s@%s - delivering to %s. indimail (#5.1.6) ", user, domain, bounce);
	if (*bounce != '/' && !strchr(bounce, '@')) /*- IP Address */
	{
		if (status == 0)
			vdl_exit(100);
		for (i = 0, ptr = bounce;*ptr;ptr++)
		{
			if (*ptr == ':')
				i++;
		}
		if (i != 2)
		{
			fprintf(stderr, "Invalid SMTPROUTE Specification [%s]. indimail (#5.1.8)", bounce);
			vdl_exit(111);
		}
		snprintf(route, sizeof(route), "SMTPROUTE=%s", bounce);
		putenv(route);
		switch (qmail_remote(user, domain))
		{
			case -1:
				fprintf(stderr, "%s@%s has insufficient quota. indimail (#5.1.4)", user, domain);
				vdl_exit(100);
				break;
			case -2:
				fprintf(stderr, "system error: %s\n", strerror(errno));
				vdl_exit(111);
				break;
			case 0:
				vdl_exit(0);
				break;
			case 100:
				vdl_exit(100);
				break;
			case 111:
				vdl_exit(111);
				break;
		} /*- switch() */
	} else /*- check if it is a path add the /Maildir/ for delivery */
	if (*bounce == '/' && !strstr(bounce, "/Maildir/"))
		scat(bounce, "/Maildir/", AUTH_SIZE);
	/*- Send the email out, if we get a -1 then the user is over quota */
	ret = deliver_mail(bounce, MsgSize, "AUTO", indimailuid, indimailgid, domain, &MailQuotaSize, &MailQuotaCount);
	if (ret == -1 || ret == -4)
	{
		if (ret == -1)
			fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
				bounce, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
		vdl_exit(100);
	} else
	if (ret == -2)
	{
		fprintf(stderr, "system error: %s\n", strerror(errno));
		vdl_exit(111);
	} else
	if (ret == -3) /* mail is looping */
	{
		fprintf(stderr, "%s is looping\n", bounce);
		vdl_exit(100);
	} else
	if (ret == -5) /*- Defer Overquota mails */
	{
		fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
			bounce, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
		vdl_exit(111);
	}
	return(0);
}

static int
bhfcheck(char *addr)
{
	char           *ptr, *mapbhf, *sysconfdir, *controldir;
	char            tmpbuf[MAX_BUFF];
	int             i, j, fd, count, k = 0;
	struct stat     statbuf;
	char            subvalue;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(tmpbuf, sizeof(tmpbuf), "%s/blackholedsender", controldir);
	else
		snprintf(tmpbuf, sizeof(tmpbuf), "%s/%s/blackholedsender", sysconfdir, controldir);
	if (!stat(tmpbuf, &statbuf))
	{
		if (!(mapbhf = (char *) malloc(statbuf.st_size)))
		{
			fprintf(stderr, "malloc: %"PRIu64": %s\n", (mdir_t) statbuf.st_size, strerror(errno));
			vdl_exit(111);
		}
		if ((fd = open(tmpbuf, O_RDONLY)) == -1)
		{
			fprintf(stderr, "open: %s: %s\n", tmpbuf, strerror(errno));
			free(mapbhf);
			vdl_exit(111);
		}
		if ((count = read(fd, mapbhf, statbuf.st_size)) != statbuf.st_size)
		{
			fprintf(stderr, "read: %"PRIu64": %s\n", (mdir_t) statbuf.st_size, strerror(errno));
			free(mapbhf);
			close(fd);
			vdl_exit(111);
		}
		close(fd);
		for (ptr = mapbhf;ptr < mapbhf + statbuf.st_size;ptr++)
		{
			if (*ptr == '\n')
				*ptr = 0;
		}
		i = 0;
		for (j = 0; j < count; ++j)
		{
			if (!mapbhf[j])
			{
				if (mapbhf[i]) /*- Not a blank line */
				{
					if (mapbhf[i] == '@')
					{
						if (!(ptr = strchr(addr, '@')))
						{
							free(mapbhf);
							return(0);
						}
					} else
						ptr = addr;
					if (!strncmp(ptr, mapbhf + i, slen(mapbhf + i)))
					{
						free(mapbhf);
						return(1);
					}
				}
				i = j + 1;
			}
		}
		free(mapbhf);
		return(0);
	}
	if (*controldir == '/')
		snprintf(tmpbuf, sizeof(tmpbuf), "%s/blackholedpatterns",  controldir);
	else
		snprintf(tmpbuf, sizeof(tmpbuf), "%s/%s/blackholedpatterns", sysconfdir, controldir);
	if (!stat(tmpbuf, &statbuf))
	{
		if (!(mapbhf = (char *) malloc(statbuf.st_size)))
		{
			fprintf(stderr, "malloc: %"PRIu64": %s\n", (mdir_t) statbuf.st_size, strerror(errno));
			vdl_exit(111);
		}
		if ((fd = open(tmpbuf, O_RDONLY)) == -1)
		{
			fprintf(stderr, "open: %s: %s\n", tmpbuf, strerror(errno));
			vdl_exit(111);
		}
		if ((count = read(fd, mapbhf, statbuf.st_size)) != statbuf.st_size)
		{
			fprintf(stderr, "read: %"PRIu64": %s\n", (mdir_t) statbuf.st_size, strerror(errno));
			close(fd);
			vdl_exit(111);
		}
		close(fd);
		for (ptr = mapbhf;ptr < mapbhf + statbuf.st_size;ptr++)
		{
			if (*ptr == '\n')
				*ptr = 0;
		}
		i = 0;
		for (j = 0; j < count; ++j)
		{
			if (!mapbhf[j]) /*- Not a blank line */
			{
				if (mapbhf[i])
				{
					subvalue = mapbhf[i] != '!';
					if (!subvalue)
						i++;
					if ((k != subvalue) && wildmat(addr, mapbhf + i))
						k = subvalue;
				}
				i = j + 1;
			}
		}
		free(mapbhf);
		return k;
	}
	return (0);
}

static char *process_dir(char *dir, uid_t uid, gid_t gid)
{
	int             i;
	char           *maildirfolder, TheDir[AUTH_SIZE];
	static char     tmpdir[AUTH_SIZE];
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};
	maildirfolder = (char *) getenv("MAILDIRFOLDER");
	for (i = 0;i < 3;i++)
	{
		if (maildirfolder)
			snprintf(TheDir, AUTH_SIZE, "%s/%s/%s", dir, maildirfolder, MailDirNames[i]);
		else
			snprintf(TheDir, AUTH_SIZE, "%s/%s", dir, MailDirNames[i]);
		if (access(TheDir, F_OK))
		{
			if (r_mkdir(TheDir, INDIMAIL_DIR_MODE, uid, gid))
			{
				fprintf(stderr, "access: %s: %s. indimail (#5.1.2)", TheDir, strerror(errno));
				vdl_exit(111);
			}
			snprintf(TheDir, MAX_BUFF, "%s/%s/maildirfolder", dir, maildirfolder);
			close(open(TheDir, O_CREAT | O_TRUNC , 0644));
		}
	}
	if (maildirfolder)
		snprintf(tmpdir, AUTH_SIZE, "%s/%s/", dir, maildirfolder);
	else
		scopy(tmpdir, dir, AUTH_SIZE);
	return tmpdir;
}

#ifdef VALIAS
/* 
 * Process any valiases for this user@domain
 * 
 * This will look up any valiases in indimail and
 * deliver the email to the entries
 *
 * Return 1 if aliases found
 * Return 0 if no aliases found 
 */
int
process_valias(char *user, char *domain, mdir_t MsgSize)
{
	int             ret, found = 0, status = 0;
	mdir_t          MailQuotaSize, MailQuotaCount;
	char           *tmpstr;

	/*- Get the aliases for this user@domain */
	for (status = found = 0;;found++)
	{
		if (!(tmpstr = valias_select(user, domain)))
			break;
		if (tmpstr[0] == '/')
			tmpstr = process_dir(tmpstr, indimailuid, indimailgid);
		putenv("NOALIAS=1");
		ret = deliver_mail(tmpstr, MsgSize, "AUTO", indimailuid, indimailgid, domain, &MailQuotaSize, &MailQuotaCount);
		unsetenv("NOALIAS");
		if (ret == -1 || ret == -4)
		{
			if (status++)
				printf("\n\n");
			if (ret == -1)
				fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
					tmpstr, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
			continue;
		} else
		if (ret == -2)
		{
			fprintf(stderr, "temporary system error: %s\n", strerror(errno));
			vdl_exit(111);
		} else
		if (ret == -3) /* mail is looping */
		{
			if (status++)
				printf("\n\n");
			fprintf(stderr, "%s is looping\n", tmpstr);
			continue;
		} else
		if (ret == -5) /*- Defer Overquota mails */
		{
			if (status++)
				printf("\n\n");
			fprintf(stderr, "%s has insufficient quota. %"PRIu64"/%"PRIu64":%"PRIu64"/%"PRIu64":%"PRIu64". indimail (#5.1.4)", 
				tmpstr, MsgSize, CurBytes, CurCount, MailQuotaCount, MailQuotaSize);
			vdl_exit(111);
		}
	} /*- for (found = 0;;found++) */
	if (status)
		vdl_exit(100);
	/*- Return whether we found an alias or not */
	return (found);
}
#endif

void
getversion_vdelivermail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
