/*
 * $Log: addaliasdomain.c,v $
 * Revision 2.6  2010-03-07 09:27:58+05:30  Cprogrammer
 * check return value of is_distributed_domain for error
 *
 * Revision 2.5  2009-09-23 14:59:02+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.4  2008-08-02 09:04:45+05:30  Cprogrammer
 * new function error_stack
 *
 * Revision 2.3  2005-12-29 22:38:17+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-07-12 22:44:24+05:30  Cprogrammer
 * replaced system() with runcmmd()
 *
 * Revision 2.1  2002-08-25 22:29:50+05:30  Cprogrammer
 * new function for modifying control files
 *
 * Revision 1.5  2001-12-22 18:05:28+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.4  2001-12-21 00:29:42+05:30  Cprogrammer
 * add alias domains to table aliasdomain on central db to ensure mails to domains aliased to
 * distributed domains get delivered
 *
 * Revision 1.3  2001-11-24 12:17:44+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:33+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: addaliasdomain.c,v 2.6 2010-03-07 09:27:58+05:30 Cprogrammer Stab mbhangui $";
#endif

int
vaddaliasdomain(char *old_domain, char *new_domain)
{
	char            Dir[MAX_BUFF], TmpBuf1[MAX_BUFF], TmpBuf2[MAX_BUFF];
	char           *tmpstr, *qmaildir;
	int             i;
	uid_t           uid;
	gid_t           gid;

	if (!new_domain || !*new_domain || *new_domain == '-' || !old_domain || !*old_domain)
	{
		error_stack(stderr, "Invalid Domain Name\n");
		return (-1);
	}
	lowerit(new_domain);
	if (vget_assign(new_domain, 0, MAX_BUFF, 0, 0))
	{
		error_stack(stderr, "Domain %s exists\n", new_domain);
		return (-1);
	}
	lowerit(old_domain);
	if (!vget_assign(old_domain, Dir, MAX_BUFF, &uid, &gid))
	{
		error_stack(stderr, "Domain %s does not exist\n", old_domain);
		return (-1);
	}
#ifdef CLUSTERED_SITE
	if ((i = is_distributed_domain(old_domain)) == -1)
	{
		error_stack(stderr, "%s: is_distributed_domain failed\n", old_domain);
		return (-1);
	} else
	if (i)
	{
		if (open_master())
		{
			error_stack(stderr, "vaddaliasdomain: Failed to open Master Db\n");
			return(-1);
		}
		if (vauth_insertaliasdomain(old_domain, new_domain))
			return(-1);
	}
#endif
	scopy(TmpBuf1, Dir, MAX_BUFF);
	if ((tmpstr = strstr(Dir, "/domains")) != (char *) 0)
		*tmpstr = 0;
	snprintf(TmpBuf2, MAX_BUFF, "%s/domains/%s", Dir, new_domain);
	if (symlink(TmpBuf1, TmpBuf2) != 0)
	{
		error_stack(stderr, "symlink:%s->%s: %s\n",
			TmpBuf2, TmpBuf1, strerror(errno));
		return(-1);
	}
	if (add_domain_assign(new_domain, Dir, uid, gid))
		return(-1);
	if (add_control(new_domain, 0))
		return(-1);
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	snprintf(TmpBuf1, MAX_BUFF, "%s/bin/qmail-sighup", qmaildir);
	if (!access(TmpBuf1, X_OK))
		runcmmd(TmpBuf1, 0);
	else
		signal_process("qmail-send", SIGHUP);
	if (snprintf(TmpBuf1, MAX_BUFF, "%s/domains/%s/.aliasdomains", Dir, old_domain) == -1)
	{
		error_stack(stderr, "string %s/domains/%s/.aliasdomains too long\n",
			Dir, old_domain);
		return(-1);
	}  else
	if (update_file(TmpBuf1, new_domain, 0600))
		return(-1);
	if (chown(TmpBuf1, uid, gid))
	{
		error_stack(stderr, "chown/chmod %s (%d/%d/0600)\n",
			TmpBuf1, uid, gid);
		return(-1);
	}
	return (0);
}

void
getversion_addaliasdomain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
