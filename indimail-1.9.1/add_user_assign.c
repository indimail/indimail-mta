/*
 * $Log: add_user_assign.c,v $
 * Revision 2.6  2016-05-25 08:57:12+05:30  Cprogrammer
 * removed not needed qmaildir variable
 *
 * Revision 2.5  2016-05-18 11:42:01+05:30  Cprogrammer
 * use ASSIGNDIR for users/assign and DOMAINDIR as home for users
 *
 * Revision 2.4  2008-09-14 19:38:27+05:30  Cprogrammer
 * done formatting
 *
 * Revision 2.3  2008-08-02 09:05:54+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.2  2005-12-29 22:38:50+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2004-05-17 14:00:24+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.3  2001-11-24 12:17:42+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:32+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:51+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: add_user_assign.c,v 2.6 2016-05-25 08:57:12+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * add a local user to the users/assign file and compile it
 */
int
add_user_assign(char *user, char *dir)
{
	FILE           *fp;
	static char     filename[MAX_BUFF], tmpstr1[MAX_BUFF], tmpstr2[MAX_BUFF];
	char           *assigndir;

	/*
	 * stat assign file, if it's not there create one 
	 */
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	snprintf(filename, MAX_BUFF, "%s/assign", assigndir);
	if (access(filename, F_OK))
	{
		if(!(fp = fopen(filename, "a+")))
		{
			error_stack(stderr, "fopen(%s, \"a+\"): %s\n", filename, strerror(errno));
			return(-1);
		}
		fprintf(fp, ".\n");
		fclose(fp);
	}
	if(indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	if (!dir || !*dir)
	{
		snprintf(tmpstr1, MAX_BUFF, "=%s:%s:%lu:%lu:%s/%s:::",
			user, user, (unsigned long) indimailuid,
			(unsigned long) indimailgid, DOMAINDIR, user);
		snprintf(tmpstr2, MAX_BUFF, "+%s-:%s:%lu:%lu:%s/%s:-::", 
			user, user, (unsigned long) indimailuid,
			(unsigned long) indimailgid, DOMAINDIR, user);
	} else {
		snprintf(tmpstr1, MAX_BUFF, "=%s:%s:%lu:%lu:%s/%s/%s:::",
			user, user, (unsigned long) indimailuid,
			(unsigned long) indimailgid, DOMAINDIR, dir, user);
		snprintf(tmpstr2, MAX_BUFF, "+%s-:%s:%lu:%lu:%s/%s/%s:-::", 
			user, user, (unsigned long) indimailuid,
			(unsigned long) indimailgid, DOMAINDIR, dir, user);
	}
	if(update_file(filename, tmpstr1, INDIMAIL_QMAIL_MODE) || update_file(filename, tmpstr2, INDIMAIL_QMAIL_MODE))
		return(-1);
	update_newu();
	return (0);
}

void
getversion_add_user_assign_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
