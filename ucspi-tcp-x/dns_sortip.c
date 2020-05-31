/*
 * $Log: dns_sortip.c,v $
 * Revision 1.2  2005-06-10 12:10:04+05:30  Cprogrammer
 * adeed ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "byte.h"
#include "dns.h"
#ifdef IPV6

/*
 * XXX: sort servers by configurable notion of closeness? 
 * XXX: pay attention to competence of each server? 
 */

void
dns_sortip6(char *s, unsigned int n)
{
	unsigned int    i;
	char            tmp[16];

	n >>= 4;
	while (n > 1)
	{
		i = dns_random(n);
		--n;
		byte_copy(tmp, 16, s + (i << 4));
		byte_copy(s + (i << 4), 16, s + (n << 4));
		byte_copy(s + (n << 4), 16, tmp);
	}
}
#else

/*- XXX: sort servers by configurable notion of closeness?  */
/*- XXX: pay attention to competence of each server?  */

void
dns_sortip(char *s, unsigned int n)
{
	unsigned int    i;
	char            tmp[4];

	n >>= 2;
	while (n > 1)
	{
		i = dns_random(n);
		--n;
		byte_copy(tmp, 4, s + (i << 2));
		byte_copy(s + (i << 2), 4, s + (n << 2));
		byte_copy(s + (n << 2), 4, tmp);
	}
}
#endif
