/*
 * $Log: tryshsgr.c,v $
 * Revision 1.2  2024-05-09 22:58:59+05:30  Cprogrammer
 * added unistd.h
 *
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif
#include <unistd.h>

int setgroups(size_t , const gid_t *);

int
main()
{
	short           x[4];

	x[0] = x[1] = 1;
	if (getgroups(1, x) == 0)
		if (setgroups(1, x) == -1)
			_exit(1);

	if (getgroups(1, x) == -1)
		_exit(1);
	if (x[1] != 1)
		_exit(1);
	x[1] = 2;
	if (getgroups(1, x) == -1)
		_exit(1);
	if (x[1] != 2)
		_exit(1);
	_exit(0);
}
