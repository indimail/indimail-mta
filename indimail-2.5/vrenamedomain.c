/*
 * $Log: vrenamedomain.c,v $
 * Revision 2.19  2016-06-09 15:32:32+05:30  Cprogrammer
 * run if indimail gid is present in process supplementary groups
 *
 * Revision 2.18  2016-06-09 14:22:59+05:30  Cprogrammer
 * allow privilege to process running with indimail gid
 *
 * Revision 2.17  2016-05-25 09:09:54+05:30  Cprogrammer
 * use LIBEXECDIR for post handle
 *
 * Revision 2.16  2016-01-19 00:35:46+05:30  Cprogrammer
 * fixed bogus comparision.
 *
 * Revision 2.15  2013-08-03 20:23:02+05:30  Cprogrammer
 * send sighup through post_handle
 *
 * Revision 2.14  2010-04-30 14:44:12+05:30  Cprogrammer
 * free pointer returned by replacestr()
 *
 * Revision 2.13  2010-02-17 10:58:50+05:30  Cprogrammer
 * added post handle
 *
 * Revision 2.12  2009-09-23 15:00:37+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.11  2008-09-17 21:45:14+05:30  Cprogrammer
 * setuid to root for indimail
 *
 * Revision 2.10  2008-08-02 09:10:42+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.9  2008-05-28 17:42:27+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.8  2005-12-29 22:53:41+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.7  2004-07-12 22:48:48+05:30  Cprogrammer
 * replaced system() with runcmmd()
 *
 * Revision 2.6  2004-07-02 18:15:05+05:30  Cprogrammer
 * renamed .domain to domain
 *
 * Revision 2.5  2004-05-17 14:02:33+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.4  2003-01-26 02:15:58+05:30  Cprogrammer
 * setuid() to allow updation of assign file
 *
 * Revision 2.3  2002-08-25 22:37:04+05:30  Cprogrammer
 * new function add_control()
 *
 * Revision 2.2  2002-05-09 00:38:43+05:30  Cprogrammer
 * display the real domain when target domain exists
 * passed the domain directory to vauth_renamedomain()
 *
 * Revision 2.1  2002-05-05 22:24:22+05:30  Cprogrammer
 * program to rename alias and real domains
 *
 */
