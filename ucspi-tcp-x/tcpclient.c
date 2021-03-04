/*
 * $Log: tcpclient.c,v $
 * Revision 1.17  2021-03-04 11:43:16+05:30  Cprogrammer
 * use CERTSDIR for certificates
 * added -m option for matching host with common name
 *
 * Revision 1.16  2021-03-03 22:50:55+05:30  Cprogrammer
 * renamed default certificate to clientcert.pem
 *
 * Revision 1.15  2021-03-03 18:45:24+05:30  Cprogrammer
 * fixed idle timeout
 *
 * Revision 1.14  2021-03-03 17:44:14+05:30  Cprogrammer
 * added client mode feature
 *
 * Revision 1.13  2021-03-03 13:46:07+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.12  2021-03-03 13:43:26+05:30  Cprogrammer
 * added SSL/TLS support
 *
 * Revision 1.11  2020-09-16 20:50:19+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.10  2020-08-03 17:27:39+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.9  2017-03-30 23:00:07+05:30  Cprogrammer
 * prefix rbl with ip6_scan(), ip4_scan() - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.8  2008-07-26 09:45:56+05:30  Cprogrammer
 * fixed compile error
 *
 * Revision 1.7  2008-07-17 23:04:27+05:30  Cprogrammer
 * define opteof on Darwin
 *
 * Revision 1.6  2005-06-11 02:11:54+05:30  Cprogrammer
 * removed blank lines
 *
 * Revision 1.5  2005-06-10 09:13:27+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.4  2004-10-12 00:31:37+05:30  Cprogrammer
 * renamed remoteinfo.h to tcpremoteinfo.h
 *
 * Revision 1.3  2004-09-23 23:12:58+05:30  Cprogrammer
 * include getopt.h
 *
 * Revision 1.2  2003-12-30 00:32:49+05:30  Cprogrammer
 * added prototype for socket_tcpnodelay()
 *
 * Revision 1.1  2003-10-17 21:09:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/param.h>
#include <netdb.h>
#include <sig.h>
#include <getopt.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <uint16.h>
#include <fmt.h>
#include <scan.h>
#include <str.h>
#include <arpa/inet.h>
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#endif
#include <uint16.h>
#include "socket.h"
#include <fd.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <error.h>
#include <strerr.h>
#include "pathexec.h"
#include "timeoutconn.h"
#include "tcpremoteinfo.h"
#include "dns.h"
#include <unistd.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <wait.h>
#include "tls.h"
#ifdef TLS
#include <openssl/ssl.h>
#include <env.h>
#endif

#define FATAL "tcpclient: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: tcpclient.c,v 1.17 2021-03-04 11:43:16+05:30 Cprogrammer Exp mbhangui $";
#endif

extern int      socket_tcpnodelay(int);

void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
usage(void)
{
	strerr_die1x(100, "usage: tcpclient"
#ifdef IPV6
#ifdef TLS
" [ -46hHrRdDqQmv ]\n"
#else
" [ -46hHrRdDqQv ]\n"
#endif
#else
" [ -hHrRdDqQv ]\n"
#endif
" [ -i localip ]\n"
" [ -p localport ]\n"
" [ -l localname ]\n"
" [ -T timeoutconn ]\n"
" [ -t timeoutinfo ]\n"
" [ -a timeoutdata ]\n"
" [ -e timeoutidle ]\n"
#ifdef TLS
" [ -n clientcert ]\n"
" [ -c cafile ] \n"
#endif
" host port [program]");
}

int             verbosity = 1;
int             flagdelay = 1;
int             flagremoteinfo = 1;
int             flagremotehost = 1;
unsigned long   itimeout = 26;
unsigned long   ctimeout[2] = { 2, 58 };
unsigned long   dtimeout = 60, idle_timeout=300;
#ifdef IPV6
int             forcev6 = 0;
char            iplocal[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
char            ipremote[16];
char            ipstr[IP6_FMT];
uint32          netif = 0;
#else
char            iplocal[4] = {0, 0, 0, 0};
char            ipremote[4];
char            ipstr[IP4_FMT];
#endif
uint16          portlocal = 0;
char           *forcelocal = 0;
uint16          portremote;
char           *hostname;
static stralloc addresses;
static stralloc moreaddresses;
static stralloc tmp;
static stralloc fqdn;
char            strnum[FMT_ULONG], strnum2[FMT_ULONG];
char            seed[128];
#ifdef TLS
struct stralloc certfile = {0};
#endif

void
sigterm()
{
	_exit(0);
}

void
sigchld()
{
	int             i, wstat, pid;

	while ((pid = wait_nohang(&wstat)) > 0) {
		strnum[fmt_ulong(strnum, pid)] = 0;
		if (wait_crashed(wstat))
			strerr_warn3("tcpclient: end ", strnum, " status crashed", 0);
		else
		if ((i = wait_exitcode(wstat))) {
			strnum2[fmt_ulong(strnum2, i)] = 0;
			strerr_warn4("tcpclient: end ", strnum, " status ", strnum2, 0);
		}
	}
}

int
do_select(char **argv, int client_mode, int s)
{
	char            buf[512];
	int             wstat, r, pid, pi1[2], pi2[2], fdhigh,
					fdin, fdout, flagexitasp = 0;
	ssize_t         n;
	fd_set          rfds;	/*- File descriptor mask for select -*/
	struct timeval  timeout;

	if (!client_mode) {
		sig_catch(sig_child, sigchld);
		sig_ignore(sig_pipe);
		sig_catch(sig_child, sigchld);
		sig_catch(sig_term, sigterm);
		if (pipe(pi1) != 0 || pipe(pi2) != 0) {
			ssl_free();
			strerr_die2sys(111, FATAL, "unable to create pipe: ");
		}
		switch (pid = fork())
		{
			case -1:
				ssl_free();
				strerr_die2sys(111, FATAL, "fork: ");
			case 0:
				ssl_free();
				sig_uncatch(sig_pipe);
				sig_uncatch(sig_child);
				sig_uncatch(sig_term);
				close(pi1[1]);
				close(pi2[0]);
				if ((fd_move(6, pi1[0]) == -1) || (fd_move(7, pi2[1]) == -1))
					strerr_die2sys(111, FATAL, "unable to set up descriptors: ");
				pathexec(argv);
				strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
				/* Not reached */
				_exit(0);
			default:
				break;
		}
		fdhigh = pi2[0];
		fdin = pi2[0];
		fdout = pi1[1];
	} else {
		pid = -1;
		fdhigh = s;
		fdin = 0;
		fdout = 1;
	}
	while (!flagexitasp) {
		timeout.tv_sec = idle_timeout;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(s, &rfds);
		FD_SET(fdin, &rfds);
		if ((r = select(fdhigh + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			ssl_free();
			strerr_die2sys(111, FATAL, "select: ");
		} else
		if (!r) { /*-timeout */
			timeout.tv_sec = idle_timeout;
			timeout.tv_usec = 0;
			strnum[fmt_ulong(strnum, timeout.tv_sec)] = 0;
			strerr_warn4(FATAL, "idle timeout reached without input [", strnum, " sec]", 0);
			close(s);
			ssl_free();
			_exit(111);
		}
		if (FD_ISSET(s, &rfds)) {
			if ((n = saferead(s, buf, sizeof(buf), dtimeout)) < 0)
				strerr_die2sys(111, FATAL, "unable to read from network: ");
			else
			if (!n) { /*- remote closed socket */
				flagexitasp = 1;
				close(pi1[1]);
				break;
			}
			if ((n = timeoutwrite(dtimeout, fdout, buf, n)) == -1)
				strerr_die2sys(111, FATAL, "unable to write to child: ");
		}
		if (FD_ISSET(fdin, &rfds)) {
			if ((n = timeoutread(dtimeout, fdin, buf, sizeof(buf))) == -1)
				strerr_die2sys(111, FATAL, "unable to read from child: ");
			else
			if (!n) { /*- prog exited */
				flagexitasp = 1;
				if (!client_mode)
					close(pi1[1]);
				break;
			}
			if ((n = safewrite(s, buf, n, dtimeout)) == -1) {
				ssl_free();
				strerr_die2sys(111, FATAL, "unable to write to network: ");
			}
		}
	}
	if (pid > 0) {
		ssl_free();
		if (wait_pid(&wstat, pid) == -1)
			strerr_die2sys(111, FATAL, "unable to get child status: ");
		if (wait_crashed(wstat))
			strerr_die2x(111, FATAL, "child crashed");
		_exit(wait_exitcode(wstat));
	}
	return (close(s)); 
}

int
main(int argc, char **argv)
{
	unsigned long   u;
	char           *x;
#ifdef IPV6
	int             fakev4 = 0;
#endif
	int             opt, j, s, cloop, client_mode = 0, flagssl = 0;
#ifdef TLS
	SSL             *ssl;
	char           *certsdir, *cafile = NULL;
	int             match_cn = 0;
#endif

	dns_random_init(seed);
	close(6);
	close(7);
	sig_ignore(sig_pipe);
#ifdef TLS
	if (!(certsdir = env_get("CERTSDIR")))
		certsdir = "/etc/indimail/certs";
	if (!stralloc_copys(&certfile, certsdir))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_cats(&certfile, "/clientcert.pem"))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_0(&certfile) )
		strerr_die2x(111, FATAL, "out of memory");
