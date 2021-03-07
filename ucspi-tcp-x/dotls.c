/*
 * $Log: dotls.c,v $
 * Revision 1.2  2021-03-07 18:11:04+05:30  Cprogrammer
 * added opportunistic TLS for pop3
 *
 * Revision 1.1  2021-03-07 08:13:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef TLS
#include <unistd.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <openssl/ssl.h>
#include <stralloc.h>
#include <scan.h>
#include <fmt.h>
#include <substdio.h>
#include <strerr.h>
#include <case.h>
#include <env.h>
#include <fd.h>
#include <sig.h>
#include <str.h>
#include <alloc.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <getln.h>
#include <timeoutwrite.h>
#include "pathexec.h"
#include "tls.h"

#define FATAL         "sslerator: fatal: "
#define HUGECAPATEXT  5000

#ifndef	lint
static char     sccsid[] = "$Id: dotls.c,v 1.2 2021-03-07 18:11:04+05:30 Cprogrammer Exp mbhangui $";
#endif

int             do_bulk();
int             smtp_ehlo(char *, char *, int);
int             func_unimpl(char *, char *, int);
int             do_tls();
int             do_quit();
int             pop3_capa(char *, char *, int);
void            flush_data();
void            flush_io();
void            flush();

struct substdio ssin, ssto, smtpin, smtpto;
unsigned long   dtimeout = 60;
stralloc        certfile = { 0 };
stralloc        capatext = { 0 };
stralloc        sauninit = { 0 };
stralloc        line = { 0 };
char            strnum[FMT_ULONG];
int             linemode = 1;
struct scommd
{
	char           *text;
	int             (*fun) (char *, char *, int);
	void            (*flush) (void);
};

void
usage(void)
{
	strerr_die1x(100, "usage: sslerator"
		 " [ -CT ]\n"
		 " [ -h host ]\n"
		 " [ -t timeoutdata ]\n"
		 " [ -n clientcert ]\n"
		 " [ -c cafile ] \n"
		 " [ -s starttlsType (smpt|pop3) ]\n"
		 " program");
}

struct scommd smtpcommands[] = {
	{ "data", do_bulk, flush_data },
	{ "ehlo", smtp_ehlo, flush },
	{ "starttls", do_tls, flush_io },
	{ "quit", do_quit, flush},
	{ 0, func_unimpl, flush }
};

struct scommd pop3commands[] = {
	{ "retr", do_bulk, flush_data },
	{ "capa", pop3_capa, flush },
	{ "stls", do_tls, flush_io },
	{ "quit", do_quit, flush},
	{ 0, func_unimpl, flush }
};

int
do_commands(enum starttls stls, SSL *ssl, substdio *ss, int rfd, int wfd)
{
	int             i, j, found;
	char           *arg;
	char            ch;
	struct scommd   *c;
	static stralloc cmd = { 0 };

	if (!stralloc_copys(&cmd, ""))
		return -1;
	switch (stls)
	{
	case smtp:
		c = smtpcommands;
		break;
	case pop3:
		c = pop3commands;
		break;
	default:
		return -1;
	}
	for (;;) {
		if ((i = substdio_get(ss, &ch, 1)) != 1) {
			return i;
		}
		if (ch == '\n')
			break;
		if (!ch)
			ch = '\n';
		if (!stralloc_append(&cmd, &ch))
			return -1;
	}
	if (cmd.len > 0 && cmd.s[cmd.len - 1] == '\r')
		--cmd.len;
	if (!stralloc_0(&cmd))
		return -1;
	cmd.len--;
	i = str_chr(cmd.s, ' ');
	arg = cmd.s + i;
	found = 0;
	while (*arg == ' ') {
		found = 1;
		++arg;
	}
	cmd.s[i] = 0;
	for (j = 0; c[j].text; ++j) {
		if (case_equals(c[j].text, cmd.s))
			break;
	}
	if (found) /*- if we had replaced space, put it back */
		cmd.s[i] = ' ';
	i = c[j].fun(arg, cmd.s, cmd.len);
	if (c[j].flush)
		c[j].flush();
	if (i) { /*- only do_tls() returns 1 */
		switch (stls)
		{
		case smtp:
			if (substdio_put(&ssto, "220 ready for tls\r\n", 19) == -1 || substdio_flush(&ssto) == -1)
				strerr_die2sys(111, FATAL, "write: ");
			break;
		case pop3:
			if (substdio_put(&ssto, "+OK Begin SSL/TLS negotiation now.\r\n", 37) == -1 ||
					substdio_flush(&ssto) == -1)
			break;
		default:
			break;
		}
		if (tls_accept(ssl))
			strerr_die2x(111, FATAL, "unable to accept SSL connection");
		i = translate(ssl, wfd, rfd, dtimeout); /*- returns only when one side closes */
		ssl_free();
		_exit(i);
	}
	return 0;
}

