/*
 * $Log: vdeluser.c,v $
 * Revision 2.7  2011-11-09 19:46:02+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.6  2010-02-16 13:09:02+05:30  Cprogrammer
 * added post_handle function
 *
 * Revision 2.5  2009-12-30 13:14:07+05:30  Cprogrammer
 * run vdeluser with uid, gid of domain
 *
 * Revision 2.4  2008-09-17 21:36:59+05:30  Cprogrammer
 * allow only root or indimail to run vdeluser
 *
 * Revision 2.3  2008-08-02 09:10:08+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2004-07-03 23:55:14+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.1  2002-04-28 18:33:46+05:30  Cprogrammer
 * removed unused variable TmpBuf1
 *
 * Revision 1.8  2002-02-24 22:46:49+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.7  2001-12-27 01:30:06+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.6  2001-12-08 17:45:46+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.5  2001-11-29 13:21:16+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.4  2001-11-28 23:09:57+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:21:49+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:38+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:36+05:30  Cprogrammer
 * Initial revision
 *
 * vdeluser
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
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vdeluser.c,v 2.7 2011-11-09 19:46:02+05:30 Cprogrammer Stab mbhangui $";
#endif

char            Email[MAX_BUFF];
char            User[MAX_BUFF];
char            Domain[MAX_BUFF];

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err;
	char           *ptr, *base_argv0;
	uid_t           uid, uidtmp;
	gid_t           gid;

	if (get_options(argc, argv))
		return(1);
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", Email);
		return(1);
	}
	if (!vget_assign(Domain, 0, 0, &uid, &gid))
	{
		error_stack(stderr, "%s: No such domain\n", Domain);
		return (-1);
	}
	uidtmp = getuid();
	if (uidtmp != 0 && uidtmp != uid)
	{
		error_stack(stderr, "you must be root or domain user (uid=%d) to run this program\n", uid);
		return(1);
	}
	if (setgid(gid) || setuid(uid))
	{
		error_stack(stderr, "setuid/setgid (%d/%d): %s", uid, gid, strerror(errno));
		return(1);
	}
	if ((err = vdeluser(User, Domain, 1)) != VA_SUCCESS)
	{
		error_stack(stderr, 0);
		vclose();
		return(err);
	}
	vclose();
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_handle("%s/libexec/%s %s@%s", INDIMAILDIR, base_argv0, User, Domain));
	} else
		return(post_handle("%s %s@%s", ptr, User, Domain));
}


void
usage()
{
	printf("usage: vdeluser [options] email_address\n");
	printf("options: -V (print version number)\n");
	printf("options: -v (verbose)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	memset(Email, 0, MAX_BUFF);
	memset(User, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
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
	if (errflag || !*Email)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vdeluser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
