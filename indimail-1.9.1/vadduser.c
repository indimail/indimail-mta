/*
 * $Log: vadduser.c,v $
 * Revision 2.41  2016-01-28 16:34:10+05:30  Cprogrammer
 * fixed buffer overflow
 *
 * Revision 2.40  2016-01-28 15:04:39+05:30  Cprogrammer
 * fixed setting of quota using domain limits
 *
 * Revision 2.39  2016-01-28 00:04:35+05:30  Cprogrammer
 * maildirquota specification for -q option to vadduser
 *
 * Revision 2.38  2014-04-17 11:42:18+05:30  Cprogrammer
 * set supplementary group ids for indimail
 *
 * Revision 2.37  2013-06-10 16:06:39+05:30  Cprogrammer
 * allow setting of NOQUOTA from vlimits
 *
 * Revision 2.36  2012-04-22 13:59:03+05:30  Cprogrammer
 * use 64bit integer for quota calculation
 *
 * Revision 2.35  2011-11-09 19:45:49+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.34  2010-11-12 21:56:35+05:30  Cprogrammer
 * return error code 17 for EEXIST
 *
 * Revision 2.33  2010-08-10 18:41:05+05:30  Cprogrammer
 * made dir control user level configurable
 *
 * Revision 2.32  2010-05-17 10:16:13+05:30  Cprogrammer
 * use control file .base_path in domains directory
 *
 * Revision 2.31  2010-05-17 00:00:00+05:30  Cprogrammer
 * fixed setting of BASE PATH
 *
 * Revision 2.30  2010-02-16 13:08:47+05:30  Cprogrammer
 * added post_handle function
 *
 * Revision 2.29  2009-12-30 13:10:17+05:30  Cprogrammer
 * run vadduser with uid, gid of domain
 *
 * Revision 2.28  2009-12-02 11:04:34+05:30  Cprogrammer
 * use .domain_limits in domain directory to turn on domain limits
 *
 * Revision 2.27  2009-12-01 16:28:14+05:30  Cprogrammer
 * added checking of domain limits for user quota
 *
 * Revision 2.26  2009-10-14 20:46:06+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.25  2009-10-10 11:41:10+05:30  Cprogrammer
 * run vadduser only as root or indimail
 *
 * Revision 2.24  2009-08-15 20:48:59+05:30  Cprogrammer
 * display usage information for -B basepath option
 *
 * Revision 2.23  2009-08-05 14:38:29+05:30  Cprogrammer
 * added option to specify base path on command line
 *
 * Revision 2.22  2008-09-17 17:03:53+05:30  Cprogrammer
 * added hostid argument
 *
 * Revision 2.21  2008-08-02 20:51:21+05:30  Cprogrammer
 * fixed error with error_stack usage
 *
 * Revision 2.20  2008-08-02 09:09:17+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.19  2008-06-30 16:17:05+05:30  Cprogrammer
 * removed license code
 *
 * Revision 2.18  2008-06-13 10:34:51+05:30  Cprogrammer
 * do not pass mdahost for non clustered compilation
 *
 * Revision 2.17  2008-06-03 19:47:50+05:30  Cprogrammer
 * conditional compilation of license code
 * pass mdahost argument to vadduser
 * create home directory only if mdahost is not set
 *
 * Revision 2.16  2008-05-28 15:20:19+05:30  Cprogrammer
 * removed cdb module
 *
 * Revision 2.15  2006-01-26 00:54:25+05:30  Cprogrammer
 * fix for NOQUOTA
 *
 * Revision 2.14  2005-12-29 22:51:02+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.13  2004-09-21 23:40:06+05:30  Cprogrammer
 * added 'i' flag to create user as inactive
 *
 * Revision 2.12  2004-07-03 23:53:44+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.11  2004-06-20 16:37:33+05:30  Cprogrammer
 * rename ISOCOR_BASE_PATH to BASE_PATH
 *
 * Revision 2.10  2004-05-19 20:01:24+05:30  Cprogrammer
 * added len argument to random passwd generation
 *
 * Revision 2.9  2004-05-10 18:21:06+05:30  Cprogrammer
 * added VERSION check for license
 *
 * Revision 2.8  2003-12-08 20:57:40+05:30  Cprogrammer
 * default_table not initialized - caused SIGSEGV
 *
 * Revision 2.7  2003-12-06 17:30:46+05:30  Cprogrammer
 * added license check for user count in indimail
 *
 * Revision 2.6  2003-10-26 18:42:36+05:30  Cprogrammer
 * added missing initialization of quota
 *
 * Revision 2.5  2003-10-26 11:35:10+05:30  Cprogrammer
 * use defaultquota from limits if quota not given on command line
 *
 * Revision 2.4  2003-10-23 13:21:34+05:30  Cprogrammer
 * change for argument added to genpass()
 *
 * Revision 2.3  2002-10-12 23:01:56+05:30  Cprogrammer
 * added conditional compilation of clustered code which was causing compilation problems
 *
 * Revision 2.2  2002-08-11 14:13:29+05:30  Cprogrammer
 * corrected option -b not working
 *
 * Revision 2.1  2002-08-11 00:29:04+05:30  Cprogrammer
 * added option to specify the Mailstore host and option to balance the filesystem
 *
 * Revision 1.7  2002-02-24 22:46:28+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.6  2001-12-27 01:29:14+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.5  2001-12-08 17:45:16+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.4  2001-11-28 23:02:06+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:20:27+05:30  Cprogrammer
 * added sccsidh
 *
 * Revision 1.2  2001-11-20 10:56:22+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.1  2001-10-24 18:15:18+05:30  Cprogrammer
 * Initial revision
 *
 * 
 * Copyright (C) 1999 Inter7 Internet Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "indimail.h"
#include <stdio.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vadduser.c,v 2.41 2016-01-28 16:34:10+05:30 Cprogrammer Exp mbhangui $";
#endif

char            Email[MAX_BUFF], User[MAX_BUFF], Domain[MAX_BUFF], Passwd[MAX_BUFF],
                Quota[QUOTA_BUFLEN], Gecos[MAX_BUFF], envbuf[MAX_BUFF];
#ifdef CLUSTERED_SITE
char            mdahost[MAX_BUFF], hostid[MAX_BUFF];
#endif
int             apop, Random, balance_flag, actFlag = 1;

extern int      encrypt_flag, create_flag;

void            usage();
int             get_options(int, char **, char **, int *, int *);
int             checklicense(char *, int, long, char *, int);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i, pass_len = 8, users_per_level = 0;
	char           *real_domain, *ptr, *base_argv0, *base_path, *domain_dir;
	char            tmpbuf[MAX_BUFF], buffer[MAX_BUFF], quotaVal[QUOTA_BUFLEN];
	FILE           *fp;
	uid_t           uid, uidtmp;
	gid_t           gid;
#ifdef ENABLE_DOMAIN_LIMITS
	int             domain_limits;
	struct vlimits  limits;
#endif

	if (get_options(argc, argv, &base_path, &pass_len, &users_per_level))
		return(1);
	/*
	 * parse the email address into user and domain 
	 */
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", Email);
		return(1);
	}
	/* Do this so that users do not get added in a alias domain */
	real_domain = (char *) 0;
	if (*Domain && !(real_domain = vget_real_domain(Domain)))
	{
		error_stack(stderr, "%s: No such domain\n", Domain);
		return(1);
	}
	if (!(domain_dir = vget_assign(real_domain, 0, AUTH_SIZE, &uid, &gid)))
	{
		error_stack(stderr, "%s: domain does not exist\n", real_domain);
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	snprintf(tmpbuf, MAX_BUFF, "%s/.domain_limits", domain_dir);
	domain_limits = ((access(tmpbuf, F_OK) && !getenv("DOMAIN_LIMITS")) ? 0 : 1);
	if (domain_limits && vget_limits(real_domain, &limits))
	{
		error_stack(stderr, "Unable to get domain limits for %s\n", real_domain);
		return(1);
	}
	if (*Quota && (limits.perm_defaultquota & VLIMIT_DISABLE_CREATE))
	{
		error_stack(stderr, "-q option not allowed for %s\n", real_domain);
		return(1);
	}
#endif
	/*
	 * if the comment field is blank use the user name 
	 */
	if (Gecos[0] == 0)
		scopy(Gecos, User, MAX_BUFF);
	/*
	 * get the password if not set on command line 
	 */
	if (Random && !*Passwd)
	{
		scopy(Passwd, genpass(pass_len), MAX_BUFF);
	} else
	if (!*Passwd)
		scopy(Passwd, vgetpasswd(Email), MAX_BUFF);
	if (!*Passwd)
	{
		error_stack(stderr, "Please input password\n");
		usage();
		return(1);
	}
	uidtmp = getuid();
	if (uidtmp != 0 && uidtmp != uid)
	{
		error_stack(stderr, "you must be root or domain user (uid=%d) to run this program\n", uid);
		return(1);
	}
	if (setuser_privileges(uid, gid, "indimail"))
	{
		error_stack(stderr, "setuser_privilege: (%d/%d): %s", uid, gid, strerror(errno));
		return(1);
	}
	/* set the users quota if set on the command line */
	if (*Quota) {
		if (strncmp(Quota, "NOQUOTA", 8)) {
			snprintf(quotaVal, sizeof(quotaVal), "%"PRId64"", parse_quota(Quota, 0));
			if (!(ptr = strchr(quotaVal, ','))) {
				if ((ptr = strchr(Quota, ',')))
					scat(quotaVal, ptr, sizeof(quotaVal));
			}
		}
	} else
#ifdef ENABLE_DOMAIN_LIMITS
	if (domain_limits) {
		if (limits.defaultquota)
			snprintf(quotaVal, sizeof(quotaVal), "%"PRId64"", limits.defaultquota);
		else
			*quotaVal = 0;
	} else
		*quotaVal = 0;
#else
		*quotaVal = 0;
#endif
#ifdef CLUSTERED_SITE
	if (!*mdahost && *hostid)
	{
		if (!(ptr = vauth_getipaddr(hostid)))
		{
			error_stack(stderr, "Failed to obtain mdahost for host %s domain %s\n", hostid, real_domain);
			return(1);
		} else
			scopy(mdahost, ptr, MAX_BUFF);
	}
	/* add the user */
	if (*mdahost)
	{
		if (!(ptr = SqlServer(mdahost, real_domain)))
		{
			error_stack(stderr, "Failed to obtain sqlserver for mdahost %s domain %s\n", mdahost, real_domain);
			return(1);
		} else
		if (vauth_open(ptr))
		{
			error_stack(stderr, "Failed to connect to %s\n", ptr);
			return(1);
		}
		if (verbose)
			printf("Adding to MDAhost %s SqlServer %s\n", mdahost, ptr);
	} 
#endif
	*envbuf = 0;
	snprintf(tmpbuf, sizeof(tmpbuf), "%s/.base_path", domain_dir);
	if ((fp = fopen(tmpbuf, "r")))
	{
		if (fscanf(fp, "%s", buffer) != 1)
		{
			error_stack(stderr, "%s: premature eof\n", tmpbuf);
			fclose(fp);
			return (1);
		}
		fclose(fp);
		snprintf(envbuf, sizeof(envbuf), "BASE_PATH=%s", buffer);
	}
	if (base_path)
		snprintf(envbuf, sizeof(envbuf), "BASE_PATH=%s", base_path);
	if (balance_flag)
	{
		snprintf(tmpbuf, sizeof(tmpbuf), "%s/etc/lastfstab", INDIMAILDIR);
		if (!(fp = fopen(tmpbuf, "r")))
		{
			perror(tmpbuf);
			return(1);
		}
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
		{
			error_stack(stderr, "%s: premature eof\n", tmpbuf);
			fclose(fp);
			return(1);
		}
		fclose(fp);
		if ((ptr = strchr(buffer, '\n')))
			*ptr = 0;
		snprintf(envbuf, sizeof(envbuf), "BASE_PATH=%s", buffer);
	}
	if (*envbuf)
		putenv(envbuf);
#ifdef CLUSTERED_SITE
	if ((i = vadduser(User, real_domain, mdahost, Passwd, Gecos, quotaVal,
		users_per_level, apop, actFlag)) < 0)
#else
	if ((i = vadduser(User, real_domain, 0, Passwd, Gecos, quotaVal,
		users_per_level, apop, actFlag)) < 0)
#endif
	{
		if (errno == EEXIST)
			i = errno;
		error_stack(stderr, 0);
		vclose();
		return(i);
	}
	vclose();
	if (Random)
		printf("Password is %s\n", Passwd);
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_handle("%s/libexec/%s %s@%s", INDIMAILDIR, base_argv0, User, real_domain));
	} else
		return(post_handle("%s %s@%s", ptr, User, real_domain));
}

