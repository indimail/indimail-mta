/*
 * $Log: vreorg.c,v $
 * Revision 2.13  2010-08-08 13:03:35+05:30  Cprogrammer
 * made users_per_level configurable
 *
 * Revision 2.12  2009-12-30 13:16:15+05:30  Cprogrammer
 * run vreorg with uid of domain if uid does not match domain uid
 *
 * Revision 2.11  2009-02-18 21:36:08+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.10  2009-02-06 11:41:15+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.9  2008-09-14 19:51:56+05:30  Cprogrammer
 * do setuid / setgid only if not running with indimail perms
 *
 * Revision 2.8  2008-06-13 11:00:02+05:30  Cprogrammer
 * compile renaming of valias entries only if VALIAS defined
 *
 * Revision 2.7  2008-05-28 17:42:32+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.6  2008-05-28 15:34:21+05:30  Cprogrammer
 * removed ldap code
 *
 * Revision 2.5  2003-05-30 00:01:57+05:30  Cprogrammer
 * bypass ldap to prevent wrong setting of pw_gid
 *
 * Revision 2.4  2002-11-18 12:45:36+05:30  Cprogrammer
 * added option to turn on/off decrementing of dir_control
 *
 * Revision 2.3  2002-10-25 17:40:13+05:30  Cprogrammer
 * added option to reset dir control
 *
 * Revision 2.2  2002-07-03 01:29:27+05:30  Cprogrammer
 * copy passwd structure to prevent overwriting static location returned by vauth_getpw()
 * error check for malloc() added
 *
 * Revision 2.1  2002-05-10 00:57:42+05:30  Cprogrammer
 * changed rename() to MoveFile()
 *
 * Revision 1.7  2001-12-09 23:57:28+05:30  Cprogrammer
 * old alias line passed to valias_update()
 *
 * Revision 1.6  2001-12-02 20:24:22+05:30  Cprogrammer
 * used valias_delete, valias_insert for valias_update when cdb is used
 *
 * Revision 1.5  2001-12-02 18:52:42+05:30  Cprogrammer
 * change du to vdel_dir_control() function call changes
 *
 * Revision 1.4  2001-12-01 23:12:23+05:30  Cprogrammer
 * Replace all occurence of path in valias with the new path
 *
 * Revision 1.3  2001-11-24 12:22:26+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:02:12+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:44+05:30  Cprogrammer
 * Initial revision
 *
 * vreorg 
 *
 * re-organizes the user directory layout for optimal efficency 
 *
 * part of the indimail package
 *
 * Copyright (C) 2001 Inter7 Internet Technologies, Inc.
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
#include <pwd.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: vreorg.c,v 2.13 2010-08-08 13:03:35+05:30 Cprogrammer Stab mbhangui $";
#endif

int             get_options(int argc, char **argv, char *, char *, int *, int *, int *);

int
main(int argc, char **argv)
{
	FILE           *fs;
	struct passwd  *pw;
	struct passwd   PwTmp;
	char           *tmpstr, *mplexdir, *fname;
#ifdef VALIAS
	char           *ptr1, *ptr2;
#endif
	char            Domain[MAX_PW_DOMAIN], DomainDir[MAX_BUFF], OldDir[MAX_BUFF], User[MAX_BUFF], listfile[MAX_BUFF];
	int             count, reset_dir, dec_dir, users_per_level = 0;
	uid_t           uid, myuid;
	gid_t           gid;

	dec_dir = 1;
	if(get_options(argc, argv, Domain, listfile, &reset_dir, &dec_dir, &users_per_level))
		return(1);
	if(!vget_assign(Domain, DomainDir, MAX_BUFF, &uid, &gid))
	{
		fprintf(stderr, "%s: No such domain\n", Domain);
		return(1);
	}
	if ((myuid = getuid()) != uid)
	{
		if (setgid(gid) || setuid(uid))
		{
			perror("setuid");
			return(1);
		}
	}
	if(!(fs = fopen(listfile, "r")))
	{
		perror(listfile);
		return(1);
	}
	/*- reset dir control external to this program */
	if(reset_dir)
	{
		printf("Resetting directory layout status ");
		fflush(stdout);
		vdel_dir_control(Domain);
		printf("done\n");
	}
	printf("Working on users ");
	fflush(stdout);
	for(count = 0;;count++)
	{
		if (fscanf(fs, "%s", User) != 1)
		{
			if (ferror(fs))
				break;
		}
		if(feof(fs))
			break;
		/*
		 * get old pw struct 
		 */
		if (!(pw = vauth_getpw(User, Domain)))
		{
			fprintf(stderr, "%s@%s: vauth_getpw failed\n", User, Domain);
			continue;
		}
		if(!reset_dir && dec_dir)
			dec_dir_control(pw->pw_dir, User, Domain, -1, -1);
		PwTmp = *pw;
		pw = &PwTmp;
		scopy(OldDir, pw->pw_dir, MAX_BUFF);
		/*- format new directory string */
		mplexdir = (char *) get_Mplexdir(User, Domain, 0, uid, gid);
		/*- get next dir */
		fname = open_big_dir(User, Domain, mplexdir);
		tmpstr = next_big_dir(uid, gid, users_per_level);
		close_big_dir(fname, Domain, uid, gid);
		/*- get space for pw_dir */
		if(!(pw->pw_dir = (char *) malloc(MAX_BUFF)))
		{
			fprintf(stderr, "%s@%s: malloc: %s\n", User, Domain, strerror(errno));
			continue;
		}
		if (*tmpstr)
			snprintf(pw->pw_dir, MAX_BUFF, "%s/%s/%s", mplexdir, tmpstr, User);
		else
			snprintf(pw->pw_dir, MAX_BUFF, "%s/%s", mplexdir, User);
		if(mplexdir)
			free(mplexdir);
		if(!strncmp(OldDir, pw->pw_dir, MAX_BUFF))
		{
			free(pw->pw_dir);
			continue;
		}
		/*- Replace all occurence of OldDir with NewDir */
#ifdef VALIAS
		for(;;)
		{
			if(!(ptr1 = valias_select(User, Domain)))
				break;
			if(!(ptr2 = replacestr(ptr1, OldDir, pw->pw_dir)))
				continue;
			if(ptr1 == ptr2)
				continue;
			valias_update(User, Domain, ptr1, ptr2);
			free(ptr2);
		}
#endif
		/*- move directory */
		if (!access(OldDir, F_OK) && MoveFile(OldDir, pw->pw_dir))
		{
			fprintf(stderr, "MoveFile: %s->%s: %s\n", OldDir, pw->pw_dir, strerror(errno));
			free(pw->pw_dir);
			continue;
		}
		/*- update database */
		if (vauth_setpw(pw, Domain))
		{
			if (!access(pw->pw_dir, F_OK) && MoveFile(pw->pw_dir, OldDir))
				fprintf(stderr, "MoveFile: %s->%s: %s\n", pw->pw_dir, OldDir, strerror(errno));
			free(pw->pw_dir);
			continue;
		}
		/*- printf("%s@%s old %s new %s done\n", User, Domain, OldDir, pw->pw_dir); -*/
		free(pw->pw_dir);
	}
	fclose(fs);
	printf("done %d users\n", count);
	return (0);
}

