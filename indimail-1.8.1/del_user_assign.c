/*
 * $Log: del_user_assign.c,v $
 * Revision 2.3  2009-01-15 08:55:23+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.2  2005-12-29 22:43:42+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2004-05-17 14:00:54+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.3  2001-11-24 12:18:50+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:13+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: del_user_assign.c,v 2.3 2009-01-15 08:55:23+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * remove a local user from the users/assign file and recompile
 */
int
del_user_assign(char *user)
{
	char            tmpbuf1[MAX_BUFF], tmpbuf2[MAX_BUFF], fname[MAX_BUFF];
	char           *qmaildir;

	if(indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	snprintf(tmpbuf1, MAX_BUFF, "=%s:%s:%lu:%lu:%s/users/%s:::", user, user,
			(unsigned long) indimailuid,
			(unsigned long) indimailgid, INDIMAILDIR, user);
	snprintf(tmpbuf2, MAX_BUFF, "+%s-:%s:%lu:%lu:%s/users/%s:-::", user, user,
			(unsigned long) indimailuid,
			(unsigned long) indimailgid, INDIMAILDIR, user);
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	snprintf(fname, MAX_BUFF, "%s/users/assign", qmaildir);
	if(remove_line(tmpbuf1, fname, 0, INDIMAIL_QMAIL_MODE) == -1 || remove_line(tmpbuf2, fname, 0, INDIMAIL_QMAIL_MODE) == -1)
		return(-1);
	update_newu();
	return (0);
}

void
getversion_del_user_assign_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