void
usage()
{
	error_stack(stderr, "usage: vadduser [options] email_address [passwd]\n");
	error_stack(stderr, "options: -V          (print version number)\n");
	error_stack(stderr, "         -v          (verbose)\n");
	error_stack(stderr, "         -q          quota (in bytes) (sets the users quota)\n");
	error_stack(stderr, "         -l level    users per level\n");
	error_stack(stderr, "         -c          comment (sets the gecos comment field)\n");
	error_stack(stderr, "         -e          Standard Encrypted Password\n");
	error_stack(stderr, "         -r [len]    generate a len (default 8) char random password\n");
	error_stack(stderr, "         -b          Balance distribution across filesystems\n");
	error_stack(stderr, "         -B basepath Specify the base directory for user's home directory\n");
	error_stack(stderr, "         -d          Create the homedir (ignored if -h option is given)\n");
#ifdef CLUSTERED_SITE
	error_stack(stderr, "         -m mdahost  (host on which the account needs to be created - specify mdahost)\n");
	error_stack(stderr, "         -h hostid   (host on which the account needs to be created - specify hostid)\n");
#endif
	error_stack(stderr, "         -a          (sets the account to use APOP, default is POP)\n");
	error_stack(stderr, "         -i          (sets the account as inactive)\n");
}

