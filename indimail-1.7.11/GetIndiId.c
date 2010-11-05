/*
 * $Log: GetIndiId.c,v $
 * Revision 1.3  2001-11-24 12:16:57+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:16+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <pwd.h>

#ifndef	lint
static char     sccsid[] = "$Id: GetIndiId.c,v 1.3 2001-11-24 12:16:57+05:30 Cprogrammer Stab mbhangui $";
#endif

int
GetIndiId(uid_t *uid, gid_t *gid)
{
	struct passwd  *pw;
	static uid_t    suid = -1;
	static gid_t    sgid = -1;

	if(suid != -1 && sgid != -1)
	{
		*uid = suid;
		*gid = sgid;
		return(0);
	}
	if(!(pw = getpwnam(INDIUSER)))
	{
		fprintf(stderr, "getpwnam failed for user %s\n", INDIUSER);
		return(-1);
	}
	*uid = suid = pw->pw_uid;
	*gid = sgid = pw->pw_gid;
	return(0);
}

void
getversion_GetIndiId_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
