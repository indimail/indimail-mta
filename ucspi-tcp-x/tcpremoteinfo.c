/*
 * $Log: tcpremoteinfo.c,v $
 * Revision 1.4  2020-08-03 17:27:53+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.3  2008-07-25 16:50:15+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.2  2005-06-10 12:19:39+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fmt.h>
#include <substdio.h>
#include <error.h>
#include <iopause.h>
#include "socket.h"
#include "timeoutconn.h"
#include "tcpremoteinfo.h"

static struct taia now;
static struct taia deadline;

static ssize_t
mywrite(int fd, char *buf, size_t len)
{
	iopause_fd      x;

	x.fd = fd;
	x.events = IOPAUSE_WRITE;
	for (;;) {
		taia_now(&now);
		iopause(&x, 1, &deadline, &now);
		if (x.revents)
			break;
		if (taia_less(&deadline, &now)) {
			errno = error_timeout;
			return -1;
		}
	}
	return write(fd, buf, len);
}

static ssize_t
myread(int fd, char *buf, size_t len)
{
	iopause_fd      x;

	x.fd = fd;
	x.events = IOPAUSE_READ;
	for (;;) {
		taia_now(&now);
		iopause(&x, 1, &deadline, &now);
		if (x.revents)
			break;
		if (taia_less(&deadline, &now)) {
			errno = error_timeout;
			return -1;
		}
	}
	return read(fd, buf, len);
}

#ifdef IPV6
static int
doit(stralloc * out, int s, char ipremote[16], uint16 portremote, char iplocal[16],
	uint16 portlocal, unsigned int timeout, uint32 netif)
#else
static int
doit(stralloc *out, int s, char ipremote[4], uint16 portremote, char iplocal[4],
	uint16 portlocal, unsigned int timeout)
#endif
{
	substdio        b;
	char            bspace[128];
	char            strnum[FMT_ULONG];
	int             numcolons;
	char            ch;

#ifdef IPV6
	if (socket_bind6(s, iplocal, 0, netif) == -1)
#else
	if (socket_bind4(s, iplocal, 0) == -1)
#endif
		return -1;
#ifdef IPV6
	if (timeoutconn6(s, ipremote, 113, timeout, netif) == -1)
#else
	if (timeoutconn(s, ipremote, 113, timeout) == -1)
#endif
		return -1;
	substdio_fdbuf(&b, (ssize_t (*)(int,  char *, size_t)) mywrite, s, bspace, sizeof bspace);
	substdio_put(&b, strnum, fmt_ulong(strnum, portremote));
	substdio_put(&b, " , ", 3);
	substdio_put(&b, strnum, fmt_ulong(strnum, portlocal));
	substdio_put(&b, "\r\n", 2);
	if (substdio_flush(&b) == -1)
		return -1;
	substdio_fdbuf(&b, (ssize_t (*)(int,  char *, size_t)) myread, s, bspace, sizeof bspace);
	numcolons = 0;
	for (;;) {
		if (substdio_get(&b, &ch, 1) != 1)
			return -1;
		if ((ch == ' ') || (ch == '\t') || (ch == '\r'))
			continue;
		if (ch == '\n')
			return 0;
		if (numcolons < 3) {
			if (ch == ':')
				++numcolons;
		} else {
			if (!stralloc_append(out, &ch))
				return -1;
			if (out->len > 256)
				return 0;
		}
	}
}

#ifdef IPV6
int
remoteinfo6(stralloc *out, char ipremote[16], uint16 portremote, char iplocal[16],
	uint16 portlocal, unsigned int timeout, uint32 netif)
#else
int
remoteinfo(stralloc *out, char ipremote[4], uint16 portremote, char iplocal[4],
	uint16 portlocal, unsigned int timeout)
#endif
{
	int             s;
	int             r;

	if (!stralloc_copys(out, ""))
		return -1;
	taia_now(&now);
	taia_uint(&deadline, timeout);
	taia_add(&deadline, &now, &deadline);
#ifdef IPV6
	if ((s = socket_tcp6()) == -1)
		return -1;
	r = doit(out, s, ipremote, portremote, iplocal, portlocal, timeout, netif);
#else
	if ((s = socket_tcp()) == -1)
		return -1;
	r = doit(out, s, ipremote, portremote, iplocal, portlocal, timeout);
#endif
	close(s);
	return r;
}
