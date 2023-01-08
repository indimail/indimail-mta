/*
 * $Id: tcpclient.c,v 1.28 2023-01-08 07:47:25+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netdb.h>
#include <sig.h>
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
#include <uint16.h>
#include <fd.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <error.h>
#include <strerr.h>
#include "timeoutconn.h"
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <wait.h>
#include <openreadclose.h>
#include <noreturn.h>
#ifdef TLS
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <case.h>
#include <ctype.h>
#include <env.h>
#include <getln.h>
#include <ndelay.h>
#include <tls.h>
#endif
#include "ip4.h"
#ifdef IPV6
#include "ip6.h"
#endif
#include "socket.h"
#include "upathexec.h"
#include "tcpremoteinfo.h"
#include "dns.h"

#define FATAL "tcpclient: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: tcpclient.c,v 1.28 2023-01-08 07:47:25+05:30 Cprogrammer Exp mbhangui $";
#endif

extern int      socket_tcpnodelay(int);

no_return void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
usage(void)
{
	strerr_die1x(100, "usage: tcpclient"
#ifdef IPV6
#ifdef TLS
	 " [ -46hHrRdDqQmvz ]\n"
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
#ifdef TLS
	 " [ -n clientcert ]\n"
	 " [ -C certdir ] \n"
	 " [ -c cafile ] \n"
	 " [ -L crlfile ] \n"
	 " [ -s starttlsType (smtp|pop3) ]\n"
	 " [ -f cipherlist ]\n"
	 " [ -M TLS method ] \n"
#endif
	 " host port [program]");
}

static int      verbosity = 1;
static int      flagdelay = 1;
static int      flagremoteinfo = 1;
static int      flagremotehost = 1;
typedef unsigned long my_ulong;
static my_ulong itimeout = 26;           /* timeoutinfo -t option */
static my_ulong ctimeout[2] = { 2, 58 }; /* timeoutconn -T option */
static my_ulong dtimeout = 300;          /* timeoutdata -a option */
#ifdef IPV6
static int      forcev6;
static char     iplocal[16] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
static char     ipremote[16];
static char     ipstr[IP6_FMT];
static uint32   netif;
#else
static char     iplocal[4] = {0, 0, 0, 0};
static char     ipremote[4];
static char     ipstr[IP4_FMT];
#endif
static uint16   portlocal;
static uint16   portremote;
static char    *forcelocal;
static char    *hostname;
static stralloc addresses;
static stralloc moreaddresses;
static stralloc tmp;
static stralloc fqdn;
#ifdef TLS
static stralloc tls_server_version, tls_client_version;
static int      provide_data = 0;
#endif
static char     strnum[FMT_ULONG];
static char     seed[128];
#ifdef TLS
static struct stralloc certfile, cafile, crlfile;
struct stralloc saciphers;
#endif

no_return void
sigterm()
{
	_exit(0);
}

void
sigchld()
{
	int             i, wstat;
	pid_t           pid;
	char            tmp1[FMT_ULONG], tmp2[FMT_ULONG];

	while ((pid = wait_nohang(&wstat)) > 0) {
		tmp1[fmt_ulong(tmp1, pid)] = 0;
		if (wait_stopped(wstat) || wait_continued(wstat)) {
			i = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("tcpclient: end ", tmp1, wait_stopped(wstat) ? " stopped by signal " : " continued by signal ", tmp2, 0);
		} else
		if (wait_signaled(wstat)) {
			i = wait_termsig(wstat);
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("tcpclient: end ", tmp1, " killed by signal ", tmp2, 0);
		} else
		if ((i = wait_exitcode(wstat))) {
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("tcpclient: end ", tmp1, " status ", tmp2, 0);
		}
	}
}

