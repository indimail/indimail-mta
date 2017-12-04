/*
 * $Log: adddomain.c,v $
 * Revision 2.19  2017-03-13 13:33:58+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.18  2016-06-13 14:22:47+05:30  Cprogrammer
 * change umask to 0007 to avoid creating .qmail-default with restrictive perms
 *
 * Revision 2.17  2016-05-17 14:40:32+05:30  Cprogrammer
 * replace control directory with CONTROLDIR
 *
 * Revision 2.16  2013-08-03 20:21:40+05:30  Cprogrammer
 * send sighup through post_handle
 *
 * Revision 2.15  2009-09-28 13:53:29+05:30  Cprogrammer
 * added option chk_rcpt to selectively add domains to chkrcptdomains
 *
 * Revision 2.14  2009-09-23 14:59:21+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.13  2009-04-12 10:46:52+05:30  Cprogrammer
 * added domain to chkrcptdomains
 *
 * Revision 2.12  2009-01-28 23:18:06+05:30  Cprogrammer
 * rollback entries from control files, delete domain directory on failure
 *
 * Revision 2.11  2008-08-02 09:05:46+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.10  2008-06-13 08:34:20+05:30  Cprogrammer
 * use vfilter only if VFILTER is defined
 *
 * Revision 2.9  2008-06-05 16:15:25+05:30  Cprogrammer
 * moved vfilter, vdelivermail to sbin
 *
 * Revision 2.8  2005-12-29 22:38:47+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.7  2004-07-12 22:44:36+05:30  Cprogrammer
 * replaced system() with runcmmd
 *
 * Revision 2.6  2004-05-17 14:00:19+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.5  2003-11-20 11:37:53+05:30  Cprogrammer
 * create bulk mail directory when creating domains
 *
 * Revision 2.4  2003-01-26 18:30:36+05:30  Cprogrammer
 * added option to set vfilter
 *
 * Revision 2.3  2002-11-26 19:25:10+05:30  Cprogrammer
 * incorrect variable passed to del_domain_assign
 *
 * Revision 2.2  2002-09-20 23:57:07+05:30  Cprogrammer
 * corrected entry for autoturn not getting added in assign file
 *
 * Revision 2.1  2002-08-25 22:30:11+05:30  Cprogrammer
 * code for adding autoturn, etrn domains
 *
 * Revision 1.7  2002-02-23 01:54:16+05:30  Cprogrammer
 * check for domain name length
 *
 * Revision 1.6  2002-02-23 00:23:59+05:30  Cprogrammer
 * changed MAX_DOMAINNAME to MAX_PW_DOMAIN
 *
 * Revision 1.5  2001-12-30 09:52:32+05:30  Cprogrammer
 * length of domain names check
 *
 * Revision 1.4  2001-12-13 13:35:27+05:30  Cprogrammer
 * del domain from assign if vauth_addomain fails
 *
 * Revision 1.3  2001-11-24 12:17:46+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:34+05:30  Cprogrammer
 * Added getversion_adddomain_c()
 *
 * Revision 1.1  2001-10-24 18:14:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: adddomain.c,v 2.19 2017-03-13 13:33:58+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vadddomain(char *domain, char *ipaddr, char *dir, uid_t uid, gid_t gid, int chk_rcpt)
{
	FILE           *fs;
	char           *ptr;
	char            tmpbuf[MAX_BUFF];
	int             i;

	if (!domain || !*domain || *domain == '-')
	{
		error_stack(stderr, "Invalid Domain Name\n");
		return (-1);
	}
	if (slen(domain) > MAX_PW_DOMAIN)
	{
		error_stack(stderr, "%s: Name too long\n", domain);
		return (-1);
	}
	if (!dir || !*dir)
	{
		error_stack(stderr, "Invalid Directory\n");
		return (-1);
	}
	/*
	 * check invalid email domain characters 
	 */
	for (ptr = domain, i = 0; *ptr; ptr++, i++)
	{
		if ((*ptr == '-') || (*ptr == '.'))
			continue;
		if (i > MAX_PW_DOMAIN)
		{
			error_stack(stderr, "Domain name too long\n");
			return (-1);
		}
		if (!isalnum((int) *ptr))
		{
			error_stack(stderr, "Invalid char '%c'\n", *ptr);
			return (-1);
		}
		if (isupper((int) *ptr))
			*ptr = tolower(*ptr);
	}
	if ((*(ptr - 1)) == '-')
	{
		error_stack(stderr, "Last component cannot be '-'\n");
		return (-1);
	}
	if (vget_assign(domain, NULL, MAX_BUFF, NULL, NULL))
	{
		error_stack(stderr, "Domain %s exists\n", domain);
		return (-1);
	}
	umask(INDIMAIL_UMASK);
	if (use_etrn == 1)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/%s", dir, domain);
		if (vmake_maildir(tmpbuf, uid, gid, domain) == -1)
		{
			error_stack(stderr, "vmake_maildir(%s, %d, %d): %s\n", tmpbuf, uid, gid, strerror(errno));
			return(-1);
		}
	} else
	if (use_etrn == 2)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/%s", dir, ipaddr);
		if (vmake_maildir(tmpbuf, uid, gid, domain) == -1)
		{
			error_stack(stderr, "vmake_maildir(%s, %d, %d): %s\n", tmpbuf, uid, gid, strerror(errno));
			return(-1);
		}
	} else
	if (!use_etrn)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/domains/%s", dir, domain);
		if (r_mkdir(tmpbuf, INDIMAIL_DIR_MODE, uid, gid))
		{
			error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
			return (-1);
		}
		snprintf(tmpbuf, MAX_BUFF, "%s/%s/%s", CONTROLDIR, domain,
			 (((ptr = getenv("BULK_MAILDIR"))) ? ptr : BULK_MAILDIR));
		if (r_mkdir(tmpbuf, INDIMAIL_DIR_MODE, uid, gid))
		{
			error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
			return (-1);
		}
	}
	if (use_etrn == 1)
		snprintf(tmpbuf, MAX_BUFF, "%s/%s/.qmail-default", dir, domain);
	else
	if (use_etrn == 2)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/.qmail-%s-default", dir, ipaddr);
		i = slen(dir);
		for (ptr = tmpbuf + i + 8;*ptr;ptr++)
		{
			if (*ptr == '.')
				*ptr = ':';
		}
	}
	else
	if (!use_etrn)
		snprintf(tmpbuf, MAX_BUFF, "%s/domains/%s/.qmail-default", dir, domain);
	umask(0007);
	if (!(fs = fopen(tmpbuf, "w+")))
	{
		error_stack(stderr, "fopen: %s: %s\n", tmpbuf, strerror(errno));
		return (-1);
	} else
	{
		if (use_etrn == 1)
			fprintf(fs, "%s/%s/Maildir/\n", dir, domain);
		else
		if (use_etrn == 2)
			fprintf(fs, "./%s/Maildir/\n", ipaddr);
		else
		{
#ifdef VFILTER
			if (use_vfilter)
				fprintf(fs, "| %s/sbin/vfilter '' bounce-no-mailbox\n", PREFIX);
			else
#endif
				fprintf(fs, "| %s/sbin/vdelivermail '' bounce-no-mailbox\n", PREFIX);
		}
		fclose(fs);
	}
	if (chown(tmpbuf, uid, gid))
	{
		error_stack(stderr, "chown(%s, %d, %d): %s\n", tmpbuf, uid, gid, strerror(errno));
		return(-1);
	}
	if (add_domain_assign(domain, dir, uid, gid))
		return(-1);
	umask(INDIMAIL_UMASK);
	if (!use_etrn || use_etrn == 1)
	{
		if (add_control(domain, domain))
			return(-1);
	} else
	if (use_etrn == 2)
	{
		snprintf(tmpbuf, MAX_BUFF, "autoturn-%s", ipaddr);
		if (add_control(domain, tmpbuf))
			return(-1);
	}
	if (!use_etrn)
	{
		if (vauth_adddomain(domain))
		{
			snprintf(tmpbuf, MAX_BUFF, "%s/domains/%s", dir, domain);
			del_domain_assign(domain, tmpbuf, uid, gid);
			del_control(domain);
			vdelfiles(tmpbuf, 0, 0);
			return(-1);
		}
		if (chk_rcpt)
		{
			snprintf(tmpbuf, MAX_BUFF, "%s/chkrcptdomains", CONTROLDIR);
			if (update_file(tmpbuf, domain, INDIMAIL_QMAIL_MODE))
			{
				snprintf(tmpbuf, MAX_BUFF, "%s/domains/%s", dir, domain);
				del_domain_assign(domain, tmpbuf, uid, gid);
				del_control(domain);
				vdelfiles(tmpbuf, 0, 0);
				return(-1);
			}
		}
		CreateDomainDirs(domain, uid, gid);
	}
	return (0);
}

void
getversion_adddomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
