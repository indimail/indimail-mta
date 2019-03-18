/*
 * $Log: deldomain.c,v $
 * Revision 2.19  2019-03-18 18:20:07+05:30  Cprogrammer
 * open_master now returns 2 if hostcntrl not present
 *
 * Revision 2.18  2018-09-11 10:25:27+05:30  Cprogrammer
 * fixed commpiler warnings
 *
 * Revision 2.17  2017-04-28 09:37:37+05:30  Cprogrammer
 * fixed incorrect handling of return status from del_control()
 *
 * Revision 2.16  2013-08-03 20:21:51+05:30  Cprogrammer
 * send sighup through post_handle
 *
 * Revision 2.15  2010-08-12 18:55:28+05:30  Cprogrammer
 * remove filters prefilt and postfilt when removing a domain
 *
 * Revision 2.14  2010-05-17 10:15:38+05:30  Cprogrammer
 * use .base_path control file in domains directory
 *
 * Revision 2.13  2009-09-23 14:59:28+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.12  2009-02-18 09:06:40+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.11  2009-01-15 08:55:18+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.10  2008-09-14 20:33:01+05:30  Cprogrammer
 * BUG - Code was wrongly using .filesysetms to remove directories
 *
 * Revision 2.9  2008-08-02 09:06:38+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.8  2008-05-28 16:35:06+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.7  2005-12-29 22:42:55+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.6  2004-07-12 22:44:54+05:30  Cprogrammer
 * replaced system() with runcmmd()
 *
 * Revision 2.5  2004-01-06 17:32:43+05:30  Cprogrammer
 * delete dir_control and limits before the assign file is deleted
 *
 * Revision 2.4  2003-01-26 02:13:08+05:30  Cprogrammer
 * corrected removal of .filesystem when alias domain was deleted
 *
 * Revision 2.3  2002-10-27 19:41:13+05:30  Cprogrammer
 * remove limits for domain
 *
 * Revision 2.2  2002-08-25 22:32:07+05:30  Cprogrammer
 * code for deleting etrn, autoturn domains
 *
 * Revision 2.1  2002-05-05 21:07:46+05:30  Cprogrammer
 * use return value of 1 to determine if domain is an alias domain
 *
 * Revision 1.9  2001-12-22 18:06:40+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.8  2001-12-21 01:24:45+05:30  Cprogrammer
 * add alias domain to table aliasdomain on central db for distributed domains
 *
 * Revision 1.7  2001-12-02 20:19:36+05:30  Cprogrammer
 * conditional compilation for mysql
 *
 * Revision 1.6  2001-12-02 18:41:17+05:30  Cprogrammer
 * modification for dir_control filesystem modification
 *
 * Revision 1.5  2001-11-29 13:17:48+05:30  Cprogrammer
 * added verbose switch
 *
 * Revision 1.4  2001-11-28 22:57:07+05:30  Cprogrammer
 * modifiction for change in vdelfiles() function
 *
 * Revision 1.3  2001-11-24 12:18:52+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:14+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#ifndef	lint
static char     sccsid[] = "$Id: deldomain.c,v 2.19 2019-03-18 18:20:07+05:30 Cprogrammer Exp mbhangui $";
#endif

int
vdeldomain(char *domain)
{
	char            Dir[MAX_BUFF], TmpBuf[MAX_BUFF + 15], BasePath[MAX_BUFF];
	char           *ptr, *tmpstr, *base_path;
	int             is_alias, i;
	FILE           *fp;
	uid_t           uid;
	gid_t           gid;
	char           *FileSystems[] = {
		"A2E",
		"F2K",
		"L2P",
		"Q2S",
		"T2Zsym",
	};

	if (!domain || !*domain)
	{
		error_stack(stderr, "Invalid Domain Name\n");
		return (-1);
	}
	lowerit(domain);
	if (use_etrn == 2)
	{
		if (!(ptr = autoturn_dir(domain)))
		{
			error_stack(stderr, "Domain %s does not exist\n", domain);
			return (-1);
		} else
		if (!(tmpstr = vget_assign("autoturn", 0, 0, &uid, &gid)))
		{
			error_stack(stderr, "Domain %s does not exist: No entry for autoturn in assign\n",
				domain);
			return (-1);
		}
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/%s", tmpstr, ptr);
		if (vdelfiles(TmpBuf, "", ptr) != 0)
		{
			error_stack(stderr, "Failed to remove Dir %s: %s\n", TmpBuf, strerror(errno));
			return (-1);
		}
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/.qmail-%s-default", tmpstr, ptr);
		if ((ptr = strrchr(TmpBuf, '/')))
		{
			ptr++;
			if (*ptr == '.' && *(ptr + 1) == 'q')
				ptr++;
			for(;*ptr;ptr++)
			{
				if (*ptr == '.')
					*ptr = ':';
			}
			if (!access(TmpBuf, F_OK) && unlink(TmpBuf))
			{
				perror(TmpBuf);
				return(-1);
			}
		}
		del_control(domain);
		return(0);
	}
	if (!vget_assign(domain, Dir, MAX_BUFF, &uid, &gid))
	{
		error_stack(stderr, "Domain %s does not exist\n", domain);
		return (-1);
	}
	snprintf(TmpBuf, sizeof(TmpBuf), "%s/.base_path", Dir);
	if ((fp = fopen(TmpBuf, "r")))
	{
		if (fscanf(fp, "%s", BasePath) != 1)
			*BasePath = 0;
		fclose(fp);
	} else
		*BasePath = 0;
	snprintf(TmpBuf, sizeof(TmpBuf), "%s/.aliasdomains", Dir);
	if ((is_alias = is_alias_domain(domain)) == 1)
	{
		if (verbose)
			printf("Removing alias domain %s\n", domain);
#ifdef CLUSTERED_SITE
		i = open_master();
		if (i && i != 2) {
			error_stack(stderr, "vdeldomain: Failed to open Master Db\n");
			return (-1);
		}
		if (!i && vauth_get_realdomain(domain) && vauth_delaliasdomain(domain)) {
			error_stack(stderr, "Failed to remove alias Domain %s from aliasdomain Table\n",
				domain);
			return (-1);
		}
#endif
		if (remove_line(domain, TmpBuf, 0, 0600) == -1) {
			error_stack(stderr, "Failed to remove alias Domain %s from %s\n",
				domain, TmpBuf);
			return (-1);
		}
	} else
	if ((fp = fopen(TmpBuf, "r")) != (FILE *) NULL)
	{
		if (verbose)
			printf("Removing domains aliased to %s\n", domain);
		for(;;)
		{
			if (!fgets(TmpBuf, MAX_BUFF - 2, fp))
			{
				if (feof(fp))
					break;
				perror("vdel_dir_control: fgets");
				return (-1);
			}
			if ((tmpstr = strrchr(TmpBuf, '\n')) != NULL)
				*tmpstr = 0;
			if (vdeldomain(TmpBuf))
			{
				fclose(fp);
				return (-1);
			}
		}
		fclose(fp);
	}
	if (is_alias != 1)
	{
		if (vdel_dir_control(domain))
		{
			error_stack(stderr, "vdel_dir_control: Failed to remove dir_control for %s\n", 
				domain);
			return (-1);
		}
#ifdef ENABLE_DOMAIN_LIMITS
		vdel_limits(domain);
#endif
	}
	snprintf(TmpBuf, sizeof(TmpBuf), "prefilt@%s", domain);
	vfilter_delete(TmpBuf, -1);
	snprintf(TmpBuf, sizeof(TmpBuf), "postfilt@%s", domain);
	vfilter_delete(TmpBuf, -1);
	/*
	 * delete the assign file line 
	 */
	if (del_domain_assign(domain, Dir, uid, gid))
		return (-1);
	/* delete the Mail File systems */
	if (is_alias != 1)
	{
		if (*BasePath)
			base_path = BasePath;
		else
			getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
		for (i = 0; i < 5;i++)
		{
			snprintf(TmpBuf, MAX_BUFF, "%s/%s/%s", base_path, FileSystems[i], domain);
			if (verbose)
				printf("Removing %s\n", TmpBuf);
			if (vdelfiles(TmpBuf, "", domain))
			{
				error_stack(stderr, "Failed to remove Dir %s: %s\n", TmpBuf, strerror(errno));
				continue;
			}
		}
	}
	/*- Delete /var/indimail/domains/domain_name */
	if (vdelfiles(Dir, "", domain) != 0)
	{
		error_stack(stderr, "Failed to remove Dir %s: %s\n",
			Dir, strerror(errno));
		return (-1);
	}
	/*
	 * call the auth module to delete the domain from the authentication
	 * database
	 */
	if (is_alias != 1 && vauth_deldomain(domain))
		return (-1);
	/*
	 * delete the email domain from the qmail control files 
	 */
	if (del_control(domain) == -1)
		return (-1);
	/*
	 * return back to the callers directory 
	 */
	return (0);
}

void
getversion_deldomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
