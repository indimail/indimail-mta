/*
 * $Log: rules.c,v $
 * Revision 1.6  2020-08-03 17:25:50+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.5  2016-01-08 17:57:16+05:30  Cprogrammer
 * fix bug with tcprules when compact ipv6 notation was given in tcp.smtp
 *
 * Revision 1.4  2013-08-06 11:03:48+05:30  Cprogrammer
 * Jens Wehrenbrecht's IPv4 CIDR extension
 * Li Minh Bui's IPv6 support for compactified IPv6 addresses and CIDR notation support.
 *
 * Revision 1.3  2009-08-13 14:41:49+05:30  Cprogrammer
 * beautified code
 *
 * Revision 1.2  2005-06-10 09:13:06+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <alloc.h>
#include <stralloc.h>
#include <open.h>
#include <cdb.h>
#include <str.h>
#include <byte.h>
#include "rules.h"
#include "ip4_bit.h"
#include "ip6.h"

stralloc        rules_name = { 0 };
stralloc        ipstring = { 0 };

static struct cdb c;

static int
dorule(void     (*callback) (char *, unsigned int))
{
	char           *data;
	unsigned int    datalen;

	switch (cdb_find(&c, rules_name.s, rules_name.len))
	{
	case -1:
		return -1;
	case 0:
		return 0;
	}

	datalen = cdb_datalen(&c);
	if (!(data = alloc(datalen)))
		return -1;
	if (cdb_read(&c, data, datalen, cdb_datapos(&c)) == -1) {
		alloc_free(data);
		return -1;
	}
	callback(data, datalen);
	alloc_free(data);
	return 1;
}

static int
doit(void       (*callback) (char *, unsigned int), char *ip, char *host, char *info)
{
	int             r, p;
	int             ipv6 = str_len(ip) - byte_chr(ip, str_len(ip), ':');

	if (info) {	/*- 1. info@ip */
		if (!stralloc_copys(&rules_name, info))
			return -1;
		if (!stralloc_cats(&rules_name, "@"))
			return -1;
#ifdef IPV6
		if (ipv6) { 
			if(ip6_expandaddr(ip, &ipstring) == 1 && 
				!stralloc_catb(&rules_name, ipstring.s, ipstring.len))
				return -1;
		} else
		if (!stralloc_cats(&rules_name, ip))
			return -1;
#else
		if (!stralloc_cats(&rules_name, ip))
			return -1;
#endif
		if ((r = dorule(callback)))
			return r;
		if (host) {	/*- 2. info@=host */
			if (!stralloc_copys(&rules_name, info))
				return -1;
			if (!stralloc_cats(&rules_name, "@="))
				return -1;
			if (!stralloc_cats(&rules_name, host))
				return -1;
			if ((r = dorule(callback)))
				return r;
		}
	}
#ifdef IPV6
	if (ipv6) { /*- 3. IPv6/IPv4 */
    	if (ip6_expandaddr(ip, &ipstring) == 1) {		
        	if (!stralloc_copyb(&rules_name, ipstring.s, ipstring.len))
				return -1;
        	if ((r = dorule(callback)))
        		return r;
    	}
	}
	if (!stralloc_copys(&rules_name, ip))
		return -1;
	if ((r = dorule(callback)))
		return r;
#else
	if (!stralloc_copys(&rules_name, ip))
		return -1;
	if ((r = dorule(callback)))
		return r;
#endif
	if (host) {	/*- 4. =host */
		if (!stralloc_copys(&rules_name, "="))
			return -1;
		if (!stralloc_cats(&rules_name, host))
			return -1;
		if ((r = dorule(callback)))
			return r;
	}

#ifdef IPV6	/*- 5. IPv4 class-based */
	if (!ipv6) {
		if (!stralloc_copys(&rules_name, ip))
			return -1;
		while (rules_name.len > 0) {
			if (ip[rules_name.len - 1] == '.' || ip[rules_name.len - 1] == ':') {
				if ((r = dorule(callback)))
					return r;
			}
			--rules_name.len;
		}
	}
	if (ipv6) {	/*- 6. IPv6/IPv4 CIDR */
		if (ip6tobitstring(rules_name.s, &ipstring, 128) == 1) {
			for (p = 129; p > 1; p--) {
				if (!stralloc_copys(&rules_name, "^"))
					return -1;
				if (!stralloc_catb(&rules_name, ipstring.s, p))
					return -1;
				if ((r = dorule(callback)))
					return r;
			}
		}
	} else {
		if (!stralloc_copys(&rules_name, ip))
			return -1;
		if (getaddressasbit(ip, 32, &ipstring) != -1) {
			for (p = 33; p > 1; p--) {
				if (!stralloc_copys(&rules_name, "_"))
					return -1;
				if (!stralloc_catb(&rules_name, ipstring.s, p))
					return -1;
				if ((r = dorule(callback)))
					return r;
			}
		}
	}
#else
	if (!stralloc_copys(&rules_name, ip))
		return -1;
	while (rules_name.len > 0) {
		if (ip[rules_name.len - 1] == '.') {
			if ((r = dorule(callback)))
				return r;
		}
		--rules_name.len;
	}
#endif
	if (host) {	/*- 7. =host. */
		while (*host) {
			if (*host == '.') {
				if (!stralloc_copys(&rules_name, "="))
					return -1;
				if (!stralloc_cats(&rules_name, host))
					return -1;
				if ((r = dorule(callback)))
					return r;
			}
			++host;
		}
		if (!stralloc_copys(&rules_name, "="))	/*- 8. = rule */
			return -1;
		if ((r = dorule(callback)))
			return r;
	}
	rules_name.len = 0;
	return dorule(callback);
}

int
rules(void      (*callback) (char *, unsigned int), int fd, char *ip, char *host, char *info)
{
	int             r;
	cdb_init(&c, fd);
	r = doit(callback, ip, host, info);
	cdb_free(&c);
	return r;
}
