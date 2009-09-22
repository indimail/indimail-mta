/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
** $Id: unicode.c,v 1.9 2004/02/08 04:59:15 mrsam Exp $
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<errno.h>

const char *unicode_default_chset()
{
const char *p=UNICODECHARSET;

	if (unicode_find(p))
		return (p);

	return (unicode_ISO8859_1.chset);
}

const struct unicode_info *unicode_find(const char *chset)
{
char *p, *q;
int	i;

	if (!chset)	/* Default character set */
		return (&unicode_ISO8859_1);

	p=strdup(chset);
	if (!p)
		return (0);

	for (q=p; *q; q++)
		*q=toupper(*q);

	if (strcmp(unicode_ISO8859_1.chset, p) == 0)
	{
		free(p);
		return (&unicode_ISO8859_1);
	}

	for (i=0; unicode_chsetlist[i].chsetname; i++)
		if (strcmp(unicode_chsetlist[i].chsetname, p) == 0)
		{
			free(p);
			return (unicode_chsetlist[i].ptr);
		}
	free(p);
	return (0);
}

char *unicode_convert(const char *txt, const struct unicode_info *from,
		      const struct unicode_info *to)
{
	unicode_char *uc;
	char *s;
	int dummy;

	if (strcmp(from->chset, to->chset) == 0)	/* NOOP */
		return (strdup(txt));

	uc=(*from->c2u)(from, txt, &dummy);
	if (!uc)
	{
		if (dummy >= 0)
			errno=EINVAL;

		return (0);
	}

	s=(*to->u2c)(to, uc, &dummy);

	free(uc);

	if (dummy >= 0)
		errno=EINVAL;

	return (s);
}

char *unicode_convert_fromchset(const char *txt, const char *from,
		      const struct unicode_info *to)
{
	const struct unicode_info *fromu=unicode_find(from);

	if (!fromu)
	{
		errno=EINVAL;
		return (0);
	}
	return (unicode_convert(txt, fromu, to));
}

/*
** Convert being character sets, except ignore errors.
*/

struct ux_buf {
	char *buffer;
	size_t bufsize;
} ;

static int ux_alloc(struct ux_buf *p, size_t l)
{
	char *newbuf;

	if (l < p->bufsize)
		return (0);

	l += 64;

	newbuf=p->buffer ? realloc(p->buffer, l):malloc(l);

	if (!newbuf)
		return (-1);

	p->buffer=newbuf;
	p->bufsize=l;
	return (0);
}

char *unicode_xconvert(const char *txt, const struct unicode_info *from,
		       const struct unicode_info *to)
{
	unicode_char *uc;
	char *s, *cur_conv;
	int dummy, dummy2;
	struct ux_buf dst_str;

	char *orig_str=strdup(txt);

	if (!orig_str)
		return (0);

	if (strcmp(from->chset, to->chset) == 0)	/* NOOP */
		return (orig_str);

	dst_str.bufsize=0;
	dst_str.buffer=0;

	if (ux_alloc(&dst_str, strlen(txt)*2))
	{
		free(orig_str);
		return (NULL);
	}

	dst_str.buffer[0]=0;

	cur_conv=orig_str;

	while (*cur_conv)
	{
		size_t l;
		unicode_char *ucptr;

		l=strlen(cur_conv);

		if (from->flags & UNICODE_REPLACEABLE)
		{
			uc=(*from->c2u)(from, cur_conv, NULL);
			if (!uc)
			{
				free(orig_str);
				free(dst_str.buffer);
				return NULL;
			}
		}
		else
			uc=(*from->c2u)(from, cur_conv, &dummy);

		if (!uc)
		{
			char save_char;

			if (dummy < 0)
			{
				free(orig_str);
				free(dst_str.buffer);
				return (NULL);
			}

			/* Error converting original text to unicode.
			** Back up, and convert all the characters up until
			** the error character.
			*/

			l=dummy;

			save_char=cur_conv[dummy];

			cur_conv[dummy]=0;

			uc=(*from->c2u)(from, cur_conv, &dummy2);
			cur_conv[dummy]=save_char;

			if (!uc)
			{
				free(orig_str);
				free(dst_str.buffer);
				return (NULL);
			}
		}

		/* Ok, now convert unicode to dest charset, using the same
		** trial-and-error process.
		*/

		ucptr=uc;

		while (*ucptr)
		{
			size_t cnt_done;

			for (cnt_done=0; ucptr[cnt_done]; cnt_done++)
				;

			if (to->flags & UNICODE_REPLACEABLE)
			{
				s=(*to->u2c)(to, ucptr, NULL);
				if (!s)
				{
					free(orig_str);
					free(dst_str.buffer);
					free(uc);
					return NULL;
				}
			}
			else
				s=(*to->u2c)(to, ucptr, &dummy);

			if (!s)
			{
				unicode_char save_char;

				if (dummy < 0)
				{
					free(orig_str);
					free(dst_str.buffer);
					free(uc);
					return (NULL);
				}

				cnt_done=dummy;

			        save_char=ucptr[dummy];
				ucptr[dummy]=0;
				s=(*to->u2c)(to, ucptr, &dummy2);
				ucptr[dummy]=save_char;

				if (!s)
				{
					free(orig_str);
					free(dst_str.buffer);
					free(uc);
					return (NULL);
				}
			}

			if (ux_alloc(&dst_str,
				     strlen(dst_str.buffer)+strlen(s)+2))
			{
				free(s);
				free(orig_str);
				free(dst_str.buffer);
				free(uc);
				return (NULL);
			}

			strcat(dst_str.buffer, s);
			free(s);
			ucptr += cnt_done;
			if (*ucptr)
			{
				strcat(dst_str.buffer, ".");
				++ucptr;
			}
		}

		cur_conv += l;

		if (*cur_conv)
		{
			char buf[2];

			if (ux_alloc(&dst_str, strlen(dst_str.buffer)+1))
			{
				free(orig_str);
				free(dst_str.buffer);
				free(uc);
				return (NULL);
			}

			buf[0]= *cur_conv++;
			buf[1]=0;
			strcat(dst_str.buffer, buf);
		}
		free(uc);
	}

	free(orig_str);
	return (dst_str.buffer);
}
