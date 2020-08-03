/*
 * $Log: dns_domain.c,v $
 * Revision 1.2  2005-06-10 09:04:45+05:30  Cprogrammer
 * fixed bug with dns_domain_suffix()
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <error.h>
#include <alloc.h>
#include <case.h>
#include <byte.h>
#include "dns.h"

unsigned int
dns_domain_length(char *dn)
{
	char           *x;
	unsigned char   c;

	x = dn;
	while ((c = *x++))
		x += (unsigned int) c;
	return x - dn;
}

void
dns_domain_free(char **out)
{
	if (*out)
	{
		alloc_free(*out);
		*out = 0;
	}
}

int
dns_domain_copy(char **out, char *in)
{
	unsigned int    len;
	char           *x;

	len = dns_domain_length(in);
	x = alloc(len);
	if (!x)
		return 0;
	byte_copy(x, len, in);
	if (*out)
		alloc_free(*out);
	*out = x;
	return 1;
}

int
dns_domain_equal(char *dn1, char *dn2)
{
	unsigned int    len;

	len = dns_domain_length(dn1);
	if (len != dns_domain_length(dn2))
		return 0;
	if (case_diffb(dn1, len, dn2))
		return 0;				/*- safe since 63 < 'A' */
	return 1;
}

int
dns_domain_suffix(char *big, char *little)
{
	unsigned char   c;

	for (;;)
	{
		if (dns_domain_equal(big,little))
			return 1;
		if (!(c = *big++))
			return 0;
		big += c;
	}
}

unsigned int
dns_domain_suffixpos(char *big, char *little)
{
	char           *orig = big;
	unsigned char   c;

	for (;;)
	{
		if (dns_domain_equal(big,little))
			return big - orig;
		c = *big++;
		if (!c)
			return 0;
		big += c;
	}
}
