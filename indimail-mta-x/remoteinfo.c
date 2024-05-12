/*
 * $Log: remoteinfo.c,v $
 * Revision 1.9  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2008-07-15 19:53:22+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.6  2005-06-17 12:01:16+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.5  2004-10-22 20:29:57+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-11 14:28:29+05:30  Cprogrammer
 * removed superfluous comment
 *
 * Revision 1.3  2004-07-17 21:22:37+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include "byte.h"
#include "error.h"
#include "socket.h"
#include "substdio.h"
#include "ip.h"
#include "fmt.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "remoteinfo.h"

static char     line[999];
static int      t;

static ssize_t
mywrite(int fd, char *buf, size_t len)
{
	return timeoutwrite(t, fd, buf, len);
}

static ssize_t
myread(int fd, char *buf, size_t len)
{
	return timeoutread(t, fd, buf, len);
}

char           *
remoteinfo_get(union sockunion *saremote, union sockunion *salocal, int timeout)
{
	char           *x;
	int             s;
	union sockunion sa;
	substdio        ss;
	char            buf[32];
	unsigned int    len, rp, lp;
	int             numcolons;
	char            ch;

	t = timeout;
#ifdef IPV6
	if ((s = (saremote->sa.sa_family == AF_INET ? socket_tcp4() : socket_tcp6())) == -1)
#else
	if ((s = socket_tcp4()) == -1)
#endif
		return 0;
	switch (saremote->sa.sa_family)
	{
	case AF_INET:
		rp = ntohs(saremote->sa4.sin_port);
		lp = ntohs(salocal->sa4.sin_port);
		byte_zero((char *) &sa, sizeof(sa));
		sa.sa4.sin_family = AF_INET;
		byte_copy((char *) &sa.sa4.sin_addr, 4, (char *) &salocal->sa4.sin_addr);
		sa.sa4.sin_port = 0;
		if (bind(s, (struct sockaddr *) &sa.sa, sizeof(sa.sa4)) == -1)
		{
			close(s);
			return 0;
		}
		if (timeoutconn4(s, (ip_addr *) &saremote->sa4.sin_addr,
			(union v46addr *) &salocal->sa4.sin_addr, 113, timeout) == -1)
		{
			close(s);
			return 0;
		}
		break;
#ifdef LIBC_HAS_IP6
	case AF_INET6:
		rp = ntohs(saremote->sa6.sin6_port);
		lp = ntohs(salocal->sa6.sin6_port);
		byte_zero((char *) &sa, sizeof(sa));
		if (noipv6)
		{
			int             i;
			char            ip[16];

			byte_copy(ip, 16, (char *) &salocal->sa6.sin6_addr);
			for (i = 0; i < 16; i++)
			{
				if (ip[i] != 0)
					break;
			}
			if (i == 16 || ip6_isv4mapped(ip))
			{
				byte_copy((char *) &sa.sa4.sin_addr, 4, (char *) &salocal->sa6.sin6_addr + 12);
				sa.sa4.sin_family = AF_INET;
				sa.sa4.sin_port = 0;
			} else
			{
				close(s);
				errno = error_proto;
				return 0;
			}
		} else
		{
			byte_copy((char *) &sa.sa6.sin6_addr, 16, (char *) &salocal->sa6.sin6_addr);
			sa.sa6.sin6_family = AF_INET6;
			sa.sa6.sin6_port = 0;
			sa.sa6.sin6_flowinfo = 0;
		}
		if (bind(s, (struct sockaddr *) &sa.sa, sizeof(struct sockaddr)) == -1)
		{
			close(s);
			return 0;
		}
		if (timeoutconn6(s, (ip6_addr *) &saremote->sa6.sin6_addr,
			(union v46addr *) &salocal->sa6.sin6_addr, 113, timeout) == -1)
		{
			close(s);
			return 0;
		}
		break;
#endif
	default:
		return 0;
	}
	fcntl(s, F_SETFL, fcntl(s, F_GETFL, 0) & ~O_NDELAY);
	len = 0;
	len += fmt_ulong(line + len, rp);
	len += fmt_str(line + len, " , ");
	len += fmt_ulong(line + len, lp);
	len += fmt_str(line + len, "\r\n");
	substdio_fdbuf(&ss, mywrite, s, buf, sizeof buf);
	if (substdio_putflush(&ss, line, len) == -1)
	{
		close(s);
		return 0;
	}
	substdio_fdbuf(&ss, myread, s, buf, sizeof buf);
	x = line;
	numcolons = 0;
	for (;;)
	{
		if (substdio_get(&ss, &ch, 1) != 1)
		{
			close(s);
			return 0;
		}
		if ((ch == ' ') || (ch == '\t') || (ch == '\r'))
			continue;
		if (ch == '\n')
			break;
		if (numcolons < 3)
		{
			if (ch == ':')
				++numcolons;
		} else
		{
			*x++ = ch;
			if (x == line + sizeof(line) - 1)
				break;
		}
	}
	*x = 0;
	close(s);
	return line;
}

void
getversion_remoteinfo_c()
{
	const char     *x = "$Id: remoteinfo.c,v 1.9 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
