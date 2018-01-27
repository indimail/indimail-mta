/*
 * $Log: del_domain_assign.c,v $
 * Revision 2.4  2016-05-18 11:44:02+05:30  Cprogrammer
 * use ASSIGNDIR for users/assign
 *
 * Revision 2.3  2009-01-15 08:55:15+05:30  Cprogrammer
 * change for once_only flag in remove_line
 *
 * Revision 2.2  2005-12-29 22:42:39+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2004-05-17 14:00:49+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.3  2001-11-24 12:18:48+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:11+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <sys/stat.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: del_domain_assign.c,v 2.4 2016-05-18 11:44:02+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * delete a domain from the usrs/assign file
 * input : lots ;)
 * output : 0 = success
 *          less than error = failure
 *
 */
int
del_domain_assign(char *domain, char *dir, gid_t uid,
				  gid_t gid)
{
	char            tmpstr[MAX_BUFF], fname[MAX_BUFF], *assigndir;

	snprintf(tmpstr, MAX_BUFF, "+%s-:%s:%lu:%lu:%s:-::", domain,
		domain, (unsigned long) uid, (unsigned long) gid, dir);
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	snprintf(fname, MAX_BUFF, "%s/assign", assigndir);
	if(remove_line(tmpstr, fname, 0, INDIMAIL_QMAIL_MODE) == -1)
		return(-1);
	update_newu();
	return (0);
}

void
getversion_del_domain_assign_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
