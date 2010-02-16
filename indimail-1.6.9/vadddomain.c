/*
 * $Log: vadddomain.c,v $
 * Revision 2.27  2010-02-16 13:08:35+05:30  Cprogrammer
 * added post_hook function
 *
 * Revision 2.26  2010-02-16 09:29:37+05:30  Cprogrammer
 * added abuse, mailer-daemon as alias to postmaster
 *
 * Revision 2.25  2009-09-28 13:47:34+05:30  Cprogrammer
 * added -C option to allow selection of recipient check for a domain
 *
 * Revision 2.24  2009-03-06 19:53:55+05:30  Cprogrammer
 * allow adddomain to proceed if postmaster id exists
 *
 * Revision 2.23  2009-02-18 21:34:19+05:30  Cprogrammer
 * check chown for error
 *
 * Revision 2.22  2008-09-30 08:38:56+05:30  Cprogrammer
 * run vadddomain only for root/indimail
 *
 * Revision 2.21  2008-09-17 22:31:53+05:30  Cprogrammer
 * replaced error statements with error_stack
 *
 * Revision 2.20  2008-08-02 09:09:07+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.19  2008-06-13 10:34:07+05:30  Cprogrammer
 * fixed conditional compilation of VFILTER, CLUSTERED_SITE code
 *
 * Revision 2.18  2008-06-05 16:21:40+05:30  Cprogrammer
 * moved vdelivermail to sbin
 *
 * Revision 2.17  2008-06-03 19:47:27+05:30  Cprogrammer
 * added mdahost argument to vadduser()
 *
 * Revision 2.16  2008-05-28 15:18:54+05:30  Cprogrammer
 * removed ldap module
 *
 * Revision 2.15  2005-12-29 22:50:59+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.14  2004-09-21 23:39:32+05:30  Cprogrammer
 * change for actFlag argument to vauth_adduser()
 *
 * Revision 2.13  2003-10-03 01:22:44+05:30  Cprogrammer
 * display ATRN support in usage
 *
 * Revision 2.12  2003-06-01 13:08:29+05:30  Cprogrammer
 * allow addition of Maildir path to BounceEmail
 *
 * Revision 2.11  2003-01-28 23:28:01+05:30  Cprogrammer
 * added option -c for clustered domain
 *
 * Revision 2.10  2003-01-26 02:15:32+05:30  Cprogrammer
 * setuid() to allow updation of assign file
 *
 * Revision 2.9  2002-12-29 02:48:46+05:30  Cprogrammer
 * use dbinfoAdd() only if distributed is set
 * update the mcd file if dbinfo is updated
 *
 * Revision 2.8  2002-12-08 20:08:24+05:30  Cprogrammer
 * option to add clustered domain
 *
 * Revision 2.7  2002-11-26 20:42:24+05:30  Cprogrammer
 * adding of postmaster on clustered domain now handled correctly
 *
 * Revision 2.6  2002-11-25 00:17:53+05:30  Cprogrammer
 * added check for postmaster in a distributed domain
 *
 * Revision 2.5  2002-10-13 02:25:55+05:30  Cprogrammer
 * added -f option to use vfilter as mda
 *
 * Revision 2.4  2002-08-30 00:12:16+05:30  Cprogrammer
 * print usage for -T option
 *
 * Revision 2.3  2002-08-25 22:34:48+05:30  Cprogrammer
 * added code for autoturn and etrn domains
 *
 * Revision 2.2  2002-06-26 03:18:37+05:30  Cprogrammer
 * VLDAP_USER changed to LDAP_USER
 *
 * Revision 2.1  2002-05-09 00:37:02+05:30  Cprogrammer
 * changed variable name TmpBuf1 to TmpBuf
 *
 * Revision 1.11  2002-02-24 22:46:24+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.10  2002-02-23 01:55:36+05:30  Cprogrammer
 * added check for validity of the domain name
 *
 * Revision 1.9  2002-02-17 00:18:17+05:30  Cprogrammer
 * removed hardcoding of constant 156
 *
 * Revision 1.8  2001-12-27 01:28:21+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.7  2001-12-14 10:34:03+05:30  Cprogrammer
 * delete domain if vadduser on postmaster fails
 *
 * Revision 1.6  2001-12-08 17:45:11+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.5  2001-12-03 02:07:31+05:30  Cprogrammer
 * create postmaster directory by default
 *
 * Revision 1.4  2001-11-28 23:01:56+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:20:25+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:20+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:17+05:30  Cprogrammer
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vadddomain.c,v 2.27 2010-02-16 13:08:35+05:30 Cprogrammer Exp mbhangui $";
#endif


char            Domain[MAX_BUFF], Passwd[MAX_BUFF], User[MAX_BUFF], Dir[MAX_BUFF], Quota[MAX_BUFF], BounceEmail[MAX_BUFF];
char            TmpBuf[MAX_BUFF], ipaddr[16], a_dir[MAX_BUFF];
#ifdef CLUSTERED_SITE
char            database[MAX_BUFF], sqlserver[MAX_BUFF], database[MAX_BUFF], dbuser[MAX_BUFF], dbpass[MAX_BUFF];
int             dbport, distributed;
#endif
int             Apop, Bounce;
uid_t           Uid, a_uid;
gid_t           Gid, a_gid;

void            usage();
int             get_options(int argc, char **argv, int *);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err, chk_rcpt, i;
	uid_t           uid;
	gid_t           gid;
	extern int      create_flag;
	char           *qmaildir, *ptr, *base_argv0;
	FILE           *fs;
	char            AliasLine[MAX_BUFF];
	char           *auto_ids[] = {
		"abuse",
		"mailer-daemon",
		0
	};
#ifdef CLUSTERED_SITE
	char           *hostid, *localIP;
	char            email[MAX_BUFF];
	int             is_dist, user_present, total;
#endif

	if (get_options(argc, argv, &chk_rcpt))
		return(1);
	if (!isvalid_domain(Domain))
	{
		error_stack(stderr, "Invalid domain: %s\n", Domain);
		return(1);
	}
	if (!use_etrn && slen(Passwd) <= 0)
		scopy(Passwd, vgetpasswd("postmaster"), MAX_BUFF);
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	if (uid != 0 && uid != indimailuid)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if (uid & setuid(0))
	{
		error_stack(stderr, "setuid: %s\n", strerror(errno));
		return(1);
	}
	if (!*Dir)
	{
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		if (use_etrn)
		{
			if (!(ptr = vget_assign("autoturn", 0, 0, &uid, &gid)))
				snprintf(Dir, MAX_BUFF, "%s/autoturn", qmaildir);
			else
			{
				Uid = uid;
				Gid = gid;
				scopy(Dir, ptr, MAX_BUFF);
			}
		} else
			scopy(Dir, INDIMAILDIR, MAX_BUFF);
	}
	/*
	 * add domain to virtualdomains and optionally to chkrcptdomains
	 * add domain to /var/indimail/users/assign, /var/indimail/users/cdb
	 * create .qmail-default file
	 */
	if ((err = vadddomain(Domain, ipaddr, Dir, Uid, Gid, chk_rcpt)) != VA_SUCCESS)
	{
		error_stack(stderr, 0);
		vclose();
		return(err);
	}
