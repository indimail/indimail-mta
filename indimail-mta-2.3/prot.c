/*
 * $Log: prot.c,v $
 * Revision 1.4  2004-10-22 20:28:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:20:31+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
#include "hasshsgr.h"
#include "prot.h"

/*
 * XXX: there are more portability problems here waiting to leap out at me 
 */

int
prot_gid(gid)
	int             gid;
{
#ifdef HASSHORTSETGROUPS
	short           x[2];

	x[0] = gid;
	x[1] = 73;	/*- catch errors */
	if (setgroups(1, (gid_t *) x) == -1)
		return -1;
#else
	if (setgroups(1, (gid_t *) &gid) == -1)
		return -1;
#endif
	return setgid(gid);	/*- _should_ be redundant, but on some systems it isn't */
}

int
prot_uid(uid)
	int             uid;
{
	return setuid(uid);
}

void
getversion_prot_c()
{
	static char    *x = "$Id: prot.c,v 1.4 2004-10-22 20:28:03+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
