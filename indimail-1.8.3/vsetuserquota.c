/*
 * $Log: vsetuserquota.c,v $
 * Revision 2.7  2011-11-09 19:46:37+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.6  2010-05-01 14:15:31+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.5  2009-12-02 11:05:12+05:30  Cprogrammer
 * use .domain_limits in domain directory to turn on domain limits
 *
 * Revision 2.4  2009-12-01 16:29:21+05:30  Cprogrammer
 * added checking of domain limit for user quota
 *
 * Revision 2.3  2009-09-30 00:24:38+05:30  Cprogrammer
 * do setuid to make quota operations succeed
 *
 * Revision 2.2  2008-08-02 09:10:46+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.1  2004-07-03 23:56:06+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 1.10  2002-02-24 22:47:11+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.9  2001-12-27 01:31:34+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.8  2001-12-19 20:40:22+05:30  Cprogrammer
 * err message if user does not exist
 *
 * Revision 1.7  2001-12-08 17:46:26+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.6  2001-11-29 21:00:28+05:30  Cprogrammer
 * conditional compilation of distributed arch
 *
 * Revision 1.5  2001-11-28 23:12:43+05:30  Cprogrammer
 * code for distributed architecture added
 *
 * Revision 1.4  2001-11-24 12:22:31+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.3  2001-11-20 11:02:17+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.2  2001-11-14 19:29:43+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:46+05:30  Cprogrammer
 * Initial revision
 *
 * vsetuserquota
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vsetuserquota.c,v 2.7 2011-11-09 19:46:37+05:30 Cprogrammer Stab mbhangui $";
#endif

char            Email[MAX_BUFF];
char            User[MAX_BUFF];
char            Domain[MAX_BUFF];
char            Quota[MAX_BUFF];
char            TmpBuf1[MAX_BUFF];

int             get_options(int argc, char **argv);
void            usage();

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i;
	uid_t           uid;
	char           *real_domain;
	struct passwd  *pw;
#ifdef ENABLE_DOMAIN_LIMITS
	char            tmpbuf[MAX_BUFF], TheDir[MAX_BUFF];
	int             domain_limits;
	struct vlimits  limits;
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
	if (uid && setuid(0))
	{
		error_stack(stderr, "setuid-root: %s\n", strerror(errno));
		return(1);
	}
	for (i = 1; i < argc; ++i)
	{
		if (Email[0] == 0)
			scopy(Email, argv[i], MAX_BUFF);
		else
			scopy(Quota, argv[i], MAX_BUFF);
	}
	lowerit(Email);
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", Email);
		return(1);
	}
	if (!(real_domain = vget_real_domain(Domain)))
	{
		error_stack(stderr, "%s: No such domain\n", Domain);
		return(1);
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
	if (vauthOpen_user(Email, 0))
#else
	if (vauth_open((char *) 0))
#endif
	{
		if (userNotFound)
		{
			error_stack(stderr, "%s: No such user\n", Email);
			return (1);
		}
		error_stack(stderr, "Temporary database Errror\n");
		return(1);
	}
	if (!(pw = vauth_getpw(User, Domain)))
	{
		error_stack(stderr, "%s: No such user\n", Email);
		vclose();
		return (1);
	}
#ifdef ENABLE_DOMAIN_LIMITS
	if (!(pw->pw_gid & V_OVERRIDE) && domain_limits)
	{
		if (vget_limits(real_domain, &limits))
		{
			error_stack(stderr, "Unable to get domain limits for %s\n", real_domain);
			vclose();
			return(1);
		}
		if (limits.perm_defaultquota & VLIMIT_DISABLE_MODIFY)
		{
			error_stack(stderr, "quota modification not allowed for %s\n", Email);
			vclose();
			return(1);
		}
	}
#endif
	if ((i = vsetuserquota(User, real_domain, Quota)))
		error_stack(stderr, 0);
	vclose();
	return(0);
}

void
usage()
{
	printf("usage: vsetuserquota [options] email_address quota_in_bytes\n");
	printf("options:\n");
	printf("-V (print version number)\n");
	printf("-v (verbose)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	memset(Email, 0, MAX_BUFF);
	memset(Quota, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(TmpBuf1, 0, MAX_BUFF);
	errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "v")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
		scopy(Email, argv[optind++], MAX_BUFF);
	if (optind < argc)
		scopy(Quota, argv[optind++], MAX_BUFF);
	if (errflag || !*Email || !*Quota)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vsetuserquota_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
