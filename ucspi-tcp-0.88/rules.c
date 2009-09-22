/*
 * $Log: rules.c,v $
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
#include "alloc.h"
#include "stralloc.h"
#include "open.h"
#include "cdb.h"
#include "rules.h"

stralloc        rules_name = { 0 };

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
	if (cdb_read(&c, data, datalen, cdb_datapos(&c)) == -1)
	{
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
	int             r;

	if (info)
	{
		if (!stralloc_copys(&rules_name, info))
			return -1;
		if (!stralloc_cats(&rules_name, "@"))
			return -1;
		if (!stralloc_cats(&rules_name, ip))
			return -1;
		if ((r = dorule(callback)))
			return r;
		if (host)
		{
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
	if (!stralloc_copys(&rules_name, ip))
		return -1;
	if ((r = dorule(callback)))
		return r;
	if (host)
	{
		if (!stralloc_copys(&rules_name, "="))
			return -1;
		if (!stralloc_cats(&rules_name, host))
			return -1;
		if ((r = dorule(callback)))
			return r;
	}

	if (!stralloc_copys(&rules_name, ip))
		return -1;
	while (rules_name.len > 0)
	{
#ifdef IPV6
		if (ip[rules_name.len - 1] == '.' || ip[rules_name.len - 1] == ':')
#else
		if (ip[rules_name.len - 1] == '.')
#endif
		{
			if ((r = dorule(callback)))
				return r;
		}
		--rules_name.len;
	}
	if (host)
	{
		while (*host)
		{
			if (*host == '.')
			{
				if (!stralloc_copys(&rules_name, "="))
					return -1;
				if (!stralloc_cats(&rules_name, host))
					return -1;
				if ((r = dorule(callback)))
					return r;
			}
			++host;
		}
		if (!stralloc_copys(&rules_name, "="))
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
