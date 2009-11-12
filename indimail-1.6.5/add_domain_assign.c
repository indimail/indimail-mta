/*
 * $Log: add_domain_assign.c,v $
 * Revision 2.6  2008-08-02 09:05:24+05:30  Cprogrammer
 * new function error_stack
 *
 * Revision 2.5  2005-12-29 22:38:43+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.4  2004-05-17 14:00:14+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.3  2002-09-03 17:49:11+05:30  Cprogrammer
 * corrected non-etrn domains not getting added
 *
 * Revision 2.2  2002-08-25 22:29:42+05:30  Cprogrammer
 * added code for autoturn
 *
 * Revision 2.1  2002-05-05 21:07:01+05:30  Cprogrammer
 * changed argument name dir to be more meaningful
 *
 * Revision 1.3  2001-11-24 12:17:29+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:30+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: add_domain_assign.c,v 2.6 2008-08-02 09:05:24+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * Add a domain to all the control files 
 * And signal qmail
 */
int
add_domain_assign(char *domain, char *domain_base_dir, uid_t uid,
				  gid_t gid)
{
	FILE           *fs1 = NULL;
	char            filename[MAX_BUFF], tmpstr[MAX_BUFF];
	char           *qmaildir, *controldir;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	/*- stat assign file, if it's not there create one */
	snprintf(filename, MAX_BUFF, "%s/users/assign", qmaildir);
	if (access(filename, F_OK))
	{
		/*
		 * put a . on one line by itself 
		 */
		if (!(fs1 = fopen(filename, "w+")))
		{
			error_stack(stderr, "%s: %s\n", filename, strerror(errno));
			return (-1);
		}
		fputs(".\n", fs1);
		fclose(fs1);
	}
	if(use_etrn == 2  && !vget_assign("autoturn", 0, 0, 0, 0))
	{
		GetIndiId(&indimailuid, &indimailgid);
		snprintf(tmpstr, MAX_BUFF, "+autoturn-:indimail:%lu:%lu:%s/autoturn:-::",
			(unsigned long) indimailuid, (unsigned long) indimailgid, qmaildir);
		/*- update the file and add the above line and remove duplicates */
		if(update_file(filename, tmpstr, INDIMAIL_QMAIL_MODE))
			return (-1);
		if (!OptimizeAddDomain)
			update_newu();
	} else
	{
		snprintf(tmpstr, MAX_BUFF, "+%s-:%s:%lu:%lu:%s%s%s:-::", domain, domain, (unsigned long) uid,
			(unsigned long) gid, domain_base_dir, use_etrn ? "/" : "/domains/", domain);
		if(update_file(filename, tmpstr, INDIMAIL_QMAIL_MODE))
			return (-1);
		/*- compile the assign file */
		if (!OptimizeAddDomain)
			update_newu();
	}
	return(0);
}

void
getversion_add_domain_assign_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