#ifdef CLUSTERED_SITE
	/*
	 * add domain to dbinfo
	 */
	if (distributed >= 0)
	{
		if (!(localIP = get_local_ip()))
		{
			error_stack(stderr, "vadddomain: failed to get local ipaddr\n");
			vclose();
			return(1);
		}
		if (!(ptr = vhostid_select()))
		{
			if (!(hostid = get_local_hostid()))
			{
				error_stack(stderr, "vadddomain: failed to get local hostid\n");
				vclose();
				return(1);
			}
			if (vhostid_insert(hostid, localIP))
			{
				error_stack(stderr, "vadddomain: failed to get insert hostid\n");
				vclose();
				return(1);
			}
		}
		if (dbinfoAdd(Domain, distributed, sqlserver, localIP, dbport, database, dbuser, dbpass))
		{
			error_stack(stderr, "Failed to add %s domain %s\n", distributed == 1 ? "Clustered" : "NonClustered", Domain);
			vclose();
			return(1);
		}
		LoadDbInfo_TXT(&total);
	}
#endif
	if (use_etrn)
		return(0);
	if (*BounceEmail)
	{
		if (strchr(BounceEmail, '@') != NULL || *BounceEmail == '/')
		{
			vget_assign(Domain, a_dir, MAX_BUFF, &a_uid, &a_gid);
			snprintf(TmpBuf, MAX_BUFF, "%s/.qmail-default", a_dir);
			if ((fs = fopen(TmpBuf, "w+")) != NULL)
			{
				fprintf(fs, "| %s/sbin/vdelivermail '' %s\n", INDIMAILDIR, BounceEmail);
				fclose(fs);
				if (chown(TmpBuf, a_uid, a_gid) == -1)
				{
					error_stack(stderr, "chown: %s (%d:%d): %s\n", TmpBuf, a_uid, a_gid, strerror(errno));
					return(1);
				}
			} else
			{
				printf("Error: could not open %s\n", TmpBuf);
				return(1);
			}
		} else
		{
			printf("Invalid bounce email address %s\n", BounceEmail);
			return(1);
		}
	}
	create_flag = 1;
	/*
	 * Add the postmaster user
	 */
