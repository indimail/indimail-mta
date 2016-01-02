/*-
 * $Log: tai64n_encode.c,v $
 * Revision 1.1  2016-01-02 19:21:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "tai2.h"

static char     INT2HEX[16] = "0123456789abcdef";
#define int2hex(X) INT2HEX[(X)&0xf]

int
tai64n_encode(tai *t, char *buf)
{
	unsigned long   n = t->nanoseconds;
	unsigned long   s = t->seconds;
	char           *ptr;
	
	ptr = buf + 1 + 8 + 8 + 8 + 1;
	*--ptr = 0;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	n >>= 4;
	*--ptr = int2hex(n);
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	s >>= 4;
	*--ptr = int2hex(s);
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '0';
	*--ptr = '4';
	*--ptr = '@';
	return 0;
}

void
getversion_tai64n_encode_c()
{
	static char    *x = "$Id: tai64n_encode.c,v 1.1 2016-01-02 19:21:48+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
