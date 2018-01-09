/*
 * $Log: caltime_fmt.c,v $
 * Revision 1.2  2004-10-22 20:23:11+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "caldate.h"
#include "caltime.h"

unsigned int
caltime_fmt(s, ct)
	char           *s;
	struct caltime *ct;
{
	unsigned int    result;
	long            x;

	result = caldate_fmt(s, &ct->date);

	if (s)
	{
		s += result;

		x = ct->hour;
		s[0] = ' ';
		s[2] = '0' + (x % 10);
		x /= 10;
		s[1] = '0' + (x % 10);
		s += 3;

		x = ct->minute;
		s[0] = ':';
		s[2] = '0' + (x % 10);
		x /= 10;
		s[1] = '0' + (x % 10);
		s += 3;

		x = ct->second;
		s[0] = ':';
		s[2] = '0' + (x % 10);
		x /= 10;
		s[1] = '0' + (x % 10);
		s += 3;

		s[0] = ' ';
		x = ct->offset;
		if (x < 0)
		{
			s[1] = '-';
			x = -x;
		} else
			s[1] = '+';

		s[5] = '0' + (x % 10);
		x /= 10;
		s[4] = '0' + (x % 6);
		x /= 6;
		s[3] = '0' + (x % 10);
		x /= 10;
		s[2] = '0' + (x % 10);
	}

	return result + 15;
}

void
getversion_caltime_fmt_c()
{
	static char    *x = "$Id: caltime_fmt.c,v 1.2 2004-10-22 20:23:11+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
