/*
 * $Log: dns_packet.c,v $
 * Revision 1.3  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2020-08-03 17:22:53+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 *
 * DNS should have used LZ77 instead of its own sophomoric compression algorithm.
 */

#include <error.h>
#include "dns.h"

unsigned int
dns_packet_copy(const char *buf, unsigned int len, unsigned int pos, char *out, unsigned int outlen)
{
	while (outlen)
	{
		if (pos >= len)
		{
			errno = error_proto;
			return 0;
		}
		*out = buf[pos++];
		++out;
		--outlen;
	}
	return pos;
}

unsigned int
dns_packet_skipname(const char *buf, unsigned int len, unsigned int pos)
{
	unsigned char   ch;

	for (;;)
	{
		if (pos >= len)
			break;
		ch = buf[pos++];
		if (ch >= 192)
			return pos + 1;
		if (ch >= 64)
			break;
		if (!ch)
			return pos;
		pos += ch;
	}

	errno = error_proto;
	return 0;
}

unsigned int
dns_packet_getname(const char *buf, unsigned int len, unsigned int pos, char **d)
{
	unsigned int    loop = 0;
	unsigned int    state = 0;
	unsigned int    firstcompress = 0;
	unsigned int    where;
	unsigned char   ch;
	char            name[255];
	unsigned int    namelen = 0;

	for (;;)
	{
		if (pos >= len)
			goto PROTO;
		ch = buf[pos++];
		if (++loop >= 1000)
			goto PROTO;

		if (state)
		{
			if (namelen + 1 > sizeof name)
				goto PROTO;
			name[namelen++] = ch;
			--state;
		} else
		{
			while (ch >= 192)
			{
				where = ch;
				where -= 192;
				where <<= 8;
				if (pos >= len)
					goto PROTO;
				ch = buf[pos++];
				if (!firstcompress)
					firstcompress = pos;
				pos = where + ch;
				if (pos >= len)
					goto PROTO;
				ch = buf[pos++];
				if (++loop >= 1000)
					goto PROTO;
			}
			if (ch >= 64)
				goto PROTO;
			if (namelen + 1 > sizeof name)
				goto PROTO;
			name[namelen++] = ch;
			if (!ch)
				break;
			state = ch;
		}
	}

	if (!dns_domain_copy(d, name))
		return 0;

	if (firstcompress)
		return firstcompress;
	return pos;

  PROTO:
	errno = error_proto;
	return 0;
}
