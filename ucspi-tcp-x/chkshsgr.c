/*
 * $Log: chkshsgr.c,v $
 * Revision 1.3  2020-08-03 17:28:41+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.2  2007-06-10 10:12:16+05:30  Cprogrammer
 * datatype changed to gid_t
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
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
	if (!getgroups(1, (gid_t *) x) && setgroups(1, (gid_t *) x) == -1)
		_exit(1);
	_exit(0);
	/*- Not reached */
}
