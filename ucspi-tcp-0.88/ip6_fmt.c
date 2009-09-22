/*
 * $Log: ip6_fmt.c,v $
 * Revision 1.2  2005-06-10 12:10:40+05:30  Cprogrammer
 * conditional ipv6 support
 *
 * Revision 1.1  2005-06-10 09:04:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef IPV6
#include "fmt.h"
#include "byte.h"
#include "ip4.h"
#include "ip6.h"

unsigned int
ip6_fmt(char *s, char ip[16])
{
	unsigned int    len;
	unsigned int    i;
	unsigned int    temp;
	unsigned int    compressing;
	int             j;

	len = 0;
	compressing = 0;
	for (j = 0; j < 16; j += 2)
	{
		if (j == 12 && ip6_isv4mapped(ip))
		{
			temp = ip4_fmt(s, ip + 12);
			len += temp;
			s += temp;
			break;
		}
		temp = ((unsigned long) (unsigned char) ip[j] << 8) + (unsigned long) (unsigned char) ip[j + 1];
		if (temp == 0)
		{
			if (!compressing)
			{
				compressing = 1;
				if (j == 0)
				{
					*s = ':';
					s += 1;
					++len;
				}
			}
		} else
		{
			if (compressing)
			{
				compressing = 0;
				*s = ':';
				s += 1;
				++len;
			}
			i = fmt_xlong(s, temp);
			len += i;
			if (s)
				s += i;
			if (s && j < 14)
				*s++ = ':';
			++len;
		}
	}
	if (s)
		*s = 0;
	return len;
}

static char
tohex(char num)
{
	if (num < 10)
		return num + '0';
	else
	if (num < 16)
		return num - 10 + 'a';
	else
		return -1;
}

unsigned int
ip6_fmt_flat(char *s, char ip[16])
{
	int             i;
	for (i = 0; i < 16; i++)
	{
		*s++ = tohex((unsigned char) ip[i] >> 4);
		*s++ = tohex((unsigned char) ip[i] & 15);
	}
	return 32;
}
#endif
