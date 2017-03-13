/*
 * $Log: vadddomain.c,v $
 * Revision 2.39  2017-03-13 14:11:19+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.38  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.37  2016-06-09 14:22:23+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.36  2016-05-25 09:06:24+05:30  Cprogrammer
 * use LIBEXECDIR for post handle
 *
 * Revision 2.35  2016-05-18 11:47:03+05:30  Cprogrammer
 * fixed comments
 *
 * Revision 2.34  2016-05-17 15:24:11+05:30  Cprogrammer
 * use domain directory set by configure
 *
 * Revision 2.33  2016-01-12 14:27:05+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.32  2011-11-09 19:45:44+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.31  2010-08-10 18:42:13+05:30  Cprogrammer
 * made dir control user level configurable
 *
 * Revision 2.30  2010-05-18 18:49:46+05:30  Cprogrammer
 * fix ownership of .base_path
 *
 * Revision 2.29  2010-05-17 10:14:15+05:30  Cprogrammer
 * create control file .base_path in domains directory
 *
 * Revision 2.28  2010-05-16 23:59:20+05:30  Cprogrammer
 * added -B option to specify BASE PATH
 *
 * Revision 2.27  2010-02-16 13:08:35+05:30  Cprogrammer
 * added post_handle function
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
#include <sys/socket.h>
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vadddomain.c,v 2.39 2017-03-13 14:11:19+05:30 Cprogrammer Exp mbhangui $";
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
int             get_options(int argc, char **argv, char **, int *, int *);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err, fd, chk_rcpt, i, users_per_level = 0;
	uid_t           uid;
	gid_t           gid;
	extern int      create_flag;
	char           *domaindir, *ptr, *base_argv0, *base_path;
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

	if (get_options(argc, argv, &base_path, &chk_rcpt, &users_per_level))
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
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid) != 1)
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
		getEnvConfigStr(&domaindir, "DOMAINDIR", DOMAINDIR);
		if (use_etrn)
		{
			if (!(ptr = vget_assign("autoturn", 0, 0, &uid, &gid)))
				snprintf(Dir, MAX_BUFF, "%s/autoturn", domaindir);
			else
			{
				Uid = uid;
				Gid = gid;
				scopy(Dir, ptr, MAX_BUFF);
			}
		} else
			scopy(Dir, domaindir, MAX_BUFF);
	}
	/*
	 * add domain to virtualdomains and optionally to chkrcptdomains
	 * add domain to users/assign, users/cdb
	 * create .qmail-default file
	 */
	if (base_path && !use_etrn)
	{
		if (access(base_path, F_OK) && r_mkdir(base_path, INDIMAIL_DIR_MODE, Uid, Gid))
		{
			error_stack(stderr, "%s: %s\n", base_path, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (setenv("BASE_PATH", base_path, 1))
		{
			error_stack(stderr, "setenv BASE_PATH=%s: %s\n", base_path, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
	}
	if ((err = vadddomain(Domain, ipaddr, Dir, Uid, Gid, chk_rcpt)) != VA_SUCCESS)
	{
		error_stack(stderr, 0);
		vclose();
		return(err);
	}
	if (users_per_level)
	{
		if (!vget_assign(Domain, Dir, MAX_BUFF, &uid, &gid))
		{
			error_stack(stderr, "Domain %s does not exist\n", Domain);
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/.users_per_level", Dir);
		if ((fd = open(TmpBuf, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1)
		{
			error_stack(stderr, "open: %s: %s\n", TmpBuf, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (filewrt(fd, "%d", users_per_level) == -1)
		{
			error_stack(stderr, "write: %s\n", strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (fchown(fd, uid, gid))
		{
			error_stack(stderr, "fchown: %s: (uid %d: gid %d): %s\n", TmpBuf, uid, gid, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		close(fd);
	}
	if (base_path && !use_etrn)
	{
		if (!vget_assign(Domain, Dir, MAX_BUFF, &uid, &gid))
		{
			error_stack(stderr, "Domain %s does not exist\n", Domain);
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/.base_path", Dir);
		if ((fd = open(TmpBuf, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR)) == -1)
		{
			error_stack(stderr, "open: %s: %s\n", TmpBuf, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (write(fd, base_path, strlen(base_path)) == -1)
		{
			error_stack(stderr, "write: %s\n", strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		if (fchown(fd, uid, gid))
		{
			error_stack(stderr, "fchown: %s: (uid %d: gid %d): %s\n", TmpBuf, uid, gid, strerror(errno));
			vdeldomain(Domain);
			vclose();
			return(1);
		}
		close(fd);
	}
#ifdef CLUSTERED_SITE
	/*
	 * add domain to dbinfo
	 */
	if (distributed >= 0)
	{
		if (!(localIP = get_local_ip(PF_INET)))
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
				fprintf(fs, "| %s/sbin/vdelivermail '' %s\n", PREFIX, BounceEmail);
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
	if ((err = vadduser("postmaster", Domain, 0, Passwd, "Postmaster", 0, users_per_level,
		Apop, 1)) != VA_SUCCESS)
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
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_handle("%s/%s %s", LIBEXECDIR, base_argv0, Domain));
	} else
		return(post_handle("%s %s", ptr, Domain));
}

int
get_options(int argc, char **argv, char **base_path, int *chk_rcpt, int *users_per_level)
{
	int             c;
	int             errflag;
	struct passwd  *mypw;
	char            optbuf[MAX_BUFF];

	*Domain = *Passwd = *Quota = *Dir = *BounceEmail = *TmpBuf = 0;
	*base_path = 0;
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
	scopy(optbuf, "atT:q:l:bB:e:u:vCci:g:d:D:S:U:P:p:O", MAX_BUFF);
#else
	scopy(optbuf, "atT:q:l:bB:e:u:vCi:g:d:O", MAX_BUFF);
#endif
#ifdef VFILTER
	scat(optbuf, "f", MAX_BUFF);
#endif
	while (!errflag && (c = getopt(argc, argv, optbuf)) != -1)
	{
		switch (c)
		{
		case 'B':
			if (use_etrn)
				error_stack(stderr, "option -B not valid for ETRN/ATRN/AUTORUN\n");
			else
				*base_path = optarg;
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
					return (1);
				}
			}
			break;
		case 'q':
			scopy(Quota, optarg, MAX_BUFF);
			break;
		case 'l':
			*users_per_level = atoi(optarg);
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
			usage();
			return (1);
		}
	}
	if (optind < argc)
		scopy(Domain, argv[optind++], MAX_BUFF);
	if (optind < argc)
		scopy(Passwd, argv[optind++], MAX_BUFF);
	if (!*Domain)
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
	error_stack(stderr, "usage: vaddomain [options] virtual_domain [postmaster password]\n");
	error_stack(stderr, "options: -V print version number\n");
	error_stack(stderr, "         -v verbose\n");
	error_stack(stderr, "         -q quota_in_bytes (sets the quota for postmaster account)\n");
	error_stack(stderr, "         -l level (users per level)\n");
	error_stack(stderr, "         -C (Do recipient check for this domain)\n");
	error_stack(stderr, "         -b (bounces all mail that doesn't match a user, default)\n");
	error_stack(stderr, "         -e [email_address|maildir] (forwards all non matching user to this address [*])\n");
	error_stack(stderr, "         -u user (sets the uid/gid based on a user in /etc/passwd)\n");
	error_stack(stderr, "         -B basepath Specify the base directory for postmaster's home directory\n");
	error_stack(stderr, "         -d dir (sets the dir to use for this domain)\n");
	error_stack(stderr, "         -i uid (sets the uid to use for this domain)\n");
	error_stack(stderr, "         -g gid (sets the gid to use for this domain)\n");
	error_stack(stderr, "         -a sets the account to use APOP, default is POP\n");
	error_stack(stderr, "         -f Sets the Domain with VFILTER capability\n");
	error_stack(stderr, "         -t Sets the Domain for ETRN/ATRN\n");
	error_stack(stderr, "         -T ip_address Sets the Domain for AUTOTURN\n");
	error_stack(stderr, "         -O optimize adding, for bulk adds set this for all\n");
	error_stack(stderr, "            except the last one\n");
#ifdef CLUSTERED_SITE
	error_stack(stderr, "         -D database (adds a clustered domain, extra options apply as below)\n");
	error_stack(stderr, "         -S SqlServer host/IP\n");
	error_stack(stderr, "         -U Database User\n");
	error_stack(stderr, "         -P Database Password\n");
	error_stack(stderr, "         -p Database Port\n");
	error_stack(stderr, "         -c Add domain as a clustered domain\n");
#endif
	return;
}

void
getversion_vadddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
