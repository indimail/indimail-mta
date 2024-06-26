/*
 * $Log: dns_rcrw.c,v $
 * Revision 1.4  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2020-08-03 17:23:10+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.2  2005-06-10 09:12:13+05:30  Cprogrammer
 * code beautified
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <taia.h>
#include <env.h>
#include <byte.h>
#include <str.h>
#include <unistd.h>
#include "openreadclose.h"
#include "dns.h"

static stralloc data = { 0 };

static int
init(stralloc * rules)
{
	char            host[256];
	const char     *x;
	int             i, j, k;

	if (!stralloc_copys(rules, ""))
		return -1;
	if (!(x = env_get("DNSREWRITEFILE")))
		x = "/etc/dnsrewrite";
	if ((i = openreadclose(x, &data, 64)) == -1)
		return -1;
	if (i) {
		if (!stralloc_append(&data, "\n"))
			return -1;
		i = 0;
		for (j = 0; j < data.len; ++j) {
			if (data.s[j] == '\n') {
				if (!stralloc_catb(rules, data.s + i, j - i))
					return -1;
				while (rules->len) {
					if (rules->s[rules->len - 1] != ' ' && rules->s[rules->len - 1] != '\t' && rules->s[rules->len - 1] != '\r')
						break;
					--rules->len;
				}
				if (!stralloc_0(rules))
					return -1;
				i = j + 1;
			}
		}
		return 0;
	}
	if ((x = env_get("LOCALDOMAIN"))) {
		if (!stralloc_copys(&data, x))
			return -1;
		if (!stralloc_append(&data, " "))
			return -1;
		if (!stralloc_copys(rules, "?:"))
			return -1;
		i = 0;
		for (j = 0; j < data.len; ++j) {
			if (data.s[j] == ' ') {
				if (!stralloc_cats(rules, "+."))
					return -1;
				if (!stralloc_catb(rules, data.s + i, j - i))
					return -1;
				i = j + 1;
			}
		}
		if (!stralloc_0(rules))
			return -1;
		if (!stralloc_cats(rules, "*.:"))
			return -1;
		if (!stralloc_0(rules))
			return -1;
		return 0;
	}
	if ((i = openreadclose("/etc/resolv.conf", &data, 64)) == -1)
		return -1;
	if (i) {
		if (!stralloc_append(&data, "\n"))
			return -1;
		i = 0;
		for (j = 0; j < data.len; ++j) {
			if (data.s[j] == '\n') {
				if (byte_equal("search ", 7, data.s + i) || byte_equal("search\t", 7, data.s + i) ||
					byte_equal("domain ", 7, data.s + i) || byte_equal("domain\t", 7, data.s + i)) {
					if (!stralloc_copys(rules, "?:"))
						return -1;
					i += 7;
					while (i < j) {
						k = byte_chr(data.s + i, j - i, ' ');
						k = byte_chr(data.s + i, k, '\t');
						if (!k) {
							++i;
							continue;
						}
						if (!stralloc_cats(rules, "+."))
							return -1;
						if (!stralloc_catb(rules, data.s + i, k))
							return -1;
						i += k;
					}
					if (!stralloc_0(rules))
						return -1;
					if (!stralloc_cats(rules, "*.:"))
						return -1;
					if (!stralloc_0(rules))
						return -1;
					return 0;
				}
				i = j + 1;
			}
		}
	}
	host[0] = 0;
	if (gethostname(host, sizeof host) == -1)
		return -1;
	host[(sizeof host) - 1] = 0;
	i = str_chr(host, '.');
	if (host[i]) {
		if (!stralloc_copys(rules, "?:"))
			return -1;
		if (!stralloc_cats(rules, host + i))
			return -1;
		if (!stralloc_0(rules))
			return -1;
	}
	if (!stralloc_cats(rules, "*.:"))
		return -1;
	if (!stralloc_0(rules))
		return -1;
	return 0;
}

static int      ok = 0;
static unsigned int uses;
static struct taia deadline;
static stralloc rules = { 0 };	/*- defined if ok */

int
dns_resolvconfrewrite(stralloc *out)
{
	struct taia     now;

	taia_now(&now);
	if (taia_less(&deadline, &now))
		ok = 0;
	if (!uses)
		ok = 0;
	if (!ok) {
		if (init(&rules) == -1)
			return -1;
		taia_uint(&deadline, 600);
		taia_add(&deadline, &now, &deadline);
		uses = 10000;
		ok = 1;
	}
	--uses;
	if (!stralloc_copy(out, &rules))
		return -1;
	return 0;
}