#endif
#ifdef IPV6
#ifdef TLS
	while ((opt = getopt(argc, argv, "46dDvqQhHrRi:p:t:T:l:I:n:c:a:e:m")) != opteof)
#else
	while ((opt = getopt(argc, argv, "46dDvqQhHrRi:p:t:T:l:I:")) != opteof)
#endif
#else
#ifdef TLS
	while ((opt = getopt(argc, argv, "dDvqQhHrRi:p:t:T:l:n:c:a:e:m")) != opteof)
#else
	while ((opt = getopt(argc, argv, "dDvqQhHrRi:p:t:T:l:m")) != opteof)
#endif
#endif
	{
		switch (opt)
		{
#ifdef IPV6
		case '4':
			noipv6 = 1;
			break;
		case '6':
			forcev6 = 1;
			break;
		case 'I':
			netif = socket_getifidx(optarg);
			break;
#endif
		case 'd':
			flagdelay = 1;
			break;
		case 'D':
			flagdelay = 0;
			break;
		case 'v':
			verbosity = 2;
			break;
		case 'q':
			verbosity = 0;
			break;
		case 'Q':
			verbosity = 1;
			break;
		case 'l':
			forcelocal = optarg;
			break;
		case 'H':
			flagremotehost = 0;
			break;
		case 'h':
			flagremotehost = 1;
			break;
		case 'R':
			flagremoteinfo = 0;
			break;
		case 'r':
			flagremoteinfo = 1;
			break;
		case 't':
			scan_ulong(optarg, &itimeout);
			break;
		case 'T':
			j = scan_ulong(optarg, &ctimeout[0]);
			if (optarg[j] == '+')
				++j;
			scan_ulong(optarg + j, &ctimeout[1]);
			break;
		case 'i':
#ifdef IPV6
			if (!rblip6_scan(optarg, iplocal))
#else
			if (!rblip4_scan(optarg, iplocal))
#endif
				usage();
			break;
		case 'p':
			scan_ulong(optarg, &u);
			portlocal = u;
			break;
		case 'e':
			scan_ulong(optarg, &u);
			idle_timeout = u;
			break;
		case 'a':
			scan_ulong(optarg, &u);
			dtimeout = u;
			break;
#ifdef TLS
		case 'n':
			if (!optarg)
				usage();
			if (*optarg  && (!stralloc_copys(&certfile, optarg) || !stralloc_0(&certfile)))
				strerr_die2x(111, FATAL, "out of memory");
			flagssl = access(certfile.s, F_OK) ? 0 : 1;
			break;
		case 'c':
			cafile = optarg;
			break;
		case 'm':
			match_cn = 1;
			break;
#endif
		default:
			usage();
		}
	}
	argv += optind;
	if (!verbosity)
		subfderr->fd = -1;
	if (!(hostname = *argv))
		usage();
	if (!hostname[0] || str_equal(hostname,"0"))
