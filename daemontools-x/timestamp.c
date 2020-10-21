/*
 * $Log: timestamp.c,v $
 * Revision 1.2  2004-10-22 20:31:47+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-19 18:54:00+05:30  Cprogrammer
 * Initial revision
 *
 */

#include "taia.h"
#include "timestamp.h"

static char     hex[16] = "0123456789abcdef";

void
timestamp(char s[TIMESTAMP])
{
	struct taia     now;
	char            nowpack[TAIA_PACK];
	int             i;

	taia_now(&now);
	taia_pack(nowpack, &now);

	s[0] = '@';
	for (i = 0; i < 12; ++i)
	{
		s[i * 2 + 1] = hex[(nowpack[i] >> 4) & 15];
		s[i * 2 + 2] = hex[nowpack[i] & 15];
	}
}

void
getversion_timestamp_c()
{
	static char    *x = "$Id: timestamp.c,v 1.2 2004-10-22 20:31:47+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
