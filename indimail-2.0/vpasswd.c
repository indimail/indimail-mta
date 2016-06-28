/*
 * $Log: vpasswd.c,v $
 * Revision 2.15  2016-05-25 09:09:31+05:30  Cprogrammer
 * use LIBEXECDIR for post handle
 *
 * Revision 2.14  2011-11-09 19:46:27+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.13  2010-02-17 14:14:29+05:30  Cprogrammer
 * added post handle
 *
 * Revision 2.12  2008-08-02 09:10:37+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.11  2008-05-28 15:34:15+05:30  Cprogrammer
 * removed ldap code
 *
 * Revision 2.10  2008-05-21 15:49:23+05:30  Cprogrammer
 * corrections for Non-Ldap use
 *
 * Revision 2.9  2004-10-27 14:52:52+05:30  Cprogrammer
 * call vclose for both CLUSTERED_SITE and non-CLUSTERED_SITE
 *
 * Revision 2.8  2004-07-03 23:55:48+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.7  2004-06-24 20:05:12+05:30  Cprogrammer
 * handle condition when domain is null
 *
 * Revision 2.6  2003-10-23 13:23:37+05:30  Cprogrammer
 * added option to generate random password
 * print errors to stderr
 *
 * Revision 2.5  2003-08-25 17:16:11+05:30  Cprogrammer
 * bug fix in getting passwd
 *
 * Revision 2.4  2003-08-24 22:03:06+05:30  Cprogrammer
 * added option to set passwd in LDAP
 *
 * Revision 2.3  2003-02-27 23:57:11+05:30  Cprogrammer
 * corrected usage
 * corrected incorrect return values
 *
 * Revision 2.2  2002-12-05 00:36:21+05:30  Cprogrammer
 * corrected exit status
 *
 * Revision 2.1  2002-08-28 20:36:38+05:30  Cprogrammer
 * close_db() to be called for a distributed domain
 *
 * Revision 1.9  2002-02-24 22:47:06+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.8  2001-12-27 01:31:07+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.7  2001-12-08 17:46:09+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.6  2001-11-28 23:11:23+05:30  Cprogrammer
 * removed findhost() call
 *
 * Revision 1.5  2001-11-24 12:22:12+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.4  2001-11-23 00:15:58+05:30  Cprogrammer
 * return from code when real_domain is null
 *
 * Revision 1.3  2001-11-20 11:01:23+05:30  Cprogrammer
 * change for inactive user updation
 *
 * Revision 1.2  2001-11-14 19:29:07+05:30  Cprogrammer
 * distributed arch change
 *
 * Revision 1.1  2001-10-24 18:15:43+05:30  Cprogrammer
 * Initial revision
 *
 * vpasswd
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
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vpasswd.c,v 2.15 2016-05-25 09:09:31+05:30 Cprogrammer Exp mbhangui $";
#endif


extern int      encrypt_flag;

char            Email[MAX_BUFF];
char            User[MAX_BUFF];
char            Domain[MAX_BUFF];
char            Passwd[MAX_BUFF];
char            TmpBuf1[MAX_BUFF];
int             apop;

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             i;
	char           *real_domain, *ptr, *base_argv0;

	if (get_options(argc, argv))
		return(1);
	if (parse_email(Email, User, Domain, MAX_BUFF))
	{
		fprintf(stderr, "%s: Email too long\n", Email);
		return (1);
	}
	if (!*Domain)
	{
		fprintf(stderr, "%s: No domain specified\n", User);
		return(0);
	}
	real_domain = (char *) 0;
	if (!(real_domain = vget_real_domain(Domain)))
	{
		fprintf(stderr, "%s: No such domain\n", Domain);
		return (1);
	}
	if ((i = vpasswd(User, real_domain, Passwd, apop)) != 1)
	{
		if (!i)
			error_stack(stderr, "%s@%s: No such user\n", User, real_domain);
		vclose();
		if (i == -1)
			return(-1);
		return(1);
	}
	vclose();
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return (post_handle("%s/%s %s@%s", LIBEXECDIR, base_argv0, User, real_domain));
	} else
		return (post_handle("%s %s@%s", ptr, User, real_domain));
}

void
usage()
{
	fprintf(stderr, "usage: vpasswd [options] email_address [password]\n");
	fprintf(stderr, "options: -V (print version number)\n");
	fprintf(stderr, "         -v (verbose)\n");
	fprintf(stderr, "         -a (use apop, pop is default)\n");
	fprintf(stderr, "         -e encrypted password (set the encrypted password field)\n");
	fprintf(stderr, "         -r Generate a random password\n");
}

int
get_options(int argc, char **argv)
{
	int             c, Random;
	int             errflag;
	char           *ptr;

	memset(Email, 0, MAX_BUFF);
	memset(Passwd, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	memset(TmpBuf1, 0, MAX_BUFF);
	apop = USE_POP;
	Random = errflag = 0;
	while (!errflag && (c = getopt(argc, argv, "vrae")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
		case 'a':
			apop = USE_APOP;
			break;
		case 'r':
			Random = 1;
			break;
		case 'e':
			encrypt_flag = 1;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (optind < argc)
		scopy(Email, argv[optind++], MAX_BUFF);
	if (*Email)
	{
		if (optind < argc)
			scopy(Passwd, argv[optind++], MAX_BUFF);
		else
		{
			if (Random)
				ptr = genpass(8);
			else
			if (!(ptr = vgetpasswd(Email)))
			{
				usage();
				return(1);
			}
			scopy(Passwd, ptr, MAX_BUFF);
		}
	}
	if (errflag || !*Email || !*Passwd)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vpasswd_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
