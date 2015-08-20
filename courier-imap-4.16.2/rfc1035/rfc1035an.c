/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"rfc1035.h"
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<errno.h>
#include	<string.h>


void rfc1035_ntoa_ipv4(const struct in_addr *in, char *buf)
{
union	{
	unsigned char addr[4];
	RFC1035_UINT32 n;
	} u;

int	i;
char	*p;
char	pbuf[4];

	u.n=in->s_addr;
	for (i=0; i<4; i++)
	{
		if (i)	*buf++='.';
		p=pbuf+3;
		*p=0;
		do
		{
			*--p = '0' + (u.addr[i] % 10);
		} while ( (u.addr[i] /= 10) != 0);
		while (*p)
			*buf++ = *p++;
	}
	*buf=0;
}

void rfc1035_ntoa(const RFC1035_ADDR *in, char *buf)
{
#if	RFC1035_IPV6
	inet_ntop(AF_INET6, in, buf, RFC1035_MAXNAMESIZE+1);
#else
	rfc1035_ntoa_ipv4(in, buf);
#endif
}

static RFC1035_UINT32 doaton(const char *p, int *err)
{
RFC1035_UINT32 octets[4];
int	i;
int	base;

	*err=1;
	for (i=0; i<4; i++)
	{
		if (i > 0)
		{
			if (*p == '\0')	break;
			if (*p != '.')	return ( -1 );
			++p;
		}
		if (*p < '0' || *p > '9')	return (-1);
		octets[i]=0;
		base=10;
		if (*p == '0')	base=8;

		while (*p >= '0' && *p < '0'+base)
			octets[i] = octets[i] * base + (*p++ - '0');
	}
	if (*p)	return ( -1 );
	*err=0;
	if (i == 1)	return (octets[0]);
	*err=1;
	if (octets[0] > 255)	return ( -1 );
	octets[0] <<= 24;

	if (i == 2)
	{
		if (octets[1] > 0x00FFFFFF)	return ( -1 );
		*err=0;
		return (octets[0] | octets[1]);
	}

	if (octets[1] > 255)	return ( -1 );
	octets[1] <<= 16;
	if (i == 3)
	{
		if (octets[2] > 0x0000FFFF)	return ( -1 );
		*err=0;
		return (octets[0] | octets[1] | octets[2]);
	}
	if (octets[2] > 255 || octets[3] > 255)	return ( -1 );
	*err=0;
	return (octets[0] | octets[1] | (octets[2] << 8) | octets[3]);
}

int rfc1035_aton_ipv4(const char *p, struct in_addr *in4)
{
int	dummy;
RFC1035_UINT32 n=doaton(p, &dummy);
union	{
	unsigned char addr[4];
	struct in_addr n;
	} u;

	u.addr[3]=n;
	u.addr[2]=n >> 8;
	u.addr[1]=n >> 16;
	u.addr[0]=n >> 24;

	if (dummy)	errno=EINVAL;
	*in4=u.n;
	return (dummy);
}

#if	RFC1035_IPV6

int rfc1035_aton(const char *p, RFC1035_ADDR *in6)
{
struct in_addr	in4;

	if (rfc1035_aton_ipv4(p, &in4) == 0)
	{
		rfc1035_ipv4to6(in6, &in4);
		return (0);
	}
	if (inet_pton(AF_INET6, p, in6) <= 0)
		return (-1);
	return (0);
}

#else

int rfc1035_aton(const char *p, RFC1035_ADDR *in4)
{
	return (rfc1035_aton_ipv4(p, in4));
}
#endif