void
get1(char *ch)
{
	substdio_get(&smtpin, ch, 1);
	if (*ch != '\r' && capatext.len < HUGECAPATEXT &&
			!stralloc_append(&capatext, ch))
		strerr_die2x(111, FATAL, "out of memory");
}

unsigned long
get3()
{
	char            str[4];
	int             i;
	unsigned long   code;

	substdio_get(&smtpin, str, 3);
	str[3] = 0;
	for (i = 0; i < 3; i++) {
		if (str[i] == '\r')
			continue;
		if (capatext.len < HUGECAPATEXT &&
				!stralloc_append(&capatext, str + i))
			strerr_die2x(111, FATAL, "out of memory");
	}
	scan_ulong(str, &code);
	return code;
}

unsigned long
smtpcode(unsigned int *ilen)
{
	char            ch;
	unsigned long   code;
	int             err = 0;

	if (!stralloc_copys(&capatext, ""))
		strerr_die2x(111, FATAL, "out of memory");
	if ((code = get3()) < 200)
		err = 1;
	for (;;) {
		get1(&ch);
		if (ch != ' ' && ch != '-')
			err = 1;
		if (ch == ' ')
			*ilen = capatext.len - 1;
		if (ch != '-')
			break;
		while (ch != '\n')
			get1(&ch);
		if (get3() != code)
			err = 1;
	}
	while (ch != '\n')
		get1(&ch);
	return err ? 400 : code;
}

void
flush()
{
	substdio_flush(&smtpto);
}

void
flush_io()
{
	ssin.p = 0;
	substdio_flush(&smtpto);
}

GEN_ALLOC_typedef(saa, stralloc, sa, len, a)
GEN_ALLOC_readyplus(saa, stralloc, sa, len, a, 10, saa_readyplus)
saa             capakw = { 0 };	/*- list of EHLO keywords and parameters */

int             maxcapakwlen = 0;

void
extract_kw(enum starttls stls)
{
	stralloc       *saptr;
	char           *s, *e, *p;

	if (capakw.len > maxcapakwlen)
		maxcapakwlen = capakw.len;
	capakw.len = 0;
	s = capatext.s;
	if (stls == smtp)
		while (*s++ != '\n'); /*- skip the first line: contains the domain */
	e = capatext.s + capatext.len - (stls == smtp ? 6 : 0);	/*- 250-?\n */
	while (s <= e) {
		int             wasspace = 0;

		if (!saa_readyplus(&capakw, 1))
			strerr_die2x(111, FATAL, "out of memory");
		saptr = capakw.sa + capakw.len++;
		if (capakw.len > maxcapakwlen)
			*saptr = sauninit;
		else
			saptr->len = 0;

		/*- capatext is known to end in a '\n' */
		for (p = (s += (stls == smtp ? 4 : 0));; ++p) {
			if (*p == '\n' || *p == ' ' || *p == '\t') {
				if (!wasspace)
					if (!stralloc_catb(saptr, s, p - s) || !stralloc_0(saptr))
						strerr_die2x(111, FATAL, "out of memory");
				if (*p == '\n')
					break;
				wasspace = 1;
			} else
			if (wasspace == 1) {
				wasspace = 0;
				s = p;
			}
		}
		s = ++p;

		/*
		 * keyword should consist of alpha-num and '-'
		 * broken AUTH might use '=' instead of space
		 */
		if (stls == smtp) {
			for (p = saptr->s; *p; ++p) {
				if (*p == '=') {
					*p = 0;
					break;
				}
			}
		}
	}
	return;
}

