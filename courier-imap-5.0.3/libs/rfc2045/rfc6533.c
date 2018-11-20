/*
** Copyright 2018 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/

#if    HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif
#include	"rfc2045.h"
#include	"unicode/courier-unicode.h"
#include	<string.h>
#include	<stdlib.h>

static const char xdigit[]="0123456789ABCDEF";

static void count(const char *s, size_t n, void *ptr)
{
	(*(size_t *)ptr) += n;
}

static void save(const char *s, size_t n, void *ptr)
{
	char **p=(char **)ptr;

	memcpy(*p, s, n);
	*p += n;
}

static void encode_rfc822(const char *address,
			  void (*callback)(const char *, size_t, void *),
			  void *arg)
{
	callback("rfc822;", 7, arg);

	while (*address)
	{
		size_t i;

		for (i=0; address[i]; ++i)
		{
			if (address[i] < '!' || address[i] > '~')
				break;
			if (address[i] == '+' || address[i] == '=')
				break;
		}

		if (i == 0)
		{
			(*callback)("+", 1, arg);

			(*callback)(xdigit + ((*address >> 4) & 15), 1, arg);
			(*callback)(xdigit + (*address & 15), 1, arg);
			++address;
			continue;
		}

		(*callback)(address, i, arg);
		address += i;
	}
	(*callback)("", 1, arg);
}

static void encode_rfc6533(const char *address,
			   void (*callback)(const char *, size_t, void *),
			   void *arg)
{
	callback("utf-8;", 6, arg);

	while (*address)
	{
		size_t i;

		for (i=0; address[i]; ++i)
		{
			if ((unsigned char)address[i] <= ' ')
				break;

			if (address[i] == '\\' ||
			    address[i] == '+' ||
			    address[i] == '=' ||
			    address[i] == 0x7f)
				break;
		}

		if (i == 0)
		{
			static const char xdigit[]="0123456789ABCDEF";

			(*callback)("\\x{", 3, arg);
			(*callback)(xdigit + ((*address >> 4) & 15), 1, arg);
			(*callback)(xdigit + (*address & 15), 1, arg);
			(*callback)("}", 1, arg);
			++address;
			continue;
		}

		(*callback)(address, i, arg);
		address += i;
	}
	(*callback)("", 1, arg);
}

char *rfc6533_encode(const char *address, int use_rfc822)
{
	size_t l=0;
	char *buffer;
	char *p;
	const char *cp;

	for (cp=address; *cp; ++cp)
		if (*cp & 0x80)
			break;

	if (!*cp || use_rfc822)
	{
		encode_rfc822(address, count, &l);

		if ((buffer=malloc(l)) == NULL)
			abort();

		p=buffer;
		encode_rfc822(address, save, &p);
		return buffer;
	}

	encode_rfc6533(address, count, &l);

	if ((buffer=malloc(l)) == NULL)
		abort();
	p=buffer;
	encode_rfc6533(address, save, &p);
	return buffer;
}

static int decode_rfc6533(const char *address,
			  void (*callback)(const char *, size_t, void *),
			  void *arg)
{
	while (*address)
	{
		size_t i;
		char32_t c;
		char *p;
		size_t ignore1;
		int err;

		for (i=0; address[i]; ++i)
		{
			if (address[i] == '\\')
				break;
		}

		if (i)
		{
			(*callback)(address, i, arg);

			address += i;
			continue;
		}

		if (address[1] != 'x' ||
		    address[2] != '{')
			return -1;

		c=0;

		address += 3;

		while (*address != '}')
		{
			const char *p;

			if (!*address)
				return -1;

			p=strchr(xdigit, *address);
			if (!p)
				return -1;
			c <<= 4;
			c |= (p-xdigit);
			++address;
		}
		++address;
		if (c == 0)
			return -1;

		err=0;
		if (unicode_convert_fromu_tobuf(&c, 1, "utf-8",
						&p, &ignore1, &err))
			return NULL;

		if (err)
		{
			free(p);
			return NULL;
		}
		(*callback)(p, strlen(p), arg);
		free(p);
	}
	(*callback)("", 1, arg);
	return 0;
}

char *rfc6533_decode(const char *address)
{
	size_t l;
	char *buf;
	char *p;

	if (strncasecmp(address, "rfc822;", 7) == 0)
	{
		buf=malloc(strlen(address));

		if (!buf)
			abort();

		p=buf;

		address += 7;

		while (*address)
		{
			const char *hi, *lo;

			if (*address != '+')
			{
				*p++ = *address++;
				continue;
			}

			++address;

			if (*address)
			{
				hi=strchr(xdigit, *address);
				++address;
				if (*address)
				{
					lo=strchr(xdigit, *address);
					++address;

					if (hi && lo)
					{
						char n= (char)
							((hi-xdigit) * 16
							 +(lo-xdigit));

						if (n)
							*p++=n;
					}
				}
			}
		}
		*p=0;
	}
	else
	{
		if (strncasecmp(address, "utf-8;", 6))
			return NULL;

		l=0;

		if (decode_rfc6533(address+6, count, &l))
			return NULL;

		if ((buf=malloc(l)) == NULL)
			abort();

		p=buf;
		decode_rfc6533(address+6, save, &p);
	}

	for (p=buf; *p; ++p)
		if ((unsigned char)*p <= ' ')
		{
			free(buf);
			return NULL;
		}
	return buf;
}
