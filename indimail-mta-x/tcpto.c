/*-
 * $Id: tcpto.c,v 1.18 2025-01-01 21:35:30+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/socket.h>
#include "tcpto.h"
#include "open.h"
#include "lock.h"
#include "seek.h"
#include "now.h"
#include "ip.h"
#include "byte.h"
#include "env.h"
#include "fmt.h"
#include "tcpto.h"
#include "datetime.h"
#include "variables.h"

char            tcpto_buf[TCPTO_BUFSIZ];

static int      flagwasthere, fdlock;

static int
getbuf()
{
	int             r, fd;
	char            lockfile[1024];

	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	if ((r = fmt_strn(lockfile, queuedir, 1024)) > 1012)
		return(0);
	r += fmt_str(lockfile + r, "/lock/tcpto");
	lockfile[r] = 0;
	if ((fdlock = open_write(lockfile)) == -1)
		return 0;
	if ((fd = open_read(lockfile)) == -1) {
		close(fdlock);
		return 0;
	}
	if (lock_ex(fdlock) == -1) {
		close(fdlock);
		close(fd);
		return 0;
	}
	r = read(fd, tcpto_buf, sizeof(tcpto_buf));
	close(fd);
	if (r < 0) {
		close(fdlock);
		return 0;
	}
	r >>= 5;
	if (!r)
		close(fdlock);
	return r;
}

/*-
	struct tcpto
	{  32bytes
	char af;             +0
	char nul[3];         +1
	char reason;         +4
	char nul[3];         +5
	ulong when;          +8
	char nul[4];         +12
	union
	{
		ip4, ip6
	};  +16
*/

int
tcpto(struct ip_mx *ix, int min_backoff)
{
	int             n, i, af = ix->af;
	char           *record;
	ip_addr        *ip4 = &ix->addr.ip;
#ifdef IPV6
	ip6_addr       *ip6 = &ix->addr.ip6;
#endif
	datetime_sec    when, min_penalty;

	flagwasthere = 0;
	if (!(n = getbuf()))
		return 0;
	close(fdlock);
	record = tcpto_buf;
	for (i = 0; i < n; ++i) {
#ifdef IPV6
		if (af == record[0] && byte_equal(af == AF_INET ? (char *) ip4->d : (char *) ip6->d, af == AF_INET ? 4 : 16, (char *) record + 16))
#else
		if (af == record[0] && byte_equal((char *) ip4->d, 4, (char *) record + 16))
#endif
		{
			flagwasthere = 1;
			if (record[4] >= 2) {
				when = (unsigned long) (unsigned char ) record[11];
				when = (when << 8) + (unsigned long) (unsigned char ) record[10];
				when = (when << 8) + (unsigned long) (unsigned char ) record[9];
				when = (when << 8) + (unsigned long) (unsigned char ) record[8];
				/*
				 * ((60 + (getpid() & 31)) << 6) =
				 * Timeout in seconds: 64 - 97 minutes, depending on pid
				 */
				if (min_backoff > 0)
					min_penalty = ((60 + (getpid() & 31)) << 6) - MIN_PENALTY;
				else {
					min_penalty = 0;
					min_backoff = -min_backoff;
				}
				/*
				 * If this IP address had a SMTP connection timeout last
				 * time, don't connect to it again immediately
				 */
				if (now() - when < min_backoff + min_penalty)
					return 1;
			}
			return 0;
		}
		record += 32;
	}
	return 0;
}