#ifdef TLS
void
write_provider_data(SSL *ssl, int writefd, int readfd)
{
	int             i;
	ssize_t         n;

	if (!stralloc_copyb(&tmp, "TLS Server=", 11) ||
			!stralloc_cat(&tmp, &tls_server_version) ||
			!stralloc_catb(&tmp, ", Client=", 9) ||
			!stralloc_cat(&tmp, &tls_client_version) ||
			!stralloc_catb(&tmp, ", Ciphers=", 10) ||
			!stralloc_cats(&tmp, SSL_CIPHER_get_name(SSL_get_current_cipher(ssl))))
		strerr_die2x(111, FATAL, "out of memory");
	i = tmp.len;
	if (write(writefd, (char *) &i, sizeof(int)) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider data length: ");
	if (write(writefd, tmp.s, tmp.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider data: ");
	if ((n = timeoutread(5, readfd, (char *) &i, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider ACK: ");
	return;
}

void
read_provider_data(stralloc *t, int readfd, int writefd)
{
	int             i;
	ssize_t         n;

	if ((n = timeoutread(dtimeout, readfd, (char *) &i, sizeof(int))) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider data length: ");
	if (!stralloc_ready(t, t->len + i))
		strerr_die2x(111, FATAL, "out of memory");
	if ((n = timeoutread(dtimeout, readfd, t->s + t->len, i)) == -1)
		strerr_die2sys(111, FATAL, "unable to read provider data: ");
	else
	if (n)
		t->len += i;
	if ((n = timeoutwrite(dtimeout, writefd, (char *) &i, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider ACK: ");
	return;
}
#endif

int
#ifdef TLS
do_select(char **argv, SSL *ssl, int flag_tcpclient, int flagssl, int s)
#else
do_select(char **argv, int flag_tcpclient, int s)
#endif
{
	int             wstat, r, pi1[2], pi2[2], fdin, fdout;
	pid_t           pid;

	if (flag_tcpclient) {
		sig_catch(sig_child, sigchld);
		sig_catch(sig_term, sigterm);
		if (pipe(pi1) != 0 || pipe(pi2) != 0)
			strerr_die2sys(111, FATAL, "unable to create pipe: ");
		switch (pid = fork())
		{
		case -1:
			strerr_die2sys(111, FATAL, "fork: ");
		case 0:
			close(s);
			sig_uncatch(sig_pipe);
			sig_uncatch(sig_child);
			sig_uncatch(sig_term);
#ifdef TLS
			if (flagssl) {
				if (!stralloc_copyb(&tmp, "tcpclient, ", 11))
					strerr_die2x(111, FATAL, "out of memory");
				if (provide_data)
					read_provider_data(&tmp, pi2[0], pi1[1]);
				if (!stralloc_0(&tmp))
					strerr_die2x(111, FATAL, "out of memory");
				if (!env_put2("TLS_PROVIDER", tmp.s))
					strerr_die2x(111, FATAL, "out of memory");
			}
#endif
			close(pi1[1]);
			close(pi2[0]);
			if ((fd_move(6, pi1[0]) == -1) || (fd_move(7, pi2[1]) == -1))
				strerr_die2sys(111, FATAL, "unable to set up descriptors: ");
			upathexec(argv);
			strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
			/* Not reached */
			_exit(0);
		default:
			break;
		}
#ifdef TLS
		if (provide_data)
			write_provider_data(ssl, pi2[1], pi1[0]);
#endif
		close(pi1[0]);
		close(pi2[1]);
		fdin = pi2[0];
		fdout = pi1[1];
	} else {
		fdin = 0;
		fdout = 1;
	}
	if ((r = translate(s, s, fdout, fdin, dtimeout)))
		strerr_warn1("tcpclient: translate returned non-zero: ", &strerr_sys);
	if (flag_tcpclient) {
		if (wait_pid(&wstat, pid) == -1)
			strerr_die2sys(111, FATAL, "unable to get child status: ");
		if (wait_crashed(wstat))
			strerr_die2x(111, FATAL, "child crashed");
		_exit(wait_exitcode(wstat));
	}
#ifdef TLS
	ssl_free();
#endif
	return (close(s)); 
}

#ifdef TLS
int
do_starttls(int sfd, enum starttls stls, char *clientcert, int verbose)
{
	char            inbuf[512];
	int             match;
	unsigned long   code;
	static stralloc line = { 0 };
	struct substdio ssin;

	substdio_fdbuf(&ssin, read, sfd, inbuf, sizeof(inbuf));
	switch (stls)
	{
	case smtp:
		/*- read greeting */
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "getln: read-smtpd: ");
		if (!line.len || !match)
			strerr_die2x(111, FATAL, "failed to get greeting");
		if (verbose && write(1, line.s, line.len) == -1)
			strerr_die2sys(111, FATAL, "unable to write to network: ");
		line.s[line.len - 1] = 0;
		scan_ulong(line.s, &code);
		if (code != 220)
			strerr_die2x(111, FATAL, "connected but greeting failed");
		/*- issue STARTTLS command and check response */
		if (tlswrite(sfd, "STARTTLS\r\n", 10, dtimeout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to network: ");
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "getln: read-smtpd: ");
		if (!line.len || !match)
			strerr_die4x(111, FATAL, "NO TLS achived while ", clientcert, " exists");
		line.s[line.len - 1] = 0;
		scan_ulong(line.s, &code);
		if (code != 220)
			strerr_die4x(111, FATAL, "STARTTLS rejected while ", clientcert, " exists");
		ssin.p = 0;
		substdio_flush(&ssin);
		break;
	case pop3:
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "getln: read-pop3d: ");
		if (tlswrite(sfd, "STLS\r\n", 6, dtimeout) == -1)
			strerr_die2sys(111, FATAL, "unable to write to network: ");
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "getln: read-pop3d: ");
		if (!line.len || !match)
			strerr_die4x(111, FATAL, "NO TLS achived while ", clientcert, " exists");
		if (!case_startb(line.s, 3, "+OK"))
			strerr_die4x(111, FATAL, "STLS rejected while ", clientcert, " exists");
		break;
	default:
		break;
	}
	return 0;
}
#endif

int
main(int argc, char **argv)
{
	unsigned long   u;
	char           *x;
	struct stralloc options = {0};
#ifdef IPV6
	int             fakev4 = 0;
#endif
	int             opt, j, s, cloop, flag_tcpclient = 0, flagssl = 0;
#ifdef TLS
	SSL_CTX        *ctx = NULL;
	SSL            *ssl = NULL;
	char           *certdir, *ciphers = NULL,
				   *cipherfile = NULL, *tls_method = NULL;
	enum starttls   stls = unknown;
	int             match_cn = 0;
	struct stat     st;
#endif

	dns_random_init(seed);
	close(6);
	close(7);
	sig_ignore(sig_pipe);
	if (!stralloc_copys(&options, "dDvqQhHrRi:p:t:T:l:a:"))
		strerr_die2x(111, FATAL, "out of memory");
#ifdef IPV6
	if (!stralloc_cats(&options, "46I:"))
		strerr_die2x(111, FATAL, "out of memory");
#endif
#ifdef TLS
	if (!stralloc_cats(&options, "n:c:s:mf:M:L:z"))
		strerr_die2x(111, FATAL, "out of memory");
#endif
	if (!stralloc_0(&options))
		strerr_die2x(111, FATAL, "out of memory");
	while ((opt = getopt(argc, argv, options.s)) != opteof) {
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
		case 'i':
			if (!rblip6_scan(optarg, iplocal))
				usage();
			break;
#else
		case 'i':
			if (!rblip4_scan(optarg, iplocal))
				usage();
			break;
#endif
#ifdef TLS
		case 'n':
			flagssl = 1;
			if (*optarg && (!stralloc_copys(&certfile, optarg) || !stralloc_0(&certfile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'c':
			if (*optarg && (!stralloc_copys(&cafile, optarg) || !stralloc_0(&cafile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'L':
			if (*optarg && (!stralloc_copys(&crlfile, optarg) || !stralloc_0(&crlfile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'C':
			certdir = optarg;
			break;
		case 's':
			if (case_diffs(optarg, "smtp") && case_diffs(optarg, "pop3"))
				usage();
			if (!case_diffs(optarg, "smtp"))
				stls = smtp;
			else
			if (!case_diffs(optarg, "pop3"))
				stls = pop3;
			break;
		case 'm':
			match_cn = 1;
			break;
		case 'f':
			cipherfile = optarg;
			break;
		case 'M':
			tls_method = optarg;
			break;
		case 'z':
			provide_data = 1;
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
		case 'p':
			scan_ulong(optarg, &u);
			portlocal = u;
			break;
		case 'a':
			scan_ulong(optarg, &u);
			dtimeout = u;
			break;
		default:
			usage();
		} /*- switch (opt) */
	} /*- while ((opt = getopt(argc, argv, options.s)) != opteof) */
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
	if (*++argv)
		flag_tcpclient = 1;
#ifdef TLS
	if (!(certdir = env_get("CERTDIR")))
		certdir = "/etc/indimail/certs";
	if (flagssl && !certfile.len) {
		if (!(x = env_get("TLS_CERTFILE")))
			x = "clientcert.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&certfile, certdir) ||
					!stralloc_append(&certfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&certfile, x) ||
				!stralloc_0(&certfile))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (verbosity >= 2) {
#ifdef TLS
		if (flagssl)
			strerr_warn2("tcpclient: using certificate ", certfile.s, 0);
		else
#endif
			strerr_warn1("tcpclient: using un-encrypted connection", 0);
	}
	if (!cafile.len) {
		if (!(x = env_get("SERVERCA")))
			x = "serverca.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&cafile, certdir) ||
					!stralloc_append(&cafile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&cafile, x) ||
				!stralloc_0(&cafile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(cafile.s, R_OK))
			cafile.len = 0;
	}
	if (!crlfile.len) {
		if (!(x = env_get("SERVERCRL")))
			x = "servercrl.pem";
		if (*x != '.' && *x != '/') {
			if (!stralloc_copys(&crlfile, certdir) ||
					!stralloc_append(&crlfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&crlfile, x) ||
				!stralloc_0(&crlfile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(crlfile.s, R_OK))
			crlfile.len = 0;
	}
#endif
#ifdef TLS
	if (!flagssl && stls != unknown)
		strerr_die2x(100, FATAL, "STARTTLS options require Certificates");
#endif
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
	if (!upathexec_env("PROTO", "TCP"))
		nomem();
	if (socket_local4(s, iplocal, &portlocal) == -1)
#else
	if (socket_local6(s, iplocal, &portlocal, &netif) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to get local address: ");
#ifdef IPV6
	if (!forcev6 && (ip6_isv4mapped(iplocal) || byte_equal(iplocal, 16, (char *) V6any)))
		fakev4 = 1;
	if (!upathexec_env("PROTO", fakev4 ? "TCP" : "TCP6"))
		nomem();
#endif
	strnum[fmt_ulong(strnum, portlocal)] = 0;
	if (!upathexec_env("TCPLOCALPORT", strnum))
		nomem();
#ifdef IPV6
	if (fakev4)
		ipstr[ip4_fmt(ipstr, iplocal + 12)] = 0;
	else
		ipstr[ip6_fmt(ipstr, iplocal)] = 0;
#else
	ipstr[ip4_fmt(ipstr, iplocal)] = 0;
#endif
	if (!upathexec_env("TCPLOCALIP", ipstr))
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
	if (!upathexec_env("TCPLOCALHOST", x))
		nomem();
#ifdef IPV6
	if (socket_remote6(s, ipremote, &portremote, &netif) == -1)
#else
	if (socket_remote4(s, ipremote, &portremote) == -1)
#endif
		strerr_die2sys(111, FATAL, "unable to get remote address: ");
	strnum[fmt_ulong(strnum, portremote)] = 0;
	if (!upathexec_env("TCPREMOTEPORT", strnum))
		nomem();
#ifdef IPV6
	if (fakev4)
		ipstr[ip4_fmt(ipstr, ipremote + 12)] = 0;
	else
		ipstr[ip6_fmt(ipstr, ipremote)] = 0;
#else
	ipstr[ip4_fmt(ipstr, ipremote)] = 0;
#endif
	if (!upathexec_env("TCPREMOTEIP", ipstr))
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
	if (!upathexec_env("TCPREMOTEHOST", x))
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
	if (!upathexec_env("TCPREMOTEINFO", x))
		nomem();
#ifdef TLS
	if (flagssl) {
		if (cipherfile) {
			if (lstat(cipherfile, &st) == -1)
				strerr_die4sys(111, FATAL, "lstat: ", cipherfile, ": ");
			if (openreadclose(cipherfile, &saciphers, st.st_size) == -1)
				strerr_die3sys(111, FATAL, cipherfile, ": ");
			if (saciphers.s[saciphers.len - 1] == '\n')
				saciphers.s[saciphers.len - 1] = 0;
			else
			if (!stralloc_0(&saciphers))
				strerr_die2x(111, FATAL, "out of memory");
			for (ciphers = saciphers.s; *ciphers; ciphers++)
				if (isspace(*ciphers)) {
					*ciphers = 0;
					break;
				}
			ciphers = saciphers.s;
		} else
		if (!(ciphers = env_get("TLS_CIPHER_LIST")))
			ciphers = "PROFILE=SYSTEM";
		if (!(ctx = tls_init(tls_method, certfile.s,
				cafile.len ? cafile.s : NULL, crlfile.len ? crlfile.s : NULL,
				ciphers, client)))
			_exit(111);
		if (!(ssl = tls_session(ctx, s)))
			_exit(111);
		SSL_CTX_free(ctx);
		ctx = NULL;
		if (stls != unknown)
			do_starttls(s, stls, certfile.s, !flag_tcpclient);
		if (!stralloc_copys(&tls_server_version, SSL_get_version(ssl)) ||
				!stralloc_0(&tls_server_version))
			strerr_die2x(111, FATAL, "out of memory");
		tls_server_version.len--;
		if (tls_connect(ctimeout[0] + ctimeout[1], s, s, ssl, match_cn ? hostname : 0) == -1)
			_exit(111);
		if (!stralloc_copys(&tls_client_version, SSL_get_version(ssl)) ||
				!stralloc_0(&tls_client_version))
			strerr_die2x(111, FATAL, "out of memory");
		tls_client_version.len--;
	}
#endif
	if (!flag_tcpclient || flagssl) {
#ifdef TLS
		if (stls != unknown) {
			set_essential_fd(0);
			if (ndelay_on(s) == -1)
				strerr_die2sys(111, FATAL, "unable to set up non-blocking mode for socket: ");
		}
#endif
#ifdef TLS
		_exit(do_select(argv, ssl, flag_tcpclient, flagssl, s));
#else
		_exit(do_select(argv, flag_tcpclient, s));
#endif
	} else {
		if (fd_move(6, s) == -1)
			strerr_die2sys(111, FATAL, "unable to set up descriptor 6: ");
		if (fd_copy(7, 6) == -1)
			strerr_die2sys(111, FATAL, "unable to set up descriptor 7: ");
		sig_uncatch(sig_pipe);
		upathexec(argv);
		strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	}
	/* Not reached */
	return (0);
}

#ifndef lint
void
getversion_tcpclient_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
#endif

/*
 * $Log: tcpclient.c,v $
 * Revision 1.28  2023-01-08 07:47:25+05:30  Cprogrammer
 * added -z option to turn on setting of TLS_PROVIDER env variable
 * replace internal TLS function with TLS functions from libqmail
 *
 * Revision 1.27  2022-12-27 09:01:23+05:30  Cprogrammer
 * fixed bug in setting command line args
 *
 * Revision 1.26  2022-12-27 08:04:27+05:30  Cprogrammer
 * added -L option to specify CRL
 * set TLS_PROVIDER env variable
 * do TLS/SSL session only when -n is specified
 *
 * Revision 1.25  2022-12-25 19:38:13+05:30  Cprogrammer
 * added -C option to specify certdir
 *
 * Revision 1.24  2022-12-23 10:35:36+05:30  Cprogrammer
 * added -M option to set TLS / SSL client/server method
 * added -f option to specify ciphers from a file
 *
 * Revision 1.23  2022-12-13 20:22:26+05:30  Cprogrammer
 * display diagnostic on exit status
 *
 * Revision 1.22  2022-07-01 19:57:38+05:30  Cprogrammer
 * use unencrypted connection if argument to -n is an empty string
 *
 * Revision 1.21  2021-08-30 12:47:59+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.20  2021-05-12 21:04:42+05:30  Cprogrammer
 * replaced pathexec with upathexec
 *
 * Revision 1.19  2021-03-10 18:23:43+05:30  Cprogrammer
 * use set_essential_fd() to avoid deadlock
 *
 * Revision 1.18  2021-03-09 08:37:06+05:30  Cprogrammer
 * removed unnecessary initializations and type casts
 *
 * Revision 1.17  2021-03-09 00:54:22+05:30  Cprogrammer
 * use non-blocking io
 * use translate() functions from tls.c instead of select()
 *
 * Revision 1.16  2021-03-08 15:28:56+05:30  Cprogrammer
 * removed imap options
 *
 * Revision 1.15  2021-03-07 21:15:44+05:30  Cprogrammer
 * call ssl_free() as last step on do_select()
 *
 * Revision 1.14  2021-03-07 18:09:06+05:30  Cprogrammer
 * added starttls for pop3 and imap
 *
 * Revision 1.13  2021-03-06 23:12:11+05:30  Cprogrammer
 * make specifying certificate mandatory for starttls option
 *
 * Revision 1.12  2021-03-06 21:24:02+05:30  Cprogrammer
 * added SSL/TLS and opportunistic TLS
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
