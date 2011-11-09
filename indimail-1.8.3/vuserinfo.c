/*
 * $Log: vuserinfo.c,v $
 * Revision 2.7  2011-11-09 19:46:44+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.6  2008-05-21 15:53:13+05:30  Cprogrammer
 * fixed usage display
 *
 * Revision 2.5  2004-07-03 23:56:31+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.4  2003-10-23 13:50:56+05:30  Cprogrammer
 * set DisplayAll to one if explicitely specified
 *
 * Revision 2.3  2003-01-26 02:16:24+05:30  Cprogrammer
 * corrected returning correct error status
 *
 * Revision 2.2  2002-10-11 20:09:05+05:30  Cprogrammer
 * added option to display filters
 *
 * Revision 2.1  2002-04-15 19:50:23+05:30  Cprogrammer
 * incorrect "message for displaying times"
 *
 * Revision 1.10  2002-03-03 13:17:55+05:30  Cprogrammer
 * option 'l' was not working
 *
 * Revision 1.9  2002-02-24 22:47:20+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.8  2001-12-27 01:31:44+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.7  2001-12-22 21:03:44+05:30  Cprogrammer
 * option to display mail summary added
 *
 * Revision 1.6  2001-12-08 17:46:31+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.5  2001-11-29 14:35:17+05:30  Cprogrammer
 * added vclose()
 *
 * Revision 1.4  2001-11-28 23:13:02+05:30  Cprogrammer
 * Removed printing of VERSION
 *
 * Revision 1.3  2001-11-24 12:22:39+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:02:24+05:30  Cprogrammer
 * added getversion_vuserinfo_c
 *
 * Revision 1.1  2001-10-24 18:15:48+05:30  Cprogrammer
 * Initial revision
 *
 * vuserinfo
 *
 * prints user information from the authentication files
 *
 * part of the indimail package
 *
 * Copyright (C) 2000 Inter7 Internet Technologies, Inc.
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
#ifdef ENABLE_AUTH_LOGGING
#include <time.h>
#endif

#ifndef	lint
static char     sccsid[] = "$Id: vuserinfo.c,v 2.7 2011-11-09 19:46:44+05:30 Cprogrammer Stab mbhangui $";
#endif

void            usage();

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             c, errflag, version_flag;
	char            Email[MAX_BUFF], User[MAX_BUFF], Domain[MAX_BUFF], opt_str[56];
	int             DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, DisplayQuota;
	int             DisplayLastAuth, DisplayAll, DisplayMail, DisplayFilter, tmpAll;

	DisplayName = DisplayPasswd = DisplayUid = DisplayGid = DisplayComment = DisplayDir = 0;
	DisplayQuota = DisplayLastAuth = DisplayMail = DisplayFilter = 0;
	DisplayAll = 1;
	tmpAll = 0;
	memset(User, 0, MAX_BUFF);
	memset(Email, 0, MAX_BUFF);
	memset(Domain, 0, MAX_BUFF);
	version_flag = errflag = 0;
	snprintf(opt_str, sizeof(opt_str), "anpugcdqm");
#ifdef ENABLE_AUTH_LOGGING
	strncat(opt_str, "l", 1);
#endif
#ifdef VFILTER
	strncat(opt_str, "f", 1);
#endif
	while (!errflag && (c = getopt(argc, argv, opt_str)) != -1)
	{
		switch (c)
		{
		case 'a':
			tmpAll = 1;
			break;
		case 'n':
			DisplayName = 1;
			DisplayAll = 0;
			break;
		case 'p':
			DisplayPasswd = 1;
			DisplayAll = 0;
			break;
		case 'u':
			DisplayUid = 1;
			DisplayAll = 0;
			break;
		case 'g':
			DisplayGid = 1;
			DisplayAll = 0;
			break;
		case 'c':
			DisplayComment = 1;
			DisplayAll = 0;
			break;
		case 'd':
			DisplayDir = 1;
			DisplayAll = 0;
			break;
		case 'q':
			DisplayQuota = 1;
			DisplayAll = 0;
			break;
#ifdef ENABLE_AUTH_LOGGING
		case 'l':
			DisplayLastAuth = 1;
			DisplayAll = 0;
			break;
#endif
		case 'm':
			DisplayMail = 1;
			DisplayAll = 0;
			break;
		case 'f':
			DisplayFilter = 1;
			DisplayAll = 0;
			break;
		default:
			errflag = 1;
			break;
		}
	}
	if (tmpAll)
		DisplayAll = 1;
	if (errflag > 0)
	{
		usage();
		return(1);
	}
	if (optind < argc)
		scopy(Email, argv[optind++], MAX_BUFF);
	if (!*Email && !version_flag)
	{
		usage();
		return(1);
	}
	if(*Email)
	{
		if (parse_email(Email, User, Domain, MAX_BUFF))
		{
			fprintf(stderr, "%s: Email too long\n", Email);
			return(1);
		}
		errflag = vuserinfo(Email, User, Domain, DisplayName, DisplayPasswd, DisplayUid, DisplayGid, DisplayComment, DisplayDir, 
			DisplayQuota, DisplayLastAuth, DisplayMail, DisplayFilter, DisplayAll);
		vclose();
		return(errflag);
	}
	return(1);
}

void
usage()
{
	printf("usage: vuserinfo [options] email_address\n");
	printf("options: -V (print version number)\n");
	printf("         -a (display all fields, this is the default)\n");
	printf("         -n (display name)\n");
	printf("         -p (display crypted password)\n");
	printf("         -u (display uid field)\n");
	printf("         -g (display gid field)\n");
	printf("         -c (display comment field)\n");
	printf("         -d (display directory)\n");
	printf("         -q (display quota field)\n");
#ifdef ENABLE_AUTH_LOGGING
	printf("         -l (display usage times)\n");
#endif
	printf("         -m (display Mail Summary)\n");
}

void
getversion_vuserinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
