/*
 * $Log: vmoveuser.c,v $
 * Revision 2.10  2016-05-25 09:09:22+05:30  Cprogrammer
 * use LIBEXECDIR for post handle
 *
 * Revision 2.9  2010-03-02 08:18:27+05:30  Cprogrammer
 * changed Username xxx@yyy does not exist to xxx@yyy: No such user
 *
 * Revision 2.8  2010-02-17 14:14:24+05:30  Cprogrammer
 * added post handle
 *
 * Revision 2.7  2009-09-30 00:24:05+05:30  Cprogrammer
 * do setuid so that move succeeds
 *
 * Revision 2.6  2008-08-02 09:10:32+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.5  2008-06-13 10:57:17+05:30  Cprogrammer
 * conditional compilation of renaming of valias entries (if VALIAS defined)
 *
 * Revision 2.4  2008-05-28 17:41:41+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.3  2003-06-25 12:20:01+05:30  Cprogrammer
 * added check for local user
 *
 * Revision 2.2  2003-06-18 23:40:15+05:30  Cprogrammer
 * update all occurence instead of only for the given user
 *
 * Revision 2.1  2002-07-03 01:22:02+05:30  Cprogrammer
 * copy passwd structure to prevent overwriting static location returned by vauth_getpw()
 * avoid unecessary malloc
 *
 * Revision 1.7  2002-02-22 17:35:09+05:30  Cprogrammer
 * replaced rename() with MoveFile() to allow moving between filesystems
 *
 * Revision 1.6  2001-12-09 23:56:50+05:30  Cprogrammer
 * old alias_line passed to valias_update
 *
 * Revision 1.5  2001-12-02 20:23:44+05:30  Cprogrammer
 * used valias_delete and valias_insert for valias_update for cdb
 *
 * Revision 1.4  2001-12-01 23:10:32+05:30  Cprogrammer
 * valias_update to update new directory in valias
 *
 * Revision 1.3  2001-11-24 12:22:08+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:59+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:42+05:30  Cprogrammer
 * Initial revision
 *
 * vmoveuser 
 *
 * Moves the user directory to another directory in the same file system
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
static char     sccsid[] = "$Id: vmoveuser.c,v 2.10 2016-05-25 09:09:22+05:30 Cprogrammer Exp mbhangui $";
#endif

int
main(int argc, char **argv)
{
	struct passwd  *pw;
	struct passwd   PwTmp;
	char           *tmpstr, *Domain, *NewDir, *User, *real_domain, *base_argv0;
	char            OldDir[MAX_BUFF], Dir[MAX_BUFF];
#if defined(CLUSTERED_SITE) || defined(VALIAS)
	char           *ptr1;
#endif
#ifdef VALIAS
	char           *ptr2;
	char            tmp_domain[MAX_BUFF];
#endif
	uid_t           uid;
	gid_t           gid;
	int             i;
#ifdef CLUSTERED_SITE
	int             err;
	char            TmpBuf[MAX_BUFF];
	char           *mailstore;
#endif

	if (argc != 3)
	{
		if ((tmpstr = strrchr(argv[0], '/')) != NULL)
			tmpstr++;
		else
			tmpstr = argv[0];
		fprintf(stderr, "USAGE: %s user new_dir\n", tmpstr);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	if (uid != 0 && uid != indimailuid)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return (1);
	}
	User = argv[1];
	NewDir = argv[2];
	if ((tmpstr = strchr(User, '@')) != NULL)
	{
		*tmpstr = 0;
		Domain = tmpstr + 1;
	} else
	{
		fprintf(stderr, "%s: Not a fully qualified email\n", User);
		return (1);
	}
	if (!(real_domain = vget_real_domain(Domain)))
	{
		fprintf(stderr, "Domain %s does not exist\n", Domain);
		return (1);
	} else
	if (!vget_assign(real_domain, Dir, MAX_BUFF, &uid, &gid))
	{
		fprintf(stderr, "Domain %s does not exist\n", real_domain);
		return (1);
	}
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n", real_domain);
		return (1);
	} else
	if (err == 1)
	{
		if (open_master())
		{
			fprintf(stderr, "vmoveuser: Failed to Open Master Db\n");
			return (1);
		}
		snprintf(TmpBuf, MAX_BUFF, "%s@%s", User, real_domain);
		if ((mailstore = findhost(TmpBuf, 0)) != (char *) 0)
		{
			if ((ptr1 = strrchr(mailstore, ':')) != (char *) 0)
				*ptr1 = 0;
			for(;*mailstore && *mailstore != ':';mailstore++);
			mailstore++;
		} else
		{
			if (userNotFound)
				fprintf(stderr, "%s@%s: No such user\n", User, real_domain);
			else
				fprintf(stderr, "Error connecting to db\n");
			return (1);
		}
		if (!islocalif(mailstore))
		{
			fprintf(stderr, "%s@%s not local (mailstore %s)\n", User, real_domain, mailstore);
			return (1);
		}
	}
#endif
	if (!(pw = vauth_getpw(User, Domain)))
	{
		fprintf(stderr, "%s@%s: No such user\n", User, Domain);
		return (1);
	}
	PwTmp = *pw;
	pw = &PwTmp;
	scopy(OldDir, pw->pw_dir, MAX_BUFF);
	pw->pw_dir = NewDir;
#ifdef VALIAS
	/*
	 * Replace all occurence of OldDir with NewDir
	 */
	for(;;)
	{
		*tmp_domain = 0;
		if (!(ptr1 = valias_select_all(0, tmp_domain, 0)))
			break;
		if (!(ptr2 = replacestr(ptr1, OldDir, NewDir)))
			continue;
		if (ptr1 == ptr2)
			continue;
		valias_update(User, tmp_domain, ptr1, ptr2);
		free(ptr2);
	}
#endif
	/*- move directory */
	if (setuid(0))
	{
		perror("setuid-root");
		return (1);
	}
	if (!access(OldDir, F_OK) && MoveFile(OldDir, NewDir))
	{
		fprintf(stderr, "MoveFile: %s->%s: %s\n", OldDir, pw->pw_dir, strerror(errno));
		return (1);
	}
	/*- update database */
	if ((i = vauth_setpw(pw, Domain)))
	{
		error_stack(stderr, "vauth_setpw failed\n");
		if (!access(NewDir, F_OK) && MoveFile(NewDir, OldDir))
			error_stack(stderr, "MoveFile: %s->%s: %s\n", NewDir, OldDir, strerror(errno));
		return (1);
	}
	printf("%s@%s old %s new %s done\n", User, Domain, OldDir, NewDir);
	if (!(tmpstr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return (post_handle("%s/%s %s@%s %s %s",
					LIBEXECDIR, base_argv0, User, real_domain, OldDir, NewDir));
	} else
		return (post_handle("%s %s@%s %s %s", tmpstr, User, real_domain, OldDir, NewDir));
}

void
getversion_vmoveuser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
