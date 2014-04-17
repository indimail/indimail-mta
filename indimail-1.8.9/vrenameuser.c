/*
 * $Log: vrenameuser.c,v $
 * Revision 2.12  2014-04-17 11:43:04+05:30  Cprogrammer
 * set supplementary group ids for indimail
 *
 * Revision 2.11  2011-11-09 19:46:32+05:30  Cprogrammer
 * removed getversion
 *
 * Revision 2.10  2011-07-29 09:26:38+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.9  2010-02-17 10:58:55+05:30  Cprogrammer
 * added post handle
 *
 * Revision 2.8  2009-09-30 00:24:23+05:30  Cprogrammer
 * added missing call to GetIndiId()
 *
 * Revision 2.7  2008-09-17 21:39:15+05:30  Cprogrammer
 * allow root and indimail to run vrenameuser
 *
 * Revision 2.6  2008-09-14 20:28:22+05:30  Cprogrammer
 * do setuid to indimail
 *
 * Revision 2.5  2008-09-12 22:28:18+05:30  Cprogrammer
 * removed not needed Setgid, Setuid calls
 *
 * Revision 2.4  2004-07-17 14:36:23+05:30  Cprogrammer
 * run with root privileges
 *
 * Revision 2.3  2004-07-03 23:55:57+05:30  Cprogrammer
 * check return status of parse_email
 *
 * Revision 2.2  2003-06-18 23:12:25+05:30  Cprogrammer
 * corrected incorrect return code
 *
 * Revision 2.1  2002-04-26 15:34:28+05:30  Cprogrammer
 * program to rename users
 *
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
static char     sccsid[] = "$Id: vrenameuser.c,v 2.12 2014-04-17 11:43:04+05:30 Cprogrammer Exp mbhangui $";
#endif

char            oldEmail[MAX_BUFF];
char            newEmail[MAX_BUFF];
char            oldUser[MAX_BUFF];
char            oldDomain[MAX_BUFF];
char            newUser[MAX_BUFF];
char            newDomain[MAX_BUFF];

void            usage();
int             get_options(int argc, char **argv);

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	uid_t           uid1, uid2, myuid;
	gid_t           gid1, gid2;
	char           *ptr, *base_argv0;

	if(get_options(argc, argv))
		return (1);
	if (parse_email(oldEmail, oldUser, oldDomain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", oldEmail);
		return (1);
	} else
	if (parse_email(newEmail, newUser, newDomain, MAX_BUFF))
	{
		error_stack(stderr, "%s: Email too long\n", newEmail);
		return (1);
	} 
	if(!vget_assign(oldDomain, 0, 0, &uid1, &gid1))
	{
		error_stack(stderr, "%s: No such domain\n", oldDomain);
		return (1);
	}
	if(!vget_assign(newDomain, 0, 0, &uid2, &gid2))
	{
		error_stack(stderr, "%s: No such domain\n", newDomain);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	myuid = getuid();
	if (myuid != 0 && myuid != indimailuid)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return (1);
	}
	if (uid1 != uid2)
	{
		if (setuid(0))
		{
			perror("setuid-root");
			return (1);
		}
	} else
	{
		if(indimailuid == -1 || indimailgid == -1)
			GetIndiId(&indimailuid, &indimailgid);
		myuid = geteuid();
		if (setuser_privileges(uid1, gid1, "indimail"))
		{
			error_stack(stderr, "setuser_privileges: (%d/%d): %s", uid1, gid1, strerror(errno));
			return (1);
		}
	}
	if (vrenameuser(oldUser, oldDomain, newUser, newDomain))
	{
		vclose();
		return (1);
	}
	vclose();
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return (post_handle("%s/libexec/%s %s %s", INDIMAILDIR, base_argv0, oldEmail, newEmail));
	} else
		return (post_handle("%s %s %s", ptr, oldEmail, newEmail));
}


void
usage()
{
	printf("usage: vrenameuser [options] old_email_address new_email_address\n");
	printf("options: -V (print version number)\n");
	printf("options: -v (verbose)\n");
}

int
get_options(int argc, char **argv)
{
	int             c;
	int             errflag;

	memset(oldEmail, 0, MAX_BUFF);
	memset(newEmail, 0, MAX_BUFF);
	memset(oldUser, 0, MAX_BUFF);
	memset(oldDomain, 0, MAX_BUFF);
	memset(newUser, 0, MAX_BUFF);
	memset(newDomain, 0, MAX_BUFF);
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
	{
		scopy(oldEmail, argv[optind++], MAX_BUFF);
		scopy(newEmail, argv[optind++], MAX_BUFF);
	}
	if (errflag || !*oldEmail || !*newEmail)
	{
		usage();
		return (1);
	}
	return (0);
}

void
getversion_vrenameuser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