#include "indimail.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: vrenamedomain.c,v 2.19 2016-06-09 15:32:32+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(int argc, char **argv)
{
	uid_t           uid;
	gid_t           gid;
	FILE           *fp;
	char            OldDir[MAX_BUFF], NewDir[MAX_BUFF], TmpBuf[MAX_BUFF], DomDir[MAX_BUFF];
	char           *real_domain, *ptr, *tmpstr, *base_argv0;
	struct passwd  *pw;
	struct stat     statbuf;

	if(argc != 3)
	{
		error_stack(stderr, "USAGE: %s old_domain_name new_domain_name\n", argv[0]);
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = getuid();
	gid = getgid();
	if (uid != 0 && uid != indimailuid && gid != indimailgid && check_group(indimailgid) != 1)
	{
		error_stack(stderr, "you must be root or indimail to run this program\n");
		return (1);
	}
	if (uid && setuid(0))
	{
		error_stack(stderr, "setuid-root: %s\n", strerror(errno));
		return (1);
	}
	if((real_domain = vget_real_domain(argv[2])) != (char *) 0)
	{
		error_stack(stderr, "Domain %s exists [%s]\n", argv[2], real_domain);
		return (1);
	} else
	if(vget_assign(argv[2], NewDir, MAX_BUFF, &uid, &gid))
	{
		error_stack(stderr, "Domain %s exists\n", argv[2]);
		return (1);
	} else
	if(is_alias_domain(argv[1]) == 1)
	{
		if(!(real_domain = vget_real_domain(argv[1])))
		{
			error_stack(stderr, "Domain %s does not exist\n", argv[1]);
			return (1);
		} 
		printf("Renaming alias domain %s (%s) to %s\n", argv[1], real_domain, argv[2]);
		if(vaddaliasdomain(real_domain, argv[2]))
			error_stack(stderr, "vaddaliasdomain: %s->%s\n", real_domain, argv[2]);
		else
		if(vdeldomain(argv[1]))
			error_stack(stderr, "vdeldomain: %s\n", argv[1]);
		return (0);
	} else
	if(!vget_assign(argv[1], OldDir, MAX_BUFF, &uid, &gid))
	{
		error_stack(stderr, "Domain %s does not exist\n", argv[1]);
		return (1);
	}
	scopy(NewDir, OldDir, MAX_BUFF);
	if(!(ptr = replacestr(NewDir, argv[1], argv[2])))
	{
		error_stack(stderr, "%s: %s->%s: %s\n", NewDir, argv[1], argv[2], strerror(errno));
		return (1);
	} else
	if(rename(OldDir, ptr))
	{
		error_stack(stderr, "rename: %s->%s: %s\n", OldDir, ptr, strerror(errno));
		return (1);
	}
	scopy(DomDir, ptr, MAX_BUFF);
	if((tmpstr = strstr(DomDir, "/domains")) != (char *) 0)
		*tmpstr = 0;
	printf("Renaming real domain %s to %s\n", argv[1], argv[2]);
	if(vauth_renamedomain(argv[1], argv[2], ptr))
	{
		if (ptr != NewDir)
			free(ptr);
		return (1);
	}
	else
	if(add_domain_assign(argv[2], DomDir, uid, gid))
	{
		if (ptr != NewDir)
			free(ptr);
		return (1);
	} else
	if(add_control(argv[2], 0))
	{
		if (ptr != NewDir)
			free(ptr);
		return (1);
	} else
	if(del_domain_assign(argv[1], OldDir, uid, gid))
	{
		if (ptr != NewDir)
			free(ptr);
		return (1);
	}
	CreateDomainDirs(argv[2], uid, gid);
	snprintf(TmpBuf, MAX_BUFF, "%s/.aliasdomains", ptr);
	/*- Relink Alias Domains */
	if((fp = fopen(TmpBuf, "r")))
	{
		printf("Relinking domains aliased to %s\n", argv[1]);
		for(;;)
		{
			if(!fgets(TmpBuf, MAX_BUFF - 2, fp))
				break;
			if((tmpstr = strchr(TmpBuf, '\n')))
				*tmpstr = 0;
			if(!is_alias_domain(TmpBuf))
			{
				error_stack(stderr, "%s: Not an alias domain\n", TmpBuf);
				continue;
			} else
			if(!vget_assign(TmpBuf, OldDir, MAX_BUFF, &uid, &gid))
			{
				error_stack(stderr, "Domain %s does not exist\n", TmpBuf);
				continue;
			} else
			if(lstat(OldDir, &statbuf))
			{
				error_stack(stderr, "%s: %s\n", OldDir, strerror(errno));
				continue;
			} else
			if(!S_ISLNK(statbuf.st_mode))
			{
				error_stack(stderr, "%s (%s): Not an alias domain\n", OldDir, TmpBuf);
				continue;
			} else
			if(unlink(OldDir))
			{
				error_stack(stderr, "unlink: %s: %s\n", OldDir, strerror(errno));
				continue;
			} else
			if(symlink(ptr, OldDir))
			{
				error_stack(stderr, "symlink: %s->%s: %s\n", OldDir, ptr, strerror(errno));
				getchar();
				continue;
			}
			printf("Linked Domain %s to %s [%s->%s]\n", TmpBuf, argv[2], OldDir, ptr);
		}
		fclose(fp);
	}
	snprintf(TmpBuf, MAX_BUFF, "%s/.domain_rename", ptr);
	close(open(TmpBuf, O_TRUNC|O_CREAT, INDIMAIL_QMAIL_MODE));
	for (pw = vauth_getall(argv[2], 1, 0); pw; pw = vauth_getall(argv[2], 0, 0))
	{
		snprintf(TmpBuf, MAX_BUFF, "%s/Maildir", pw->pw_dir);
		if(access(TmpBuf, F_OK))
		{
			if(errno != ENOENT)
				error_stack(stderr, "%s: %s\n", TmpBuf, strerror(errno));
			continue;
		}
		snprintf(TmpBuf, MAX_BUFF, "%s/Maildir/domain", pw->pw_dir);
		if(!(fp = fopen(TmpBuf, "w")))
		{
			error_stack(stderr, "%s: %s\n", TmpBuf, strerror(errno));
			continue;
		}
		error_stack(fp, "%s\n", argv[2]);
		fclose(fp);
	}
	snprintf(TmpBuf, MAX_BUFF, "%s/.domain_rename", ptr);
	unlink(TmpBuf);
	if (ptr != NewDir)
		free(ptr);
	if (!(ptr = getenv("POST_HANDLE")))
	{
		if (!(base_argv0 = strrchr(argv[0], '/')))
			base_argv0 = argv[0];
		return(post_handle("%s/%s %s %s", LIBEXECDIR, base_argv0, argv[1], argv[2]));
	} else
		return(post_handle("%s %s %s", ptr, argv[1], argv[2]));
}

void
getversion_vrenamedomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