#ifdef CLUSTERED_SITE
	snprintf(email, MAX_BUFF, "postmaster@%s", Domain);
	if ((is_dist = is_distributed_domain(Domain)) == -1)
	{
		error_stack(stderr, "Unable to verify %s as a distributed domain\n", Domain);
		vclose();
		return(1);
	} else
	if (is_dist)
	{
		if ((user_present = is_user_present("postmaster", Domain)) == -1)
		{
			printf("Auth Db Error\n");
			vclose();
			return(1);
		} else
		if (user_present)
		{
			vclose();
			return(0);
		}
	}
#endif
	if ((err = vadduser("postmaster", Domain, 0, Passwd, "Postmaster", 0, Apop, 1)) != VA_SUCCESS)
	{
		if (errno != EEXIST)
		{
			error_stack(stderr, 0);
			vdeldomain(Domain);
			vclose();
			return(err);
		}
	}
	/* set quota for postmaster */
	if (Quota[0] != 0)
		vsetuserquota("postmaster", Domain, Quota);
	for(i = 0;auto_ids[i];i++)
	{
		printf("Adding alias %s@%s --> postmaster@%s\n", auto_ids[i], Domain, Domain);
		snprintf(AliasLine, sizeof(AliasLine), "&postmaster@%s", Domain);
		valias_insert(auto_ids[i], Domain, AliasLine, 0);
	}
	vclose();
	if (!(ptr = getenv("POST_HOOK")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_hook("%s/libexec/%s %s", INDIMAILDIR, base_argv0, Domain));
	} else
		return(post_hook("%s %s", ptr, Domain));
}

