/*
** Copyright 2002-2006 Double Precision, Inc.
** See COPYING for distribution information.
*/

#define TLSCACHEMINSIZE (sizeof(struct hdr) + 5 * (sizeof(struct obj)+8))
#include "tlscache.c"


static int printcache(void *rec, size_t recsize, int *doupdate,
		      void *arg)
{
	if (fwrite((const char *)rec, recsize, 1, stdout) == 1)
		printf("\n");
	return 0;
}

static int replacecache(void *rec, size_t recsize, int *doupdate,
		      void *arg)
{
	const char *p=(const char *)arg;
	const char *q;

	if ((q=strchr(p, '-')) == NULL || strlen(q+1) != q-p)
		return (0);

	if (recsize == q-p && memcmp(rec, p, q-p) == 0)
	{
		memcpy(rec, q+1, q-p);
		*doupdate=1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	struct CACHE *p=tls_cache_open("test.dat", TLSCACHEMINSIZE);

	if (!p)
	{
		perror("test.dat");
		return (-1);
	}

	if (argc > 1)
	{
		char *s=argv[1];

		if (*s == '+')
		{
			++s;
			if (tls_cache_add(p, s, strlen(s)))
			{
				perror("tls_cache_add");
			}
		}

		if (*s == '-')
		{
			if (tls_cache_walk(p, replacecache, s+1) < 0)
			{
				perror("tls_cache_walk");
				exit(1);
			}
		}
	}

	if (tls_cache_walk(p, printcache, NULL) < 0)
		perror("tls_cache_walk");
	tls_cache_close(p);
	return (0);
}
