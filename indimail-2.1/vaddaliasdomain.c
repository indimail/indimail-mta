/*
 * $Log: vaddaliasdomain.c,v $
 * Revision 2.7  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.6  2016-06-09 14:21:41+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.5  2011-11-09 19:45:41+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.4  2009-01-28 18:46:58+05:30  Cprogrammer
 * run as root for updation of qmail's assign file
 *
 * Revision 2.3  2008-09-17 21:32:33+05:30  Cprogrammer
 * setuid to domain uid
 *
 * Revision 2.2  2008-08-02 09:09:02+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.1  2003-01-26 02:14:28+05:30  Cprogrammer
 * setuid() to allow updation of assign file
 *
 * Revision 1.7  2002-02-24 22:46:18+05:30  Cprogrammer
 * added getversion
 *
 * Revision 1.6  2001-12-27 01:28:04+05:30  Cprogrammer
 * version and verbose switch update
 *
 * Revision 1.5  2001-12-08 17:45:03+05:30  Cprogrammer
 * usage message change
 *
 * Revision 1.4  2001-11-28 23:01:43+05:30  Cprogrammer
 * getversion() function call added
 *
 * Revision 1.3  2001-11-24 12:20:24+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:19+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:16+05:30  Cprogrammer
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: vaddaliasdomain.c,v 2.7 2016-06-09 15:32:32+05:30 Cprogrammer Stab mbhangui $";
#endif

char            Domain_old[MAX_BUFF], Domain_new[MAX_BUFF];

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	int             err;
	uid_t           uid;
	gid_t           gid;

	if(get_options(argc, argv))
		return(1);
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid) != 1)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return(1);
	}
	if (setuid(0))
	{
		error_stack(stderr, "setuid: %s", strerror(errno));
		return(1);
	}
	if((err = vaddaliasdomain(Domain_old, Domain_new)) != VA_SUCCESS)
		error_stack(stderr, 0);
	vclose();
	return(err);
}


void
usage()
{
	printf("usage: vaddaliasdomain [options] new_domain old_domain\n");
	printf("options: -V (print version number)\n");
	printf("options: -v (verbose)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	*Domain_old = *Domain_new = 0;
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
		scopy(Domain_new, argv[optind++], MAX_BUFF);
	if (optind < argc)
		scopy(Domain_old, argv[optind++], MAX_BUFF);
	if (errflag || !*Domain_new || !*Domain_old)
	{
		usage();
		return(1);
	}
	return(0);
}

void
getversion_vaddaliasdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}