int
get_options(int argc, char **argv, int *chk_rcpt)
{
	int             c;
	int             errflag;
	struct passwd  *mypw;
	char            optbuf[MAX_BUFF];

	*Domain = *Passwd = *Quota = *Dir = *BounceEmail = *TmpBuf = 0;
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	Uid = indimailuid;
	Gid = indimailgid;
	Apop = USE_POP;
	Bounce = 1;
	*chk_rcpt = errflag = 0;
#ifdef CLUSTERED_SITE
	*sqlserver = *database = *dbuser = *dbpass = 0;
	dbport = -1;
	distributed = -1;
	scopy(optbuf, "atT:q:be:u:VvCci:g:d:D:S:U:P:p:O", MAX_BUFF);
#else
	scopy(optbuf, "atT:q:be:u:VvCi:g:d:O", MAX_BUFF);
#endif
#ifdef VFILTER
	scat(optbuf, "f", MAX_BUFF);
#endif
	while (!errflag && (c = getopt(argc, argv, optbuf)) != -1)
	{
		switch (c)
		{
		case 'V':
			getversion(sccsid);
			break;
		case 'v':
			verbose = 1;
			break;
		case 'd':
			scopy(Dir, optarg, MAX_BUFF);
			break;
		case 'C':
			*chk_rcpt = 1;
			break;
#ifdef CLUSTERED_SITE
		case 'c':
			distributed = 1;
			break;
		case 'D':
			if (distributed == -1)
				distributed = 0;
			scopy(database, optarg, MAX_BUFF);
			break;
		case 'S':
			scopy(sqlserver, optarg, MAX_BUFF);
			break;
		case 'U':
			scopy(dbuser, optarg, MAX_BUFF);
			break;
		case 'P':
			scopy(dbpass, optarg, MAX_BUFF);
			break;
		case 'p':
			dbport = atoi(optarg);
			break;
#endif
		case 'u':
			scopy(User, optarg, MAX_BUFF);
			break;
		case 'q':
			scopy(Quota, optarg, MAX_BUFF);
			break;
		case 'e':
			scopy(BounceEmail, optarg, MAX_BUFF);
			break;
		case 'i':
			Uid = atoi(optarg);
			break;
		case 'g':
			Gid = atoi(optarg);
			break;
		case 'b':
			Bounce = 1;
			break;
		case 'a':
			Apop = USE_APOP;
			break;
#ifdef VFILTER
		case 'f':
			use_vfilter = 1;
			break;
#endif
		case 't': /*- ETRN/ATRN Support */
			use_etrn = 1;
			break;
		case 'T': /*- AUTOTURN Support */
			scopy(ipaddr, optarg, MAX_BUFF);
			use_etrn = 2;
			break;
		case 'O':
			OptimizeAddDomain = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
		scopy(Domain, argv[optind++], MAX_BUFF);
	if (optind < argc)
		scopy(Passwd, argv[optind++], MAX_BUFF);
	if (*User)
	{
		if ((mypw = getpwnam(User)) != NULL)
		{
			if (!*Dir)
				scopy(Dir, mypw->pw_dir, MAX_BUFF);
			Uid = mypw->pw_uid;
			Gid = mypw->pw_gid;
		} else
		{
			error_stack(stderr, "user %s not found in /etc/passwd\n", User);
			return(1);
		}
	}
	if (errflag || !*Domain)
	{
		error_stack(stderr, "Domain not specified\n");
		usage();
		return(1);
	}
#ifdef CLUSTERED_SITE
	if (distributed >=0 && (!*sqlserver || !*database || !*dbuser || !*dbpass || dbport == -1))
	{
		error_stack(stderr, "specify sqlserver, database, dbuser, dbpass and dbport\n");
		usage();
		return(1);
	}
#endif
	return(0);
}

void
usage()
{
	printf("usage: vaddomain [options] virtual_domain [postmaster password]\n");
	printf("options: -V print version number\n");
	printf("         -v verbose\n");
	printf("         -q quota_in_bytes (sets the quota for postmaster account)\n");
	printf("         -C (Do recipient check for this domain)\n");
	printf("         -b (bounces all mail that doesn't match a user, default)\n");
	printf("         -e [email_address|maildir] (forwards all non matching user to this address [*])\n");
	printf("         -u user (sets the uid/gid based on a user in /etc/passwd)\n");
	printf("         -d dir (sets the dir to use for this domain)\n");
	printf("         -i uid (sets the uid to use for this domain)\n");
	printf("         -g gid (sets the gid to use for this domain)\n");
	printf("         -a sets the account to use APOP, default is POP\n");
	printf("         -f Sets the Domain with VFILTER capability\n");
	printf("         -t Sets the Domain for ETRN/ATRN\n");
	printf("         -T ip_address Sets the Domain for AUTOTURN\n");
	printf("         -O optimize adding, for bulk adds set this for all\n");
	printf("            except the last one\n");
#ifdef CLUSTERED_SITE
	printf("         -D database (adds a clustered domain, extra options apply as below)\n");
	printf("         -S SqlServer host/IP\n");
	printf("         -U Database User\n");
	printf("         -P Database Password\n");
	printf("         -p Database Port\n");
	printf("         -c Add domain as a clustered domain\n");
#endif
	return;
}

void
getversion_vadddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