unsigned long
ehlo(char *host, int len, unsigned int *ilen)
{
	unsigned long   code;

	if (substdio_puts(&smtpto, "EHLO ") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_put(&smtpto, host, len) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_puts(&smtpto, "\r\n") == -1)
		strerr_die2sys(111, FATAL, "write: ");
	else
	if (substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if ((code = smtpcode(ilen)) != 250)
		return code;
	extract_kw(smtp);
	return 250;
}

int
do_bulk()
{
	linemode = 0;
	return 0;
}

void
flush_data()
{
	linemode=1;
	flush();
	return;
}

int
smtp_ehlo(char *arg, char *cmmd, int cmmdlen)
{
	stralloc       *saptr;
	unsigned long   code;
	unsigned int    len, ilen = 0;
	static stralloc save = {0};

	if ((code = ehlo(arg, str_len(arg), &ilen)) != 250)
		strerr_die2x(111, FATAL, "Greeting to server failed");
	len = capakw.len;
	saptr = capakw.sa;
	for (; len && case_diffs(saptr->s, "STARTTLS"); ++saptr, --len);
	if (!len && ilen) {
		if (!stralloc_copyb(&save, capatext.s + ilen, capatext.len - ilen) ||
				!stralloc_0(&save))
			strerr_die2x(111, FATAL, "out of memory");
		capatext.len = ilen;
		if (!stralloc_catb(&capatext, "-STARTTLS\n250", 14) ||
				!stralloc_cat(&capatext, &save))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (substdio_put(&ssto, capatext.s, capatext.len) == -1 || substdio_flush(&ssto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return 0;
}

int
do_tls()
{
	return 1;
}

int
do_quit()
{
	if (substdio_put(&smtpto, "quit\r\n",  6) == -1 || substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	ssl_free();
	return 0;
}

int
func_unimpl(char *arg1, char *cmd, int cmdlen)
{
	if (!*cmd)
		return 0;
	if (substdio_put(&smtpto, cmd, cmdlen) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 || substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return 0;
}

int
pop3_capa(char *arg, char *cmmd, int cmmdlen)
{
	int             i, match, len;
	stralloc       *saptr;

	if (!stralloc_copys(&capatext, ""))
		strerr_die2x(111, FATAL, "out of memory");
	if (substdio_put(&smtpto, "CAPA\n", 5) == -1 || substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (getln(&smtpin, &line, &match, '\n') == -1)
		strerr_die2sys(111, FATAL, "getln: read-smtpd: ");
	if (!line.len || !match)
		strerr_die2x(100, FATAL, "CAPA command failed");
	if (!case_startb(line.s, 3, "+OK")) {
		if (!stralloc_catb(&capatext, "STLS\n", 5))
			strerr_die2x(111, FATAL, "out of memory");
		/*- capatext.len--; -*/
	} else
	for (;;) {
		if (getln(&smtpin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "getln: read-smtpd: ");
		if (!line.len || !match)
			break;
		i = str_chr(line.s, '\n');
		if (line.s[i]) {
			line.s[i] = 0;
			line.len--;
		}
		if (line.s[0] == '.')
			break;
		if (!stralloc_cat(&capatext, &line))
			strerr_die2x(111, FATAL, "out of memory");
	}
	extract_kw(pop3);
	len = capakw.len;
	saptr = capakw.sa;
	for (; len && case_diffs(saptr->s, "STLS"); ++saptr, --len) ;
	if (!len) {
		if (!stralloc_catb(&capatext, "STLS\n", 5))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (substdio_put(&ssto, "+OK Here's what I can do:\n", 26) == -1 ||
			substdio_put(&ssto, capatext.s, capatext.len) == -1 ||
			substdio_put(&ssto, ".\n", 2) == -1 ||
			substdio_flush(&ssto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return 0;
}

ssize_t
do_read(int fd, char *buf, size_t len)
{
	return (saferead(fd, buf, len, dtimeout));
}

ssize_t
do_write(int fd, char *buf, size_t len)
{
	return (safewrite(fd, buf, len, dtimeout));
}

int
do_starttls(enum starttls stls, SSL *ssl, int rfd, int wfd)
{
	fd_set          rfds;	/*- File descriptor mask for select -*/
	char            inbuf[512], outbuf[1024], smtp_inbuf[512],
	                smtp_outbuf[1024], buf[512];
	int             r, flagexitasap = 0;
	size_t          n;
	struct timeval  timeout;

	substdio_fdbuf(&ssin, do_read, 0, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssto, do_write, 1, outbuf, sizeof(outbuf));
	substdio_fdbuf(&smtpin, read, rfd, smtp_inbuf, sizeof(smtp_inbuf));
	substdio_fdbuf(&smtpto, write, wfd, smtp_outbuf, sizeof(smtp_outbuf));
	while (!flagexitasap && linemode) {
		timeout.tv_sec = dtimeout;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(rfd, &rfds);
		if ((r = select(rfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_die2sys(111, FATAL, "select: ");
		} else
		if (!r) { /*-timeout */
			timeout.tv_sec = dtimeout;
			timeout.tv_usec = 0;
			strnum[fmt_ulong(strnum, timeout.tv_sec)] = 0;
			strerr_warn4(FATAL, "idle timeout reached without input [", strnum, " sec]", 0);
			_exit(111);
		}
		if (FD_ISSET(0, &rfds))
			do_commands(stls, ssl, &ssin, rfd, wfd);
		if (FD_ISSET(rfd, &rfds)) {
			if ((n = saferead(rfd, buf, sizeof(buf), dtimeout)) < 0)
				strerr_die2sys(111, FATAL, "unable to read from smtpd: ");
			else
			if (!n) { /*- remote closed socket */
				flagexitasap = 1;
				close(wfd);
				break;
			}
			if ((n = timeoutwrite(dtimeout, 1, buf, n)) == -1)
				strerr_die2sys(111, FATAL, "unable to write to child: ");
		}
	} /*- while (!flagexitasap) */
	return 0;
}

int
main(int argc, char **argv)
{
	int             opt, pi1[2], pi2[2], client_mode = 0, tcpclient = 0;
	pid_t           pid;
	char           *certsdir, *cafile = (char *) NULL,
				   *host = (char *) NULL, *ciphers = (char *) NULL;
	SSL            *ssl;
	SSL_CTX        *ctx = (SSL_CTX *) NULL;
	enum starttls   stls = unknown;
	unsigned long   u;

	sig_ignore(sig_pipe);
	if (!(certsdir = env_get("CERTSDIR")))
		certsdir = "/etc/indimail/certs";
	if (!stralloc_copys(&certfile, certsdir))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_cats(&certfile, "/clientcert.pem"))
		strerr_die2x(111, FATAL, "out of memory");
	else
	if (!stralloc_0(&certfile))
		strerr_die2x(111, FATAL, "out of memory");
	while ((opt = getopt(argc, argv, "CTs:h:t:n:c:")) != opteof) {
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'T':
			tcpclient = 1;
			/*-flow through */
		case 'C':
			client_mode = 1;
			break;
		case 't':
			scan_ulong(optarg, &u);
			dtimeout = u;
			break;
		case 'n':
			if (!optarg)
				usage();
			if (*optarg && (!stralloc_copys(&certfile, optarg) || !stralloc_0(&certfile)))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'c':
			cafile = optarg;
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
		default:
			usage();
		}
	}
	argv += optind;
	if (pipe(pi1) != 0 || pipe(pi2) != 0)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	if (!(ciphers = env_get("TLS_CIPHER_LIST")))
		ciphers = "PROFILE=SYSTEM";
    /* setup SSL context (load key and cert into ctx) */
	if (!(ctx = tls_init(certfile.s, cafile, ciphers, client_mode ? client : server)))
		_exit(111);
	switch ((pid = fork()))
	{
		case -1:
			SSL_CTX_free(ctx);
			strerr_die2sys(111, FATAL, "fork: ");
			break;
		case 0:
			SSL_CTX_free(ctx);
			sig_uncatch(sig_pipe);
			sig_uncatch(sig_child);
			sig_uncatch(sig_term);
			close(pi1[1]);
			close(pi2[0]);
			if ((fd_move(tcpclient ? 6 : 0, pi1[0]) == -1) || (fd_move(tcpclient ? 7 : 1, pi2[1]) == -1))
				strerr_die2sys(111, FATAL, "unable to set up descriptors: ");
			pathexec(argv);
			strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
			/* Not reached */
			_exit(0);
		default:
			close(pi1[0]);
			close(pi2[1]);
			break;
	}
	if (!(ssl = tls_session(ctx, client_mode ? 6 : 0, ciphers)))
		strerr_die2x(111, FATAL, "unable to setup SSL session");
	SSL_CTX_free(ctx);
	ctx = (SSL_CTX *) NULL;
	if (client_mode && tls_connect(ssl, host) == -1)
		_exit(111);
	else { /*- server mode */
		switch (stls)
		{
		case smtp:
		case pop3:
			do_starttls(stls, ssl, pi2[0], pi1[1]);
			break;
		default:
			if (tls_accept(ssl))
				strerr_die2x(111, FATAL, "unable to accept SSL connection");
			translate(ssl, pi1[1], pi2[0], dtimeout);
			SSL_free(ssl);
			break;
		}
	}
	_exit(0);
}

#ifndef	lint
void
getversion_sslclient_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
#endif
#else
#warning "not compiled with -DTLS"
#include "substdio.h"
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(int argc, char **argv)
{
	substdio_fdbuf(&sserr, write, 2, sserrbuf, sizeof(sserrbuf));
	substdio_puts(&sserr, "not compiled with -DTLS\n");
	substdio_flush(&sserr);
	_exit(111);
}
#endif