#ifdef IPV6
		hostname = (noipv6 ? "127.0.0.1" : "::1");
#else
		hostname = "127.0.0.1";
#endif
	if (!(x = *++argv))
		usage();
	if (!x[scan_ulong(x, &u)])
		portremote = u;
	else {
		struct servent *se;

		if (!(se = getservbyname(x, "tcp")))
			strerr_die3x(111, FATAL, "unable to figure out port number for ", x);
		portremote = ntohs(se->s_port);
		/*- i continue to be amazed at the stupidity of the s_port interface */
	}
	if (!*++argv)
		client_mode = 1;
	if (!stralloc_copys(&tmp, hostname))
		nomem();
#ifdef IPV6
	if (dns_ip6_qualify(&addresses, &fqdn, &tmp) == -1)
		strerr_die4sys(111, FATAL, "temporarily unable to figure out IP address for ", hostname, ": ");
	if (addresses.len < 16)
		strerr_die3x(111, FATAL, "no IP address for ", hostname);
	if (addresses.len == 16)
#else
	if (dns_ip4_qualify(&addresses, &fqdn, &tmp) == -1)
		strerr_die4sys(111, FATAL, "temporarily unable to figure out IP address for ", hostname, ": ");
	if (addresses.len < 4)
		strerr_die3x(111, FATAL, "no IP address for ", hostname);
	if (addresses.len == 4)
