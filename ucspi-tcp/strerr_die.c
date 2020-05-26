/*
 * $Log: strerr_die.c,v $
 * Revision 1.2  2016-05-23 04:43:21+05:30  Cprogrammer
 * added two arguments to strerr_die(), strerr_warn()
 *
 * Revision 1.1  2003-12-31 19:47:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "buffer.h"
#include "exit.h"
#include "strerr.h"

void
strerr_warn(char *x1, char *x2, char *x3, char *x4, char *x5, char *x6, char *x7, char *x8, struct strerr *se)
{
	strerr_sysinit();

	if (x1)
		buffer_puts(buffer_2, x1);
	if (x2)
		buffer_puts(buffer_2, x2);
	if (x3)
		buffer_puts(buffer_2, x3);
	if (x4)
		buffer_puts(buffer_2, x4);
	if (x5)
		buffer_puts(buffer_2, x5);
	if (x6)
		buffer_puts(buffer_2, x6);
	if (x7)
		buffer_puts(buffer_2, x7);
	if (x8)
		buffer_puts(buffer_2, x8);

	while (se)
	{
		if (se->x)
			buffer_puts(buffer_2, se->x);
		if (se->y)
			buffer_puts(buffer_2, se->y);
		if (se->z)
			buffer_puts(buffer_2, se->z);
		se = se->who;
	}

	buffer_puts(buffer_2, "\n");
	buffer_flush(buffer_2);
}

void
strerr_die(int e, char *x1, char *x2, char *x3, char *x4, char *x5, char *x6, char *x7, char *x8, struct strerr *se)
{
	strerr_warn(x1, x2, x3, x4, x5, x6, x7, x8, se);
	_exit(e);
}
