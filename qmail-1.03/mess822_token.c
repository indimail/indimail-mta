/*
 * $Log: mess822_token.c,v $
 * Revision 1.2  2004-10-22 20:27:32+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-04 23:18:10+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "mess822.h"

int
mess822_token(out, in)
	stralloc       *out;
	char           *in;
{
	char            ch;
	int             level;

	if (!stralloc_copys(out, ""))
		return 0;
	for (;;)
	{
		switch (ch = *in++)
		{
		case 0:
			return 1;

		case '"':
			if (!stralloc_append(out, "="))
				return 0;
			while (*in)
			{
				ch = *in++;
				if (ch == '"')
					break;
				if (ch == '\\')
					if (*in)
						ch = *in++;
				if (!stralloc_append(out, &ch))
					return 0;
			}
			if (!stralloc_0(out))
				return 0;
			break;

		case '[':
			if (!stralloc_append(out, "="))
				return 0;
			if (!stralloc_append(out, "["))
				return 0;
			while (*in)
			{
				ch = *in++;
				if (ch == ']')
					break;
				if (ch == '\\')
					if (*in)
						ch = *in++;
				if (!stralloc_append(out, &ch))
					return 0;
			}
			if (!stralloc_append(out, "]"))
				return 0;
			if (!stralloc_0(out))
				return 0;
			break;

		case '(':
			if (!stralloc_append(out, "("))
				return 0;
			level = 1;
			while (*in)
			{
				ch = *in++;
				if (ch == ')')
				{
					--level;
					if (!level)
						break;
					if (!stralloc_append(out, ")"))
						return 0;
					continue;
				}
				if (ch == '(')
				{
					if (level)
						if (!stralloc_append(out, "("))
							return 0;
					++level;
					continue;
				}
				if (ch == '\\')
					if (*in)
						ch = *in++;
				if (!stralloc_append(out, &ch))
					return 0;
			}
			if (!stralloc_0(out))
				return 0;
			break;

		case '<':
		case '>':
		case ',':
		case ';':
		case ':':
		case '@':
		case '.':
		case ' ':
		case '\t':
			if (!stralloc_append(out, &ch))
				return 0;
			if (!stralloc_0(out))
				return 0;
			break;

		default:
			if (!stralloc_append(out, "="))
				return 0;

			for (;;)
			{
				if (ch == '\\')
					if (*in)
						ch = *in++;
				if (!stralloc_append(out, &ch))
					return 0;
				ch = *in;
				if (!ch)
					break;
				if (ch == '"')
					break;
				if (ch == '[')
					break;
				if (ch == '(')
					break;
				if (ch == '<')
					break;
				if (ch == '>')
					break;
				if (ch == ',')
					break;
				if (ch == ';')
					break;
				if (ch == ':')
					break;
				if (ch == '@')
					break;
				if (ch == '.')
					break;
				if (ch == ' ')
					break;
				if (ch == '\t')
					break;
				++in;
			}
			if (!stralloc_0(out))
				return 0;
			break;
		}
	} /*- for (;;) */
}

void
getversion_mess822_token_c()
{
	static char    *x = "$Id: mess822_token.c,v 1.2 2004-10-22 20:27:32+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
