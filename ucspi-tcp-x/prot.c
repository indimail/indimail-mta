/*
 * $Log: prot.c,v $
 * Revision 1.3  2017-04-05 03:14:57+05:30  Cprogrammer
 * use syscall for setgroups(), setgid(), setuid() to avoid NPTL wrapper for these functions
 * probable BUG with this functions with dlmopen() in current kernel
 *
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

#include <syscall.h>
#define SYS_SETGROUPS(NGRUP,MYGIDSET) syscall(SYS_setgroups, NGRUP, MYGIDSET)
#define SYS_SETGID(MYGID) syscall(SYS_setgid, MYGID)
#define SYS_SETUID(MYUID) syscall(SYS_setuid, MYUID)

int
prot_gid(gid_t gid)
{
#ifdef HASSHORTSETGROUPS
	short           x[2];
	x[0] = gid;
	x[1] = 73; /*- catch errors */
	if (setgroups(1, x) == -1)
		return -1;
#else
	if (SYS_SETGROUPS(1, &gid) == -1)
		return -1;
#endif
	return SYS_SETGID(gid);	/*- _should_ be redundant, but on some systems it isn't */
}

int
prot_uid(uid_t uid)
{
	return SYS_SETUID(uid);
}
