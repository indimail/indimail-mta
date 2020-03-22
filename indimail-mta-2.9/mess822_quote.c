/*
 * $Log: mess822_quote.c,v $
 * Revision 1.2  2004-10-22 20:27:31+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-04 23:18:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "mess822.h"
#include "str.h"

static int
needquote(buf, len)
	char           *buf;
	int             len;
{
	int             i;
	char            ch;

	if (!len)
		return 1;
	if (buf[0] == '.')
		return 1;
	if (buf[len - 1] == '.')
		return 1;
	for (i = 0; i < len - 1; ++i)
	{
		if ((buf[i] == '.') && (buf[i + 1] == '.'))
			return 1;
	}
	for (i = 0; i < len; ++i)
	{
		ch = buf[i];
		if (ch < 33)
			return 1;
		if (ch > 126)
			return 1;
		if (ch == '@')
			return 1;
		if (ch == '<')
			return 1;
		if (ch == '>')
			return 1;
		if (ch == '[')
			return 1;
		if (ch == ']')
			return 1;
		if (ch == '(')
			return 1;
		if (ch == ')')
			return 1;
		if (ch == ',')
			return 1;
		if (ch == ';')
			return 1;
		if (ch == ':')
			return 1;
		if (ch == '"')
			return 1;
		if (ch == '\\')
			return 1;
	}
	return 0;
}

static int
doit(out, buf, len, pre, post)
	stralloc       *out;
	char           *buf;
	int             len;
	char           *pre;
	char           *post;
{
	char            ch;

	if (!stralloc_cats(out, pre))
		return 0;
	while (len--)
	{
		ch = *buf++;
		if (ch == '\n')
			ch = 0;
		if ((ch == 0) || (ch == '\r') || (ch == '"') || (ch == '\\') || (ch == '[') || (ch == ']'))
			if (!stralloc_append(out, "\\"))
				return 0;
		if (!stralloc_append(out, &ch))
			return 0;
	}
	if (!stralloc_cats(out, post))
		return 0;
	return 1;
}

int
mess822_quoteplus(out, addr, comment)
	stralloc       *out;
	char           *addr;
	char           *comment;
{
	int             i;
	char           *quote;
	int             flagempty;
	int             flagbracket;

	flagempty = 0;
	if (str_equal(addr, ""))
		flagempty = 1;
	if (str_equal(addr, "@"))
		flagempty = 1;
	flagbracket = flagempty;
	if (comment)
	{
		if (!doit(out, comment, str_len(comment), "\"", "\" "))
			return 0;
		flagbracket = 1;
	}
	if (flagbracket)
		if (!stralloc_cats(out, "<"))
			return 0;
	if (!flagempty)
	{
		i = str_rchr(addr, '@');
		quote = needquote(addr, i) ? "\"" : "";
		if (!doit(out, addr, i, quote, quote))
			return 0;
		addr += i;
		if (*addr == '@')
			++addr;
		i = str_len(addr);
		if (i)
		{
			if (!stralloc_append(out, "@"))
				return 0;
			quote = needquote(addr, i) ? "\"" : "";
			if (*quote && (i >= 2) && (addr[0] == '[') && (addr[i - 1] == ']'))
			{
				if (!doit(out, addr + 1, i - 2, "[", "]"))
					return 0;
			} else
			if (!doit(out, addr, i, quote, quote))
				return 0;
		}
	}
	if (flagbracket)
		if (!stralloc_cats(out, ">"))
			return 0;
	return 1;
}

int
mess822_quote(out, addr, comment)
	stralloc       *out;
	char           *addr;
	char           *comment;
{
	if (!stralloc_copys(out, ""))
		return 0;
	return mess822_quoteplus(out, addr, comment);
}

int
mess822_quotelist(out, in)
	stralloc       *out;
	stralloc       *in;
{
	int             i;
	int             j;
	int             comment;

	if (!stralloc_copys(out, ""))
		return 0;
	comment = 0;
	for (j = i = 0; j < in->len; ++j)
	{
		if (!in->s[j])
		{
			if (in->s[i] == '(')
			{
				if (comment)
					if (!doit(out, in->s + comment, str_len(in->s + comment), "\"", "\": ;,\n  "))
						return 0;
				comment = i + 1;
			} else
			if (in->s[i] == '+')
			{
				if (!mess822_quoteplus(out, in->s + i + 1, comment ? in->s + comment : (char *) 0))
					return 0;
				if (!stralloc_cats(out, ",\n  "))
					return 0;
				comment = 0;
			}
			i = j + 1;
		}
	}
	if (comment)
		if (!doit(out, in->s + comment, str_len(in->s + comment), "\"", "\": ;,\n  "))
			return 0;
	if (out->len && (out->s[out->len - 1] == ' '))
		--out->len;
	if (out->len && (out->s[out->len - 1] == ' '))
		--out->len;
	if (out->len && (out->s[out->len - 1] == '\n'))
		--out->len;
	if (out->len && (out->s[out->len - 1] == ','))
		--out->len;
	return 1;
}

void
getversion_mess822_quote_c()
{
	static char    *x = "$Id: mess822_quote.c,v 1.2 2004-10-22 20:27:31+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
