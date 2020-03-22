/*
 * $Log: rewritehost.c,v $
 * Revision 1.2  2004-10-22 20:30:00+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "stralloc.h"
#include "byte.h"
#include "str.h"
#include "case.h"
#include "rewritehost.h"

static stralloc work = { 0 };

static int
doit(rule)
	char           *rule;
{
	int             colon;
	char            ch;
	int             prefixlen;

	ch = *rule++;
	if ((ch != '?') && (ch != '=') && (ch != '*') && (ch != '-'))
		return 1;
	colon = str_chr(rule, ':');
	if (!rule[colon])
		return 1;
	if (work.len < colon)
		return 1;

	prefixlen = work.len - colon;
	if ((ch == '=') && prefixlen)
		return 1;
	if (case_diffb(rule, colon, work.s + prefixlen))
		return 1;

	if (ch == '?')
	{
		if (byte_chr(work.s, prefixlen, '.') < prefixlen)
			return 1;
		if (byte_chr(work.s, prefixlen, '/') < prefixlen)
			return 1;
		if (byte_chr(work.s, prefixlen, '[') < prefixlen)
			return 1;
		if (byte_chr(work.s, prefixlen, ']') < prefixlen)
			return 1;
	}

	work.len = prefixlen;
	if (ch == '-')
		work.len = 0;

	return stralloc_cats(&work, rule + colon + 1);
}

static int
appendwork(out, rules)
	stralloc       *out;
	stralloc       *rules;
{
	int             i;
	int             j;

	for (j = i = 0; j < rules->len; ++j)
		if (!rules->s[j])
		{
			if (!doit(rules->s + i))
				return 0;
			i = j + 1;
		}
	return stralloc_cat(out, &work);
}

static int
appendaddr(out, in, len, rules)
	stralloc       *out;
	char           *in;
	unsigned int    len;
	stralloc       *rules;
{
	int             at;

	at = byte_chr(in, len, '@');
	if (!at)
		if (len <= 1)
			return 1;
	if (!stralloc_catb(out, in, at))
		return 0;
	if (!stralloc_append(out, "@"))
		return 0;
	if (at < len)
		++at;
	if (!stralloc_copyb(&work, in + at, len - at))
		return 0;
	return appendwork(out, rules);
}

int
rewritehost(out, in, len, rules)
	stralloc       *out;
	char           *in;
	unsigned int    len;
	stralloc       *rules;
{
	if (!stralloc_copys(out, ""))
		return 0;
	if (!stralloc_copyb(&work, in, len))
		return 0;
	return appendwork(out, rules);
}

int
rewritehost_addr(out, in, len, rules)
	stralloc       *out;
	char           *in;
	unsigned int    len;
	stralloc       *rules;
{
	if (!stralloc_copys(out, ""))
		return 0;
	return appendaddr(out, in, len, rules);
}

int
rewritehost_list(out, in, len, rules)
	stralloc       *out;
	char           *in;
	unsigned int    len;
	stralloc       *rules;
{
	int             i;
	int             j;

	if (!stralloc_copys(out, ""))
		return 0;
	for (j = i = 0; j < len; ++j)
		if (!in[j])
		{
			if (in[i] == '+')
			{
				if (!stralloc_append(out, "+"))
					return 0;
				if (!appendaddr(out, in + i + 1, j - i - 1, rules))
					return 0;
				if (!stralloc_0(out))
					return 0;
			} else
			if (!stralloc_catb(out, in + i, j - i + 1))
				return 0;
			i = j + 1;
		}
	return 1;
}

void
getversion_rewritehost_c()
{
	static char    *x = "$Id: rewritehost.c,v 1.2 2004-10-22 20:30:00+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
