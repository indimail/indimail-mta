/*
 * $Log: qmail-qmqpc.c,v $
 * Revision 1.21  2020-11-24 13:47:09+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.20  2020-05-11 11:10:41+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.19  2020-04-14 12:31:37+05:30  Cprogrammer
 * fixed controlfile name for timeoutremote
 *
 * Revision 1.18  2017-03-23 20:06:57+05:30  Cprogrammer
 * added option to specifiy qmqpservers on command line
 *
 * Revision 1.17  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.16  2015-08-24 19:08:14+05:30  Cprogrammer
 * replaced ip_scan() with ip4_scan()
 *
 * Revision 1.15  2011-07-04 17:45:05+05:30  Cprogrammer
 * use control_readrandom to pickup up a single IP from outgoingip
 *
 * Revision 1.14  2011-01-11 10:27:37+05:30  Cprogrammer
 * use OUTGOINGIP to override control file outgoingip
 *
 * Revision 1.13  2010-07-24 18:58:41+05:30  Cprogrammer
 * added load distribution logic
 *
 * Revision 1.12  2010-04-06 20:25:20+05:30  Cprogrammer
 * use PORT_QMQP environment variable to configure QMQP port
 *
 * Revision 1.11  2008-07-15 19:57:55+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.10  2008-06-01 15:32:16+05:30  Cprogrammer
 * timeout patch by Eric Huss
 *
 * Revision 1.9  2005-06-17 22:04:38+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.8  2004-10-22 20:28:41+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-22 15:37:23+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.6  2004-10-11 14:38:43+05:30  Cprogrammer
 * use ip from control file outgoingip
 *
 * Revision 1.5  2004-07-17 21:21:08+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "substdio.h"
#include "socket.h"
#include "str.h"
#include "env.h"
#include "scan.h"
#include "getln.h"
#include "stralloc.h"
#include "slurpclose.h"
#include "error.h"
#include "sig.h"
#include "ip.h"
#include "timeoutconn.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "control.h"
#include "fmt.h"
#include "now.h"
#include "variables.h"

#define PORT_QMQP 628

void
die_success()
{
	_exit(0);
}

void
die_perm()
{
	_exit(31);
}

void
nomem()
{
	_exit(51);
}

void
die_read()
{
	if (errno == error_nomem)
		nomem();
	_exit(54);
}

void
die_control()
{
	_exit(55);
}

void
die_socket()
{
	_exit(56);
}

void
die_home()
{
	_exit(61);
}

void
die_temp()
{
	_exit(71);
}

void
die_conn()
{
	_exit(74);
}

void
die_format()
{
	_exit(91);
}

int             lasterror = 55;
int             timeoutremote = 60;
int             timeoutconnect = 10;
int             qmqpfd;

ssize_t
saferead(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	r = timeoutread(timeoutremote, qmqpfd, buf, len);
	if (r <= 0)
		die_conn();
	return r;
}

ssize_t
safewrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	r = timeoutwrite(timeoutremote, qmqpfd, buf, len);
	if (r <= 0)
		die_conn();
	return r;
}

char            buf[1024];
substdio        to = SUBSTDIO_FDBUF(safewrite, -1, buf, sizeof buf);
substdio        from = SUBSTDIO_FDBUF(saferead, -1, buf, sizeof buf);
substdio        envelope = SUBSTDIO_FDBUF(read, 1, buf, sizeof buf);
/*
 * WARNING: can use only one of these at a time! 
 */

stralloc        beforemessage = { 0 };
stralloc        message = { 0 };
stralloc        aftermessage = { 0 };

char            strnum[FMT_ULONG];
stralloc        line = { 0 };

void
getmess()
{
	int             match;

	if (slurpclose(0, &message, 1024) == -1)
		die_read();
	strnum[fmt_ulong(strnum, (unsigned long) message.len)] = 0;
	if (!stralloc_copys(&beforemessage, strnum))
		nomem();
	if (!stralloc_cats(&beforemessage, ":"))
		nomem();
	if (!stralloc_copys(&aftermessage, ","))
		nomem();
	if (getln(&envelope, &line, &match, '\0') == -1)
		die_read();
	if (!match)
		die_format();
	if (line.len < 2)
		die_format();
	if (line.s[0] != 'F')
		die_format();
	strnum[fmt_ulong(strnum, (unsigned long) line.len - 2)] = 0;
	if (!stralloc_cats(&aftermessage, strnum))
		nomem();
	if (!stralloc_cats(&aftermessage, ":"))
		nomem();
	if (!stralloc_catb(&aftermessage, line.s + 1, line.len - 2))
		nomem();
	if (!stralloc_cats(&aftermessage, ","))
		nomem();
	for (;;) {
		if (getln(&envelope, &line, &match, '\0') == -1)
			die_read();
		if (!match)
			die_format();
		if (line.len < 2)
			break;
		if (line.s[0] != 'T')
			die_format();
		strnum[fmt_ulong(strnum, (unsigned long) line.len - 2)] = 0;
		if (!stralloc_cats(&aftermessage, strnum))
			nomem();
		if (!stralloc_cats(&aftermessage, ":"))
			nomem();
		if (!stralloc_catb(&aftermessage, line.s + 1, line.len - 2))
			nomem();
		if (!stralloc_cats(&aftermessage, ","))
			nomem();
	}
}

