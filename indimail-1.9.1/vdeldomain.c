/*
 * $Log: vdeldomain.c,v $
 * Revision 2.15  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.14  2016-01-12 14:27:11+05:30  Cprogrammer
 * use AF_INET for get_local_ip()
 *
 * Revision 2.13  2013-08-03 20:22:33+05:30  Cprogrammer
 * check for host.master to check if domain is distributed
 *
 * Revision 2.12  2011-11-09 19:45:57+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.11  2010-02-16 13:08:55+05:30  Cprogrammer
 * added post_handle function
 *
 * Revision 2.10  2008-09-17 21:35:24+05:30  Cprogrammer
 * setuid(0) only for indimail
 *
 * Revision 2.9  2008-08-02 09:10:05+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.8  2005-12-29 22:52:15+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.7  2004-01-03 15:54:45+05:30  Cprogrammer
 * check existense of mcdfile before trying unlink
 *
 * Revision 2.6  2003-01-28 23:29:17+05:30  Cprogrammer
 * added option -c to remove entry from dbinfo
 *
 * Revision 2.5  2003-01-26 02:15:50+05:30  Cprogrammer
 * setuid() to allow updation of assign file
 *
 * Revision 2.4  2002-12-29 02:50:27+05:30  Cprogrammer
 * use dbinfDel() only if domain is distributed
 *
 * Revision 2.3  2002-12-08 20:08:42+05:30  Cprogrammer
 * delete from dbinfo when domain is deleted
 *
 * Revision 2.2  2002-08-30 00:13:02+05:30  Cprogrammer
 * Usage for AUTOTURN displayed before verbose option
 *
 * Revision 2.1  2002-08-25 22:35:42+05:30  Cprogrammer
 * option to remove autoturn domains
 *
 * Revision 1.8  2002-02-24 22:47:34+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.7  2001-12-27 01:29:36+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.6  2001-12-08 17:45:34+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.5  2001-11-29 13:21:06+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.4  2001-11-28 23:08:37+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:21:44+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:14+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:33+05:30  Cprogrammer
 * Initial revision
 *
 * vdeldomain
 * part of the indimail package
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
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <pwd.h>
#include <sys/socket.h>

#ifndef	lint
static char     sccsid[] = "$Id: vdeldomain.c,v 2.15 2016-05-17 17:09:39+05:30 Cprogrammer Exp mbhangui $";
#endif

char            Domain[MAX_BUFF];
int             mcd_remove = 0;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err;
	uid_t           uid;
	char           *ptr, *base_argv0;
#ifdef CLUSTERED_SITE
	char            mcdFile[MAX_BUFF];
	char           *ipaddr, *mcdfile, *qmaildir, *controldir;
	int             total, is_dist;
#endif

	if (get_options(argc, argv))
		return(0);
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	if (uid != 0 && uid != indimailuid)
	{
		fprintf(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if (uid && setuid(0))
	{
		fprintf(stderr, "setuid: %s\n", strerror(errno));
		return(1);
	}
	if ((err = vdeldomain(Domain)) != VA_SUCCESS)
		error_stack(stderr, 0);
#ifdef CLUSTERED_SITE
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(mcdFile, MAX_BUFF, "%s/host.master", controldir);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s/host.master", qmaildir, controldir);
	if (access(mcdFile, F_OK))
		err = 1;
	if (!err)
	{
		if (is_alias_domain(Domain) == 1)
		{
			vclose();
			return(err);
		}
		if ((is_dist = is_distributed_domain(Domain)) == -1)
		{
			fprintf(stderr, "Unable to verify %s as a distributed domain\n", Domain);
			vclose();
			return(1);
		} else
		if (is_dist || mcd_remove)
		{
			if (!(ipaddr = get_local_ip(PF_INET)))
			{
				fprintf(stderr, "vdeldomain: failed to get local ipaddr\n");
				vclose();
				return(1);
			}
			if (dbinfoDel(Domain, ipaddr))
			{
				fprintf(stderr, "vdeldomain: failed to get remove dbinfo entry for %s@%s\n", Domain, ipaddr);
				vclose();
				return(1);
			}
			getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
			if (*mcdfile == '/')
				scopy(mcdFile, mcdfile, MAX_BUFF);
			else {
				if (*controldir == '/')
					snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
				else
					snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
			}
			if (!access(mcdfile, F_OK) && unlink(mcdFile))
			{
				fprintf(stderr, "vdeldomain: unlink: %s: %s\n", mcdFile, strerror(errno));
				vclose();
				return(1);
			}
			LoadDbInfo_TXT(&total);
		}
	}
#endif
	vclose();
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_handle("%s/libexec/%s %s", INDIMAILDIR, base_argv0, Domain));
	} else
		return(post_handle("%s %s", ptr, Domain));
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	memset(Domain, 0, MAX_BUFF);
	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "cTv")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'T':
			use_etrn = 2;
			break;
		case 'c':
			mcd_remove = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
	{
		scopy(Domain, argv[optind], MAX_BUFF);
		++optind;
	}
	if (errflag || !*Domain)
	{
		usage();
		return(1);
	}
	return(0);
}

void
usage()
{
	printf("usage: vdeldomain [options] domain_name\n");
	printf("options: -c (Remove MCD Information)\n");
	printf("options: -T (Remove Autoturn Domain)\n");
	printf("options: -V (print version number)\n");
	printf("options: -v (verbose)\n");
}

void
getversion_vdeldomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
