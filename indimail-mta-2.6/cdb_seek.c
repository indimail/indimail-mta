/*
 * $Log: cdb_seek.c,v $
 * Revision 1.6  2005-08-23 17:14:52+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.5  2004-10-22 20:23:25+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:17:30+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "cdb.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

int
cdb_bread(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	while (len > 0)
	{
		do
			r = read(fd, buf, len);
		while ((r == -1) && (errno == EINTR));
		if (r == -1)
			return -1;
		if (r == 0)
		{
			errno = EIO;
			return -1;
		}
		buf += r;
		len -= r;
	}
	return 0;
}

static int
match(fd, key, len)
	int             fd;
	char           *key;
	unsigned int    len;
{
	char            buf[32];
	int             n;
	int             i;

	while (len > 0)
	{
		n = sizeof(buf);
		if (n > len)
			n = len;
		if (cdb_bread(fd, buf, n) == -1)
			return -1;
		for (i = 0; i < n; ++i)
			if (buf[i] != key[i])
				return 0;
		key += n;
		len -= n;
	}
	return 1;
}

int
cdb_seek(fd, key, len, dlen)
	int             fd;
	char           *key;
	unsigned int    len;
	uint32         *dlen;
{
	unsigned char   packbuf[8];
	uint32          pos;
	uint32          h;
	uint32          lenhash;
	uint32          h2;
	uint32          loop;
	uint32          poskd;

	h = cdb_hash((unsigned char *) key, len);
	pos = 8 * (h & 255);
	if (lseek(fd, (off_t) pos, SEEK_SET) == -1)
		return -1;
	if (cdb_bread(fd, (char *) packbuf, 8) == -1)
		return -1;
	pos = cdb_unpack(packbuf);
	lenhash = cdb_unpack(packbuf + 4);
	if (!lenhash)
		return 0;
	h2 = (h >> 8) % lenhash;
	for (loop = 0; loop < lenhash; ++loop)
	{
		if (lseek(fd, (off_t) (pos + 8 * h2), SEEK_SET) == -1)
			return -1;
		if (cdb_bread(fd, (char *) packbuf, 8) == -1)
			return -1;
		poskd = cdb_unpack(packbuf + 4);
		if (!poskd)
			return 0;
		if (cdb_unpack(packbuf) == h)
		{
			if (lseek(fd, (off_t) poskd, SEEK_SET) == -1)
				return -1;
			if (cdb_bread(fd, (char *) packbuf, 8) == -1)
				return -1;
			if (cdb_unpack(packbuf) == len)
				switch (match(fd, key, len))
				{
				case -1:
					return -1;
				case 1:
					*dlen = cdb_unpack(packbuf + 4);
					return 1;
				}
		}
		if (++h2 == lenhash)
			h2 = 0;
	}
	return 0;
}

void
getversion_cdb_seek_c()
{
	static char    *x = "$Id: cdb_seek.c,v 1.6 2005-08-23 17:14:52+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
