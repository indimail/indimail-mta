/*
 * $Log: mess822_addr.c,v $
 * Revision 1.4  2011-05-07 15:58:45+05:30  Cprogrammer
 * removed unused variable
 *
 * Revision 1.3  2004-10-22 20:27:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-16 01:21:05+05:30  Cprogrammer
 * comment placement changed
 *
 * Revision 1.1  2004-01-04 23:15:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "mess822.h"
#include "str.h"

static stralloc tokens = { 0 };
static stralloc comment = { 0 };
static stralloc addr = { 0 };
/*-
 * 0: no address; 
 * 1: address without @;
 * 2: address with @ 
 */
static int      state;

static int
docomment(out)
	stralloc       *out;
{
	int             i;
	int             j;
	char            ch;

	for (j = i = 0; j < comment.len; ++j)
	{
		ch = comment.s[j];
		if (ch == ' ')
		{
			if (!i || (comment.s[i - 1] == ' '))
				continue;
		}
		comment.s[i++] = ch;
	}
	while (i && (comment.s[i - 1] == ' '))
		--i;
	comment.len = 0;
	if (i)
	{
		if (!stralloc_0(out))
			return 0;
		if (!stralloc_catb(out, comment.s, i))
			return 0;
		if (!stralloc_append(out, "("))
			return 0;
	}
	return 1;
}

static int
doit(out)
	stralloc       *out;
{
	if (!state)
		return 1;
	if (!stralloc_0(out))
		return 0;
	if (state == 1)
	{
		if (!stralloc_append(out, "@"))
			return 0;
	}
	if (!stralloc_catb(out, addr.s, addr.len))
		return 0;
	if (!stralloc_append(out, "+"))
		return 0;
	if (!docomment(out))
		return 0;
	state = 0;
	addr.len = 0;
	return 1;
}

static int
addcomment(tok)
	char           *tok;
{
	int             i;

	if (*tok == ',')
		return 1;
	if (*tok == ':')
		return 1;
	if (*tok == ';')
		return 1;
	if (*tok == '<')
		return 1;
	if (*tok == '>')
		return 1;
	if ((*tok == '=') || (*tok == '('))
		++tok;
	i = str_len(tok);
	while (i--)
		if (!stralloc_append(&comment, tok + i))
			return 0;
	return 1;
}

static int
addaddr(tok)
	char           *tok;
{
	int             i;

	if ((*tok != '=') && (*tok != '.') && (*tok != '@'))
		return addcomment(tok);
	if (!state)
		state = 1;
	if (*tok == '@')
		state = 2;
	if (*tok == '=')
		++tok;
	i = str_len(tok);
	while (i--)
	{
		if (!stralloc_append(&addr, tok + i))
			return 0;
	}
	return 1;
}

int
mess822_addrlist(out, in)
	stralloc       *out;
	char           *in;
{
	int             flagwordok = 1;
	int             flagphrase = 0;
	int             j;
	int             i;
	char            ch;

	if (!mess822_token(&tokens, in))
		return 0;
	if (!stralloc_copys(out, ""))
		return 0;
	if (!stralloc_copys(&comment, ""))
		return 0;
	if (!stralloc_copys(&addr, ""))
		return 0;
	state = 0;
	j = tokens.len;
	while (j)
	{
		while (j--)
		{
			if (!j || !tokens.s[j - 1])
				break;
		}
		ch = tokens.s[j];
		if (flagphrase)
		{
			if ((ch != ',') && (ch != ';') && (ch != ':') && (ch != '>'))
			{
				if (!addcomment(tokens.s + j))
					return 0;
				continue;
			}
			if (!doit(out))
				return 0;
			flagphrase = 0;
			flagwordok = 1;
		}
		switch (tokens.s[j])
		{
		case ' ':
		case '\t':
			if (!addcomment(" "))
				return 0;
			break;
		case ';':
		case ',':
			if (!doit(out))
				return 0;
			if (!docomment(out))
				return 0;
			flagwordok = 1;
			break;

		case '(':
			if (!addcomment(" "))
				return 0;
			if (!addcomment(tokens.s + j))
				return 0;
			if (!addcomment(" "))
				return 0;
			break;
		case '=':
			if (!flagwordok)
				if (!doit(out))
					return 0;
			if (!addaddr(tokens.s + j))
				return 0;
			flagwordok = 0;
			break;
		case '>':
			if (!doit(out))
				return 0;
			if (!state)
				state = 1;		/*- <> is an address */
			for (;;)
			{
				if (!addaddr(tokens.s + j))
					return 0;
				if (!j)
					break;
				while (j--)
				{
					if (!j || !tokens.s[j - 1])
						break;
				}
				if (tokens.s[j] == ':')
					break;
				if (tokens.s[j] == '<')
					break;
			}
			if (tokens.s[j] == ':')
			{
				for (;;)
				{
					if (!addcomment(tokens.s + j))
						return 0;
					if (!j)
						break;
					while (j--)
					{
						if (!j || !tokens.s[j - 1])
							break;
					}
					if (tokens.s[j] == '<')
						break;
				}
			} /*- fall through */
		case ':':
			flagphrase = 1;
			break;
		default:
			if (!addaddr(tokens.s + j))
				return 0;
			flagwordok = 1;
			break;
		}
	}
	if (!doit(out))
		return 0;
	if (!docomment(out))
		return 0;
	i = 0;
	j = out->len - 1;
	while (i < j)
	{
		ch = out->s[i];
		out->s[i] = out->s[j];
		out->s[j] = ch;
		++i;
		--j;
	}
	return 1;
}

void
getversion_mess822_addr_c()
{
	static char    *x = "$Id: mess822_addr.c,v 1.4 2011-05-07 15:58:45+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
