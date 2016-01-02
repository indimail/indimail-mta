/*-
 * $Log: tai64n_decode.c,v $
 * Revision 1.1  2016-01-02 19:22:04+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "tai2.h"

static int
hex2int(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

tai            *
tai64n_decode(stralloc *str, char **endptr)
{
	static struct tai t;
	char           *ptr;

	ptr = str->s;
	if (*ptr++ != '@')
		return 0;
	if (str->len < 25)
		return 0;
/*
 * Check if the line is within range 
 */
	if (ptr[0] != '4' || ptr[1] != '0' || ptr[2] != '0' || ptr[3] != '0' || ptr[4] != '0' || ptr[5] != '0' || ptr[6] != '0'
		|| ptr[7] != '0')
		return 0;
	t.seconds =
		hex2int(ptr[8]) << 28 | hex2int(ptr[9]) << 24 | hex2int(ptr[10]) << 20 | hex2int(ptr[11]) << 16 | hex2int(ptr[12]) << 12 |
		hex2int(ptr[13]) << 8 | hex2int(ptr[14]) << 4 | hex2int(ptr[15]);
	t.nanoseconds =
		hex2int(ptr[16]) << 28 | hex2int(ptr[17]) << 24 | hex2int(ptr[18]) << 20 | hex2int(ptr[19]) << 16 | hex2int(ptr[20]) << 12 |
		hex2int(ptr[21]) << 8 | hex2int(ptr[22]) << 4 | hex2int(ptr[23]);
	if (endptr)
		*endptr = ptr + 24;
	return &t;
}

void
getversion_tai64n_decode_c()
{
	static char    *x = "$Id: tai64n_decode.c,v 1.1 2016-01-02 19:22:04+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
