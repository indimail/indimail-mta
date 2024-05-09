/*
 * $Log: dns_dfd.c,v $
 * Revision 1.3  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2020-08-03 17:22:02+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <error.h>
#include <alloc.h>
#include <byte.h>
#include "dns.h"

int
dns_domain_fromdot(char **out, const char *buf, unsigned int n)
{
	char            label[63];
	unsigned int    labellen = 0;	/*- <= sizeof label */
	char            name[255];
	unsigned int    namelen = 0;	/*- <= sizeof name */
	char            ch;
	char           *x;

	errno = error_proto;

	for (;;) {
		if (!n)
			break;
		ch = *buf++;
		--n;
		if (ch == '.') {
			if (labellen) {
				if (namelen + labellen + 1 > sizeof name)
					return 0;
				name[namelen++] = labellen;
				byte_copy(name + namelen, labellen, label);
				namelen += labellen;
				labellen = 0;
			}
			continue;
		}
		if (ch == '\\') {
			if (!n)
				break;
			ch = *buf++;
			--n;
			if ((ch >= '0') && (ch <= '7')) {
				ch -= '0';
				if (n && (*buf >= '0') && (*buf <= '7')) {
					ch <<= 3;
					ch += *buf - '0';
					++buf;
					--n;
					if (n && (*buf >= '0') && (*buf <= '7')) {
						ch <<= 3;
						ch += *buf - '0';
						++buf;
						--n;
					}
				}
			}
		}
		if (labellen >= sizeof label)
			return 0;
		label[labellen++] = ch;
	}

	if (labellen) {
		if (namelen + labellen + 1 > sizeof name)
			return 0;
		name[namelen++] = labellen;
		byte_copy(name + namelen, labellen, label);
		namelen += labellen;
		labellen = 0;
	}

	if (namelen + 1 > sizeof name)
		return 0;
	name[namelen++] = 0;

	x = alloc(namelen);
	if (!x)
		return 0;
	byte_copy(x, namelen, name);

	if (*out)
		alloc_free(*out);
	*out = x;
	return 1;
}