int
get_options(int argc, char **argv, char **base_path, int *pass_len, int *users_per_level)
{
	int             c;
	int             errflag;

	memset(Email, 0, MAX_BUFF);
	memset(Passwd, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(Quota, 0, QUOTA_BUFLEN);
	apop = USE_POP;
	errflag = 0;
	actFlag = 1;
	*base_path = 0;
#ifdef CLUSTERED_SITE
	while (!errflag && (c = getopt(argc, argv, "aidbB:vc:q:l:h:m:er:")) != -1)
#else
	while (!errflag && (c = getopt(argc, argv, "aidbB:vc:q:l:er:")) != -1)
#endif
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'r':
			Random = 1;
			if (optarg)
				*pass_len = atoi(optarg);
			break;
		case 'i':
			actFlag = 0;
			break;
		case 'a':
			apop = USE_APOP;
			break;
		case 'c':
			scopy(Gecos, optarg, MAX_BUFF);
			break;
		case 'e':
			encrypt_flag = 1;
			break;
		case 'd':
#ifdef CLUSTERED_SITE
			if (!*mdahost && !*hostid)
				create_flag = 1;
#else
			create_flag = 1;
#endif
			break;
		case 'b':
			balance_flag = 1;
			break;
		case 'B':
			*base_path = optarg;
			break;
		case 'q':
			scopy(Quota, optarg, QUOTA_BUFLEN);
			break;
		case 'l':
			*users_per_level = atoi(optarg);
			break;
#ifdef CLUSTERED_SITE
		case 'm':
			scopy(mdahost, optarg, MAX_BUFF);
			create_flag = 0;
			break;
		case 'h':
			scopy(hostid, optarg, MAX_BUFF);
			create_flag = 0;
			break;
#endif
		default:
			errflag = 1;
			break;
		}
	}

	if (optind < argc)
		scopy(Email, argv[optind++], MAX_BUFF);
	if (optind < argc)
		scopy(Passwd, argv[optind++], MAX_BUFF);
	if (errflag || !*Email)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vadduser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
