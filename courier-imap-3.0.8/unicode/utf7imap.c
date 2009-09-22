/*
** Copyright 2000-2003 Double Precision, Inc.
** See COPYING for distribution information.
**
*/

#include	"unicode_config.h"
#include	"unicode.h"
#include	<string.h>
#include	<stdlib.h>

static const char rcsid[]="$Id: utf7imap.c,v 1.6 2004/05/23 14:28:25 mrsam Exp $";

static const char mbase64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";

static char mbase64_lookup[256];
static int mbase64_lookup_init=0;

unicode_char *unicode_modutf7touc(const char *s, int *err)
{
	size_t l=strlen(s), i;
	unicode_char *uc=malloc(sizeof(unicode_char)*(l+1));
	/* That's the worst case scenario, that's all. */

	if (!uc)
		return (NULL);

	if (err)
		*err= -1;

	/* First time through - initialize fast lookup table */

	if (!mbase64_lookup_init)
	{
		mbase64_lookup_init=1;

		for (i=0; i<256; i++)
			mbase64_lookup[i]= (char)-1;

		for (i=0; mbase64[i]; i++)
			mbase64_lookup[(int)mbase64[i]]=i;
	}
	i=0;

	for (l=0; s[l]; l++)
	{
		unicode_char uu;
		int bitcount;

		if ( s[l] < 0x20 || s[l] >= 0x7F )
		{
			free(uc);
			if (err) *err=l;
			return (NULL);
		}

		if ( s[l] != '&' )
		{
			uc[i++]= (int)(unsigned char)s[l];
			continue;
		}

		if ( s[++l] == '-' )
		{
			uc[i++]='&';
			continue;
		}

		bitcount=0;
		uu=0;

		for ( ; s[l] != '-'; l++)
		{
			int bits;

			if ((char)(bits=
				   mbase64_lookup[s[l] & 255]) == (char)-1)
			{
				free(uc);
				if (err) *err=l;
				return (0);
			}

			if (bitcount + 6 >= 16)
				/* These six more bits are enough for UCS2 */
			{
				int n=bitcount + 6 - 16;	/* Leftover */

				uu = (uu << (6-n)) | (bits >> n);
				uc[i++] = (uu & 0xFFFF);

				uu = bits;	/* The leftovers */
				bitcount=n;
			}
			else
			{
				uu = (uu << 6) | bits;
				bitcount += 6;
			}
		}
	}
	uc[i]=0;
	return (uc);
}

static size_t uctoutf7_pass(const unicode_char *, const unicode_char *,
			    char *);

char *unicode_uctomodutf7(const unicode_char *p)
{
	return unicode_uctomodutf7x(p, NULL);
}

char *unicode_uctomodutf7x(const unicode_char *p,
			   const unicode_char *specials)
{
	size_t n=uctoutf7_pass(p, specials, NULL);
	char *s=malloc(n);

	if (s)
		uctoutf7_pass(p, specials, s);
	return (s);
}

static int is_special(unicode_char uc, const unicode_char *specials)
{
	while (specials && *specials)
		if (*specials++ == uc)
			return 1;

	return uc < 0x20 || uc >= 0x7F;
}

static size_t uctoutf7_pass(const unicode_char *uc,
			    const unicode_char *specials,
			    char *p)
{
	size_t n=0;

	while (*uc)
	{
		unsigned bits, bitcount;

		if (!is_special(*uc, specials))
		{
			/* Straightforward deal for straightforward ASCII */

			if (p)
				*p++ = (char)*uc;
			++n;

			if (*uc++ == '&')
			{
				if (p) *p++ = '-';
				++n;
			}
			continue;
		}

		if (p) *p++ = '&'; /* Begin modified base64 */
		++n;

		bits=bitcount=0;
		while ( *uc && is_special(*uc, specials))
		{
			unicode_char uu= *uc++ & 0xFFFF;
			int counter=16;

			if (!uu) uu=0xFFFD;

			/* Process 16 bits */

			while (counter)
			{
				int x;

				if (counter + bitcount < 6)
				{
					/* Add these bits, then we're done */

					bits = (bits << counter) |
						(uu >> (16-counter));
					bitcount += counter;
					break;
				}

				/* Have enough bits to encode */

				x= 6 - bitcount;

				bits = (bits << x) | (uu >> (16-x));
				uu = (uu << x) & 0xFFFF;
				counter -= x;

				if (p)
					*p++ = mbase64[bits];
				++n;
				bits=bitcount=0;
			}
		}

		if (bitcount)	/* Leftovers */
		{
			bits <<= (6-bitcount);
			if (p)
				*p++ = mbase64[bits];
			++n;
		}

		if (p)
			*p++ = '-';
		++n;
		/* End modified base64 */
	}

	if (p)
		*p=0;
	++n;
	return (n);
}

static char *toupper_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=unicode_modutf7touc(cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_uc(*p);

	s=unicode_uctomodutf7(uc);
	if (!s && ip)
		*ip=0;
	free(uc);
	return (s);
}

static char *tolower_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=unicode_modutf7touc(cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_lc(*p);

	s=unicode_uctomodutf7(uc);
	free(uc);
	if (!s && ip)
		*ip=0;
	return (s);
}

static char *totitle_func(const struct unicode_info *u,
			  const char *cp, int *ip)
{
	unicode_char *uc=unicode_modutf7touc(cp, ip), *p;
	char *s;

	if (!uc) return (0);

	for (p=uc; *p; p++)
		*p=unicode_tc(*p);

	s=unicode_uctomodutf7(uc);
	if (!s && ip)
		*ip=0;
	free(uc);
	return (s);
}

static unicode_char *tou(const struct unicode_info *ui, const char *cs,
			int *err)
{
	return unicode_modutf7touc(cs, err);
}


static char *fromu(const struct unicode_info *ui,
		   const unicode_char *uc, int *err)
{
	if (err) *err= -1;
	return unicode_uctomodutf7(uc);
}

const struct unicode_info unicode_IMAP_MODUTF7 = {
	"X-IMAP-MODUTF-7",
	UNICODE_UTF | UNICODE_MB |
	UNICODE_HEADER_BASE64,
	tou,
	fromu,
	toupper_func,
	tolower_func,
	totitle_func};
