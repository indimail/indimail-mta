/*
 * $Log: dns_rcip.c,v $
 * Revision 1.3  2017-03-30 22:47:02+05:30  Cprogrammer
 * prefix rbl with ip6_scan(), ip4_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.2  2005-06-10 09:05:46+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <taia.h>
#include <byte.h>
#include <env.h>
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#endif
#include "dns.h"
#include "openreadclose.h"

static stralloc data = { 0 };

static int
#ifdef IPV6
init(char ip[256])
#else
init(char ip[64])
#endif
{
	int             i;
	int             j;
	int             iplen = 0;
	char           *x;

	if ((x = env_get("DNSCACHEIP")))
	{
		while (iplen <= 60)
		{
			if (*x == '.')
				++x;
			else
			{
#ifdef IPV6
				i = rblip6_scan(x,ip + iplen);
#else
				i = rblip4_scan(x, ip + iplen);
#endif
				if (!i)
					break;
				x += i;
#ifdef IPV6
				iplen += 16;
#else
				iplen += 4;
#endif
			}
		}
	}
	if (!iplen)
	{
		if ((i = openreadclose("/etc/resolv.conf", &data, 64)) == -1)
			return -1;
		if (i)
		{
			if (!stralloc_append(&data, "\n"))
				return -1;
			i = 0;
			for (j = 0; j < data.len; ++j)
			{
				if (data.s[j] == '\n')
				{
					if (byte_equal("nameserver ", 11, data.s + i) || byte_equal("nameserver\t", 11, data.s + i))
					{
						i += 10;
						while ((data.s[i] == ' ') || (data.s[i] == '\t'))
							++i;
#ifdef IPV6
						if (iplen <= 60 && rblip6_scan(data.s + i,ip + iplen))
							iplen += 16;
#else
						if (iplen <= 60 && rblip4_scan(data.s + i, ip + iplen))
							iplen += 4;
#endif
					}
					i = j + 1;
				}
			}
		}
	}
	if (!iplen)
	{
#ifdef IPV6
		byte_copy(ip, 16, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\1");
		iplen = 16;
#else
		byte_copy(ip, 4, "\177\0\0\1");
		iplen = 4;
#endif
	}
#ifdef IPV6
	byte_zero(ip + iplen, 256 - iplen);
#else
	byte_zero(ip + iplen, 64 - iplen);
#endif
	return 0;
}

static int      ok = 0;
static unsigned int uses;
static struct taia deadline;
#ifdef IPV6
static char     ip[256]; /* defined if ok */
#else
static char     ip[64]; /*- defined if ok */
#endif

int
#ifdef IPV6
dns_resolvconfip(char s[256])
#else
dns_resolvconfip(char s[64])
#endif
{
	struct taia     now;

	taia_now(&now);
	if (taia_less(&deadline, &now))
		ok = 0;
	if (!uses)
		ok = 0;

	if (!ok)
	{
		if (init(ip) == -1)
			return -1;
		taia_uint(&deadline, 600);
		taia_add(&deadline, &now, &deadline);
		uses = 10000;
		ok = 1;
	}

	--uses;
#ifdef IPV6
	byte_copy(s, 256, ip);
#else
	byte_copy(s, 64, ip);
#endif
	return 0;
}