#endif
	{
		ctimeout[0] += ctimeout[1];
		ctimeout[1] = 0;
	}
	for (cloop = 0; cloop < 2; ++cloop) {
		if (!stralloc_copys(&moreaddresses, ""))
			nomem();
#ifdef IPV6
		for (j = 0; j + 16 <= addresses.len; j += 4)
#else
		for (j = 0; j + 4 <= addresses.len; j += 4)
#endif
		{
#ifdef IPV6
			if ((s = socket_tcp6()) == -1)
#else
			if ((s = socket_tcp()) == -1)
#endif
				strerr_die2sys(111, FATAL, "unable to create socket: ");
#ifdef IPV6
			if (socket_bind6(s, iplocal, portlocal, netif) == -1)
#else
			if (socket_bind4(s, iplocal, portlocal) == -1)
#endif
				strerr_die2sys(111, FATAL, "unable to bind socket: ");
#ifdef IPV6
			if (timeoutconn6(s, addresses.s + j, portremote, ctimeout[cloop], netif) == 0)
#else
			if (timeoutconn(s, addresses.s + j, portremote, ctimeout[cloop]) == 0)
#endif
				goto CONNECTED;
			close(s);
			if (!cloop && ctimeout[1] && (errno == error_timeout)) {
#ifdef IPV6
				if (!stralloc_catb(&moreaddresses, addresses.s + j, 16))
#else
				if (!stralloc_catb(&moreaddresses, addresses.s + j, 4))
#endif
					nomem();
			} else {
				strnum[fmt_ulong(strnum, portremote)] = 0;
#ifdef IPV6
				if (ip6_isv4mapped(addresses.s + j))
					ipstr[ip4_fmt(ipstr, addresses.s + j + 12)] = 0;
				else
					ipstr[ip6_fmt(ipstr, addresses.s + j)] = 0;
#else
				ipstr[ip4_fmt(ipstr, addresses.s + j)] = 0;
#endif
				strerr_warn5("tcpclient: unable to connect to ", ipstr, " port ", strnum, ": ", &strerr_sys);
			}
		}
		if (!stralloc_copy(&addresses, &moreaddresses))
			nomem();
	}
	_exit(111);

CONNECTED:
	if (!flagdelay)
		socket_tcpnodelay(s);	/*- if it fails, bummer */
