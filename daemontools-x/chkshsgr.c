/*
 * $Log: chkshsgr.c,v $
 * Revision 1.6  2007-12-20 12:43:15+05:30  Cprogrammer
 * changed data type to gid_t
 *
 * Revision 1.5  2004-10-22 20:23:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:17:35+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <grp.h>
int
main()
{
	gid_t           x[4];

	x[0] = x[1] = 0;
	if (getgroups(1, (gid_t *) x) == 0 && setgroups(1, (gid_t *) x) == -1)
		_exit(1);
	return(0);
}

void
getversion_chkshsgr_c()
{
	static char    *x = "$Id: chkshsgr.c,v 1.6 2007-12-20 12:43:15+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