int
get_options(int argc, char **argv, char *Domain, char *listfile, int *reset_dir,
	int *dec_dir, int *users_per_level)
{
	int             c;
	char           *tmpstr;

	if ((tmpstr = strrchr(argv[0], '/')) != NULL)
		tmpstr++;
	else
		tmpstr = argv[0];
	*listfile = *Domain = 0;
	*reset_dir = 0;
	while ((c = getopt(argc, argv, "vrRd:")) != -1)
	{
		switch (c)
		{
		case 'v':
			verbose = 1;
			break;
		case 'd':
			scopy(Domain, optarg, MAX_PW_DOMAIN);
			break;
		case 'r':
			*reset_dir = 1;
			break;
		case 'l':
			*users_per_level = atoi(optarg);
			break;
		case 'R':
			*dec_dir = 0;
			break;
		default:
			fprintf(stderr, "USAGE: %s -d domain_name [-r] [-R] user_list\n", tmpstr);
			fprintf(stderr, "           -r Reset Dir Control for the entire domain\n");
			fprintf(stderr, "           -R Do not Decrement Dir Control in the original filesystem\n");
			return(1);
		}
	}
	if (optind < argc)
		scopy(listfile, argv[optind++], MAX_BUFF);
	if (!*listfile)
	{
		fprintf(stderr, "USAGE: %s -d domain_name [-r] [-R] user_list\n", tmpstr);
		fprintf(stderr, "           -r Reset Dir Control for the entire domain\n");
		fprintf(stderr, "           -R Do not Decrement Dir Control in the original filesystem\n");
		fprintf(stderr, "must supply file containing user list\n");
		return(1);
	}
	return(0);
}

void
getversion_vreorg_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