void
doit(char *server, union v46addr *outip)
{
	int             port_qmqp = -1;
	char           *x;
#ifdef IPV6
	ip6_addr        ip;
#else
	ip_addr         ip;
#endif
	char            ch;

#ifdef IPV6
	if (!ip6_scan(server, &ip))
		return;
	qmqpfd = socket_tcp6();
#else
	if (!ip4_scan(server, &ip))
		return;
	qmqpfd = socket_tcp4();
#endif
	if (qmqpfd == -1)
		die_socket();
	if ((x = env_get("PORT_QMQP")))
		scan_int(x, &port_qmqp);
	else
		port_qmqp = PORT_QMQP;
#ifdef IPV6
	if (timeoutconn6(qmqpfd, &ip, outip, port_qmqp, timeoutconnect) != 0) {
#else
	if (timeoutconn4(qmqpfd, &ip, outip, port_qmqp, timeoutconnect) != 0) {
#endif
		lasterror = 73;
		if (errno == error_timeout)
			lasterror = 72;
		close(qmqpfd);
		return;
	}
	strnum[fmt_ulong(strnum, (unsigned long) (beforemessage.len + message.len + aftermessage.len))] = 0;
	substdio_puts(&to, strnum);
	substdio_puts(&to, ":");
	substdio_put(&to, beforemessage.s, beforemessage.len);
	substdio_put(&to, message.s, message.len);
	substdio_put(&to, aftermessage.s, aftermessage.len);
	substdio_puts(&to, ",");
	substdio_flush(&to);
	for (;;) {
		substdio_get(&from, &ch, 1);
		if (ch == 'K')
			die_success();
		if (ch == 'Z')
			die_temp();
		if (ch == 'D')
			die_perm();
	}
}

stralloc        servers = { 0 };

int
main(int argc, char **argv)
{
	int             i, j, r, server_count, server_no = 0, load_dist = 0;
	char           *x;
	union v46addr   outip;
	struct stat     st;
	stralloc        outgoingip = { 0 };
	stralloc        qmqpfn = {0};

	sig_pipeignore();
	if (chdir(auto_qmail) == -1)
		die_home();
	if (control_init() == -1)
		die_control();
	if (argc == 1 && control_readfile(&servers, "qmqpservers", 0) != 1)
		die_control();
	if (control_readint(&timeoutremote, "timeoutremote") == -1)
		die_control();
	if (control_readint(&timeoutconnect, "timeoutconnect") == -1)
		die_control();
	getmess();
	if ((x = env_get("OUTGOINGIP")) && *x) {
		if (!stralloc_copys(&outgoingip, x))
			nomem();
		r = 1;
	} else
	if (-1 == (r = control_readrandom(&outgoingip, "outgoingip"))) {
		if (errno == error_nomem)
			nomem();
		die_control();
	}
#ifdef IPV6
	if (0 == r && !stralloc_copys(&outgoingip, "::"))
		nomem();
	if (0 == str_diffn(outgoingip.s, "::", 2)) {
		for (i = 0;i < 16;i++)
			outip.ip6.d[i] = 0;
	} else
	if (!ip6_scan(outgoingip.s, &outip.ip6))
		die_format();
#else
	if (0 == r && !stralloc_copys(&outgoingip, "0.0.0.0"))
		nomem();
	if (0 == str_diffn(outgoingip.s, "0.0.0.0", 7)) {
		for (i = 0;i < 4;i++)
			outip.ip.d[i] = 0;
	} else
	if (!ip4_scan(outgoingip.s, &outip.ip))
		die_format();
#endif
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	if (!stralloc_copys(&qmqpfn, controldir))
		nomem();
	if (!stralloc_catb(&qmqpfn, "/qmqpservers\0", 13))
		nomem();
	if (stat(qmqpfn.s, &st) == -1)
		die_control();
	if (st.st_mode & 01000)
		load_dist = 1;
	if (argc == 1) {
		i = 0;
		if (load_dist) {
			server_count = 0;
			for (j = 0; j < servers.len; ++j) {
				if (!servers.s[j]) {
					server_count++;
					i = j + 1;
				}
			}
			server_no = (now() % server_count);
		}
	}
again:
	if (argc > 1) {
		for (i = 1; argv[i]; i++)
			doit(argv[i], &outip);
		_exit (lasterror);
	}
	i = server_count = 0;
	for (j = 0; j < servers.len; ++j) {
		if (!servers.s[j]) {
			if (load_dist) {
				if (server_count == server_no || server_count == -1) {
					doit(servers.s + i, &outip);
					load_dist = 0; /*- fallback to traditional method */
					goto again;
				}
				if (server_count != -1)
					server_count++;
			} else
				doit(servers.s + i, &outip);
			i = j + 1;
		}
	}
	_exit(lasterror);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_qmqpc_c()
{
	static char    *x = "$Id: qmail-qmqpc.c,v 1.21 2020-11-24 13:47:09+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
