/*
 * $Log: mgmtpass.c,v $
 * Revision 2.9  2008-09-11 23:01:16+05:30  Cprogrammer
 * BUG - Null user was being accepted
 *
 * Revision 2.8  2008-05-21 15:51:26+05:30  Cprogrammer
 * added usage to mgmtpass
 *
 * Revision 2.7  2003-10-27 22:38:26+05:30  Cprogrammer
 * do not autocreate admin user if the user being created is admin
 *
 * Revision 2.6  2003-09-16 12:30:38+05:30  Cprogrammer
 * added option to list admin users
 *
 * Revision 2.5  2003-03-24 19:23:37+05:30  Cprogrammer
 * allow root to set passwords
 *
 * Revision 2.4  2002-12-04 02:06:06+05:30  Cprogrammer
 * create 'admin' user if not exists
 *
 * Revision 2.3  2002-10-20 22:15:01+05:30  Cprogrammer
 * compilation warning corrected for Solaris
 *
 * Revision 2.2  2002-08-03 00:37:15+05:30  Cprogrammer
 * added addition of users
 *
 * Revision 2.1  2002-07-23 00:26:27+05:30  Cprogrammer
 * password program for mgmt access
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: mgmtpass.c,v 2.9 2008-09-11 23:01:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#ifdef CLUSTERED_SITE
#include <string.h>
#include <unistd.h>
#ifdef sun
#include <stdlib.h>
#endif

void            usage(char *);

int
main(int argc, char **argv)
{
	int             idx;
	time_t          tmval;
	char           *user, *pass, *ptr;

	if((ptr = strrchr(argv[0], '/')))
		ptr++;
	else
		ptr = argv[0];
	for (user = pass = 0, idx = 1; idx < argc; idx++)
	{
		if (argv[idx][0] != '-')
			continue;
		switch (argv[idx][1])
		{
		case 'u':
			user = *(argv + idx + 1);
			break;
		case 'l':
			return(mgmtlist());
		case 'p':
			if (!user || !*user)
			{
				fprintf(stderr, "user not specified\n");
				return(1);
			}
			if(getuid() || geteuid())
			{
				fprintf(stderr, "%s: Only superuser can specify -p option\n", ptr);
				return(1);
			}
			if(*(argv + idx + 1))
			{
				tmval = time(0);
				return(mgmtsetpass(user, *(argv + idx + 1), getuid(), getgid(), tmval, tmval));
			} 
			break;
		case 'i':
			if (!user || !*user)
			{
				fprintf(stderr, "user not specified\n");
				return(1);
			}
			if(getuid() && geteuid())
			{
				fprintf(stderr, "%s: Only superuser can specify -i option\n", ptr);
				return(1);
			} else
				return(mgmtpassinfo(user, 1));
			break;
		case 'a':
			if (!user || !*user)
			{
				fprintf(stderr, "user not specified\n");
				return(1);
			}
			if(strncmp(user, "admin", 6) && mgmtpassinfo("admin", 0) && userNotFound)
			{
				if(!(ptr = (char *) getpass("New Admin password: ")))
					return(1);
				tmval = time(0);
				fprintf(stderr, "Creating user admin\n");
				if(mgmtadduser("admin", ptr, getuid(), getgid(), tmval, tmval))
					return(1);
			}
			if((getuid() || geteuid()) && getpassword("admin"))
				return(1);
			if(!mgmtpassinfo(user, 0))
			{
				fprintf(stderr, "User %s exists\n", user);
				return(1);
			}
			pass = *(argv + idx + 1);
			if(!pass && !(pass = (char *) getpass("New password: ")))
				return(1);
			tmval = time(0);
			return(mgmtadduser(user, pass, getuid(), getgid(), tmval, tmval));
			break;
		default:
			printf("USAGE: %s -u user [-a passwd | -p passwd] [-i]\n", ptr);
			printf("USAGE: %s -l (list users)]\n", ptr);
			usage(ptr);
			return(1);
		}
	}
	if(!user)
	{
		printf("USAGE: %s -u user [-a passwd | -p passwd] [-i]\n", ptr);
		printf("USAGE: %s -l (list users)]\n", ptr);
		usage(ptr);
		return(1);
	}
	return(setpassword(user));
}

void
usage(char *ptr)
{
	printf("options: -u user   (specify username)\n");
	printf("         -a passwd (adds user with specified password)\n");
	printf("         -p passwd (resets/changes password of existing user)\n");
	printf("         -i        (displays user information/stats)\n");
	printf("         -l        (lists admin users)\n");
}
#else
int
main()
{
	fprintf(stderr, "IndiMail not configured with --enable-user-cluster=y\n");
	return(1);
}
#endif

void
getversion_mgmtpass_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