void
tcpto_err(struct ip_mx *ix, int flagerr, int max_tolerance)
{
	int             n, i, firstpos, af = ix->af;
	char           *record;
	datetime_sec    when, firstwhen = 0, lastwhen;
	ip_addr        *ip4 = &ix->addr.ip;
#ifdef IPV6
	ip6_addr       *ip6 = &ix->addr.ip6;
#endif

	/*- could have been added, but not worth the effort to check */
	if (!flagerr && !flagwasthere)
		return;
	if (!(n = getbuf()))
		return;
	record = tcpto_buf;
	for (i = 0; i < n; ++i) {
#ifdef IPV6
		if (af == record[0] && byte_equal(af == AF_INET ? (char *) ip4->d : (char *) ip6->d, af == AF_INET ? 4 : 16, (char *) record + 16))
#else
		if (af == record[0] && byte_equal((char *) ip4->d, 4, (char *) record + 16))
#endif
		{
			if (!flagerr)
				record[4] = 0;
			else {
				lastwhen = (unsigned long) (unsigned char ) record[11];
				lastwhen = (lastwhen << 8) + (unsigned long) (unsigned char ) record[10];
				lastwhen = (lastwhen << 8) + (unsigned long) (unsigned char ) record[9];
				lastwhen = (lastwhen << 8) + (unsigned long) (unsigned char ) record[8];
				when = now();
				if (record[4] && (when < max_tolerance + lastwhen)) {
					close(fdlock);
					return;
				}
				if (++record[4] > 10)
					record[4] = 10;
				record[8] = when;
				when >>= 8;
				record[9] = when;
				when >>= 8;
				record[10] = when;
				when >>= 8;
				record[11] = when;
			}
			if (seek_set(fdlock, i << 5) == 0 && write(fdlock, record, 32) < 32)
				; /*XXX*/
			close(fdlock);
			return;
		}
		record += 32;
	}
	if (!flagerr) {
		close(fdlock);
		return;
	}
	record = tcpto_buf;
	for (i = 0; i < n; ++i) {
		if (!record[4])
			break;
		record += 32;
	}
	if (i >= n) {
		firstpos = -1;
		record = tcpto_buf;
		for (i = 0; i < n; ++i) {
			when = (unsigned long) (unsigned char ) record[11];
			when = (when << 8) + (unsigned long) (unsigned char ) record[10];
			when = (when << 8) + (unsigned long) (unsigned char ) record[9];
			when = (when << 8) + (unsigned long) (unsigned char ) record[8];
			when += (record[4] << 10);
			if ((firstpos < 0) || (when < firstwhen)) {
				firstpos = i;
				firstwhen = when;
			}
			record += 32;
		}
		i = firstpos;
	}
	if (i >= 0) {
		record = tcpto_buf + (i << 5);
		record[0] = af;
#ifdef IPV6
		byte_copy(record + 16, af == AF_INET ? 4 : 16 , (af == AF_INET ? (char *) &ip4->d : (char *) &ip6->d));
#else
		byte_copy(record + 16, 4, (char *) &ip4->d);
#endif
		when = now();
		record[8] = when;
		when >>= 8;
		record[9] = when;
		when >>= 8;
		record[10] = when;
		when >>= 8;
		record[11] = when;
		record[4] = 1;
		if (seek_set(fdlock, i << 5) == 0 && write(fdlock, record, 32) < 32)
			; /*XXX*/
	}
	close(fdlock);
}

void
getversion_tcpto_c()
{
	const char     *x = "$Id: tcpto.c,v 1.18 2025-01-01 21:35:30+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: tcpto.c,v $
 * Revision 1.18  2025-01-01 21:35:30+05:30  Cprogrammer
 * use tcpto definitions from tcpto.h
 *
 * Revision 1.17  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.16  2022-10-18 09:24:42+05:30  Cprogrammer
 * converted function proto to ansic
 *
 * Revision 1.15  2021-05-08 12:23:35+05:30  Cprogrammer
 * use /var/indimail/queue if QUEUEDIR is not defined
 *
 * Revision 1.14  2018-01-09 12:37:11+05:30  Cprogrammer
 * removed header hasindimail.h
 *
 * Revision 1.13  2011-07-29 09:30:09+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.12  2007-12-20 13:53:15+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.11  2005-09-05 08:38:52+05:30  Cprogrammer
 * set penalty to min_backoff if min_backoff is negative
 *
 * Revision 1.10  2005-08-23 17:39:34+05:30  Cprogrammer
 * made min_backoff and max_tolerance configurable
 *
 * Revision 1.9  2005-06-29 20:55:02+05:30  Cprogrammer
 * size of buffer changed to TCPTO_BUFSIZ
 *
 * Revision 1.8  2005-06-17 21:51:33+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.7  2004-10-22 20:31:38+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:39:55+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.5  2004-07-17 21:24:46+05:30  Cprogrammer
 * added RCS log
 *
 */
