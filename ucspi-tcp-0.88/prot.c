/*
 * $Log: prot.c,v $
 * Revision 1.2  2007-06-10 10:15:28+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "hasshsgr.h"
#include "prot.h"
#include <unistd.h>
#include <grp.h>

int
prot_gid(unsigned int gid)
{
#ifdef HASSHORTSETGROUPS
	short           x[2];
	x[0] = gid;
	x[1] = 73;					/*- catch errors */
	if (setgroups(1, x) == -1)
		return -1;
#else
	if (setgroups(1, &gid) == -1)
		return -1;
#endif
	return setgid(gid);			/*- _should_ be redundant, but on some systems it isn't */
}

int
prot_uid(int uid)
{
	return setuid(uid);
}