#ifndef IPV6
	if (!pathexec_env("PROTO", "TCP"))
		nomem();
	if (socket_local4(s, iplocal, &portlocal) == -1)
#else
	if (socket_local6(s, iplocal, &portlocal, &netif) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to get local address: ");
#ifdef IPV6
	if (!forcev6 && (ip6_isv4mapped(iplocal) || byte_equal(iplocal, 16, (char *) V6any)))
		fakev4 = 1;
	if (!pathexec_env("PROTO", fakev4 ? "TCP" : "TCP6"))
		nomem();
#endif
	strnum[fmt_ulong(strnum, portlocal)] = 0;
	if (!pathexec_env("TCPLOCALPORT", strnum))
		nomem();
#ifdef IPV6
	if (fakev4)
		ipstr[ip4_fmt(ipstr, iplocal + 12)] = 0;
	else
		ipstr[ip6_fmt(ipstr, iplocal)] = 0;
#else
	ipstr[ip4_fmt(ipstr, iplocal)] = 0;
#endif
	if (!pathexec_env("TCPLOCALIP", ipstr))
		nomem();
#ifdef IPV6
	if (!(x = forcelocal) && dns_name6(&tmp, iplocal) == 0)
#else
	if (!(x = forcelocal) && dns_name4(&tmp, iplocal) == 0)
#endif
	{
		if (!stralloc_0(&tmp))
			nomem();
		x = tmp.s;
	}
	if (!pathexec_env("TCPLOCALHOST", x))
		nomem();
#ifdef IPV6
	if (socket_remote6(s, ipremote, &portremote, &netif) == -1)
#else
	if (socket_remote4(s, ipremote, &portremote) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to get remote address: ");
	strnum[fmt_ulong(strnum, portremote)] = 0;
	if (!pathexec_env("TCPREMOTEPORT", strnum))
		nomem();
#ifdef IPV6
	if (fakev4)
		ipstr[ip4_fmt(ipstr, ipremote + 12)] = 0;
	else
		ipstr[ip6_fmt(ipstr, ipremote)] = 0;
#else
	ipstr[ip4_fmt(ipstr, ipremote)] = 0;
#endif
	if (!pathexec_env("TCPREMOTEIP", ipstr))
		nomem();
	if (verbosity >= 2)
		strerr_warn4("tcpclient: connected to ", ipstr, " port ", strnum, 0);
	x = 0;
#ifdef IPV6
	if (flagremotehost && dns_name6(&tmp, ipremote) == 0)
#else
	if (flagremotehost && dns_name4(&tmp, ipremote) == 0)
#endif
	{
		if (!stralloc_0(&tmp))
			nomem();
		x = tmp.s;
	}
	if (!pathexec_env("TCPREMOTEHOST", x))
		nomem();
	x = 0;
#ifdef IPV6
	if (flagremoteinfo && remoteinfo6(&tmp, ipremote, portremote, iplocal,
		portlocal, itimeout, netif) == 0)
#else
	if (flagremoteinfo && remoteinfo(&tmp, ipremote, portremote, iplocal,
		portlocal, itimeout) == 0)
#endif
	{
		if (!stralloc_0(&tmp))
			nomem();
		x = tmp.s;
	}
	if (!pathexec_env("TCPREMOTEINFO", x))
		nomem();
#ifdef TLS
	if (flagssl) {
		if (!(ssl = tls_init(s, certfile.s, cafile)))
			_exit(111);
		if (tls_connect(ssl, match_cn ? hostname : 0) == -1)
			_exit(111);
	}
#endif
	if (client_mode || flagssl)
		_exit(do_select(argv, client_mode, s));
	else {
		if (fd_move(6, s) == -1)
			strerr_die2sys(111, FATAL, "unable to set up descriptor 6: ");
		if (fd_copy(7, 6) == -1)
			strerr_die2sys(111, FATAL, "unable to set up descriptor 7: ");
		sig_uncatch(sig_pipe);
		pathexec(argv);
		strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	}
	/* Not reached */
	return (0);
}

void
getversion_tcpclient_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
