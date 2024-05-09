/*
 * $Id: dotls.c,v 1.24 2023-08-20 15:16:12+05:30 Cprogrammer Exp mbhangui $
 */
#ifdef TLS
#include <unistd.h>
#include <ctype.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <openssl/ssl.h>
#include <sys/stat.h>
#include <error.h>
#include <stralloc.h>
#include <scan.h>
#include <fmt.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <case.h>
#include <env.h>
#include <fd.h>
#include <signal.h>
#include <sig.h>
#include <wait.h>
#include <str.h>
#include <alloc.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <getEnvConfig.h>
#include <gen_alloc.h>
#include <gen_allocdefs.h>
#include <getln.h>
#include <noreturn.h>
#include <openreadclose.h>
#include <tls.h>
#include "upathexec.h"

#define FATAL         "dotls: fatal: "
#define HUGECAPATEXT  5000

#ifndef	lint
const char      sccsid[] = "$Id: dotls.c,v 1.24 2023-08-20 15:16:12+05:30 Cprogrammer Exp mbhangui $";
#endif

int             do_data();
int             do_retr();
int             smtp_ehlo(char *, char *, int);
int             func_unimpl(char *, char *, int);
int             do_tls();
int             do_quit();
int             pop3_capa(char *, char *, int);
void            flush_data();
void            flush_io();
void            flush();

struct scommd
{
	const char     *text;
	int             (*fun) (char *, char *, int);
	void            (*flush) (void);
};
static substdio ssin, ssto, smtpin, smtpto;
typedef const char c_char;
typedef unsigned long my_ulong;
static my_ulong dtimeout = 300; /*- timeoutdata -D */
static my_ulong ctimeout = 60;  /*- timeoutconn -t */
static stralloc certfile;
static stralloc cafile;
static stralloc crlfile;
static stralloc capatext;
static stralloc sauninit;
static stralloc line;
static stralloc saciphers;
static stralloc tls_server_version, tls_client_version;
static char     strnum1[FMT_ULONG], strnum2[FMT_ULONG];
static c_char  *remoteip, *remoteip4;
static c_char  *certdir;
static int      linemode = 1, verbosity = 1;

no_return void
usage(void)
{
	strerr_die1x(100, "usage: dotls"
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		 " [ -qQvNCTz ]\n"
#else
		 " [ -qQvCTz ]\n"
#endif
		 " [ -h host ]\n"
		 " [ -D timeoutdata ]\n"
		 " [ -t timeoutconn ]\n"
		 " [ -d certdir ]\n"
		 " [ -n cert ]\n"
		 " [ -f cipherlist ]\n"
		 " [ -M TLS methods ]\n"
		 " [ -c cafile ] \n"
		 " [ -L crlfile ] \n"
		 " [ -s starttlsType (smtp|pop3) ]\n"
		 " program");
}

struct scommd   smtpcommands[] = {
	{ "data", do_data, flush_data },
	{ "ehlo", smtp_ehlo, flush },
	{ "starttls", do_tls, flush_io },
	{ "quit", do_quit, flush},
	{ 0, func_unimpl, flush }
};

struct scommd   pop3commands[] = {
	{ "retr", do_retr, flush_data },
	{ "capa", pop3_capa, flush },
	{ "stls", do_tls, flush_io },
	{ "quit", do_quit, flush},
	{ 0, func_unimpl, flush }
};

void
write_provider_data(SSL *ssl, int writefd, int readfd)
{
	int             i;
	ssize_t         n;

	if (!stralloc_copyb(&line, "TLS Server=", 11) ||
			!stralloc_cat(&line, &tls_server_version) ||
			!stralloc_catb(&line, ", Client=", 9) ||
			!stralloc_cat(&line, &tls_client_version) ||
			!stralloc_catb(&line, ", Ciphers=", 10) ||
			!stralloc_cats(&line, SSL_CIPHER_get_name(SSL_get_current_cipher(ssl))))
		strerr_die2x(111, FATAL, "out of memory");
	i = line.len;
	if (write(writefd, (char *) &i, sizeof(int)) == -1)
		strerr_die2sys(111, FATAL, "unable to write provider data length: ");
	if (write(writefd, line.s, line.len) == -1)
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

/*-
 * Returns
 * 0 for EOF
 * 1 for success
 * exits on system errors
 */
int
do_commands(enum starttls stls, SSL *ssl, substdio *ss, int clearin, int clearout)
{
	int             i, j, found;
	char           *arg, *ptr;
	char            ch;
	struct scommd  *c;
	static stralloc cmd = { 0 };

	if (!stralloc_copys(&cmd, ""))
		strerr_die2x(111, FATAL, "out of memory");
	switch (stls)
	{
	case smtp:
		c = smtpcommands;
		break;
	case pop3:
		c = pop3commands;
		break;
	default:
		strerr_die2x(111, FATAL, "invalid STARTTLS handler. Current known methods are SMTP, POP3");
	}
	for (;;) {
		if ((i = substdio_get(ss, &ch, 1)) <= 0) {
			if (i < 0)
				strerr_die2(111, FATAL, "read: ", &strerr_tls);
			/*- premature close or read error */
			return i;
		}
		if (ch == '\n')
			break;
		if (!ch)
			ch = '\n';
		if (!stralloc_append(&cmd, &ch))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (cmd.len > 0 && cmd.s[cmd.len - 1] == '\r')
		--cmd.len;
	if (!stralloc_0(&cmd))
		strerr_die2x(111, FATAL, "out of memory");
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
	if (i) { /*- only do_tls() returns 1 implying STARTTLS or STLS command */
		switch (stls)
		{
		case smtp:
			if (substdio_put(&ssto, "220 ready for tls\r\n", 19) == -1 ||
					substdio_flush(&ssto) == -1)
				strerr_die2(111, FATAL, "write: ", &strerr_tls);
			break;
		case pop3:
			if (substdio_put(&ssto, "+OK Begin SSL/TLS negotiation now.\r\n", 37) == -1 ||
					substdio_flush(&ssto) == -1)
				strerr_die2(111, FATAL, "write: ", &strerr_tls);
			break;
		default:
			if ((ptr = env_get("BANNER"))) {
				if (substdio_puts(&ssto, ptr) == -1 ||
						substdio_put(&ssto, "\n", 1) == -1 ||
					substdio_flush(&ssto) == -1)
				strerr_die2(111, FATAL, "write: ", &strerr_tls);
			}
			break;
		}
		/*- starttls session */
		if (tls_accept(dtimeout, 0, 1, ssl))
			strerr_die2x(111, FATAL, "unable to accept SSL connection");
		if (verbosity >= 2) {
			if (!stralloc_copys(&tls_client_version, SSL_get_version(ssl)) ||
					!stralloc_0(&tls_client_version))
				strerr_die2x(111, FATAL, "out of memory");
			tls_client_version.len--;
			SSL_get_cipher_bits(ssl, &i);
			strnum1[fmt_ulong(strnum1, getpid())] = 0;
			strnum2[fmt_ulong(strnum2, i)] = 0;
			strerr_warn12("dotls: pid ", strnum1, " from ", remoteip, " TLS Server=",
					tls_server_version.s, ", Client=", tls_client_version.s,
					", Ciphers=", SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)), ", bits=", strnum2, 0);
		}
		i = translate(0, 1, clearout, clearin, dtimeout); /*- returns only when one side closes */
		if (verbosity >= 2) {
			strnum2[fmt_int(strnum2, i)] = 0;
			if (i)
				strerr_warn7("dotls: pid ", strnum1, " from ", remoteip, ", status=", strnum2, " exiting...", 0);
			else
				strerr_warn5("dotls: pid ", strnum1, " from ", remoteip, " exiting...", 0);
		}
		ssl_free();
		_exit(i);
	}
	return 1;
}

void
get1(char *ch)
{
	substdio_get(&smtpin, ch, 1);
	if (capatext.len >= HUGECAPATEXT)
		strerr_die2x(111, FATAL, "line too long");
	else
	if (!stralloc_append(&capatext, ch))
		strerr_die2x(111, FATAL, "out of memory");
	if (*ch == '\r')
		return;
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
		if (capatext.len >= HUGECAPATEXT)
			strerr_die2x(111, FATAL, "line too long");
		else
		if (!stralloc_append(&capatext, str + i))
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
		/*-
		 * ilen is length of capabilities minus the lengh of last
		 * capability and one space character
		 * capatext.len is the length of the string with all
		 * capabilities
		 */
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

	if (substdio_put(&smtpto, "EHLO ", 5) == -1 ||
			substdio_put(&smtpto, host, len) == -1 ||
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if ((code = smtpcode(ilen)) != 250)
		return code;
	extract_kw(smtp);
	return 250;
}

int
do_data()
{
	linemode = 0;
	if (substdio_put(&smtpto, "DATA\r\n", 6) == -1 ||
			substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return 0;
}

int
do_retr(char *arg, char *cmmd, int cmmdlen)
{
	linemode = 0;
	if (substdio_put(&smtpto, "RETR ", 5) == -1 ||
			substdio_puts(&smtpto, arg) ||
			substdio_put(&smtpto, "\r\n", 2) ||
			substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return 0;
}

void
flush_data()
{
	flush();
	return;
}

int
smtp_ehlo(char *arg, char *cmmd, int cmmdlen)
{
	stralloc       *saptr;
	unsigned long   code;
	unsigned int    len, ilen = 0;
	static stralloc save = { 0 };

	if ((code = ehlo(arg, str_len(arg), &ilen)) != 250)
		strerr_die2x(111, FATAL, "Greeting to server failed");
	len = capakw.len;
	saptr = capakw.sa;
	for (;len && case_diffs(saptr->s, "STARTTLS");++saptr, --len);
	if (!len && ilen) { /*- server/child doesn't have STARTTLS capability */
		/*-
		 * e.g. ehlo of server without STARTTLS
		 * 250-indimail.org [::ffff:127.0.0.1]\r\n
		 * 250-PIPELINING\r\n
		 * 250-8BITMIME\r\n
		 * 250-SIZE 10000000\r\n
		 * 250-ETRN\r\n
		 * 250 HELP\r\n
		 *
		 * store the last capability in save. e.g.
		 * <sp>HELP\r\n
		 * where <sp> is the single space char
		 */
		if (!stralloc_copyb(&save, capatext.s + ilen, capatext.len - ilen))
			strerr_die2x(111, FATAL, "out of memory");
		/*-
		 * truncate length of capatext so that it becoems
		 * 250-indimail.org [::ffff:127.0.0.1]\r\n
		 * 250-PIPELINING\r\n
		 * 250-8BITMIME\r\n
		 * 250-SIZE 10000000\r\n
		 * 250-ETRN\r\n
		 * 250
		 */
		capatext.len = ilen;
		/*
		 * append STARTTLS to the capability, followed by <sp>HELP\r\n.
		 * The capability passed to the client thus becomes
		 * 250-indimail.org [::ffff:127.0.0.1]\r\n
		 * 250-PIPELINING\r\n
		 * 250-8BITMIME\r\n
		 * 250-SIZE 10000000\r\n
		 * 250-ETRN\r\n
		 * 250-STARTTLS\r\n
		 * 250 HELP\r\n
		 */
		if (!stralloc_catb(&capatext, "-STARTTLS\r\n250", 14) ||
				!stralloc_cat(&capatext, &save))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (substdio_put(&ssto, capatext.s, capatext.len) == -1 ||
			substdio_flush(&ssto) == -1)
		strerr_die2(111, FATAL, "write: ", &strerr_tls);
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
	if (substdio_put(&smtpto, "quit\r\n",  6) == -1 ||
			substdio_flush(&smtpto) == -1)
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
			substdio_put(&smtpto, "\r\n", 2) == -1 ||
			substdio_flush(&smtpto) == -1)
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
	if (substdio_put(&smtpto, "CAPA\n", 5) == -1 ||
			substdio_flush(&smtpto) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	if (getln(&smtpin, &line, &match, '\n') == -1)
		strerr_die2sys(111, FATAL, "getln: read-smtpd: ");
	if (!line.len || !match)
		strerr_die2x(100, FATAL, "CAPA command failed");
	if (!case_startb(line.s, 3, "+OK")) {
		if (!stralloc_catb(&capatext, "STLS\n", 5))
			strerr_die2x(111, FATAL, "out of memory");
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
	for (;len && case_diffs(saptr->s, "STLS");++saptr, --len) ;
	if (!len) {
		if (!stralloc_catb(&capatext, "STLS\n", 5))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (substdio_put(&ssto, "+OK Here's what I can do:\n", 26) == -1 ||
			substdio_put(&ssto, capatext.s, capatext.len) == -1 ||
			substdio_put(&ssto, ".\n", 2) == -1 ||
			substdio_flush(&ssto) == -1)
		strerr_die2(111, FATAL, "write: ", &strerr_tls);
	return 0;
}

ssize_t
do_read(int fd, char *buf, size_t len)
{
	return (tlsread(fd, buf, len, dtimeout));
}

ssize_t
do_write(int fd, char *buf, size_t len)
{
	return (tlswrite(fd, buf, len, dtimeout));
}

int
do_starttls(enum starttls stls, SSL *ssl, int clearin, int clearout)
{
	fd_set          rfds; /*- File descriptor mask for select -*/
	char            inbuf[512], outbuf[1024], smtp_inbuf[512],
	                smtp_outbuf[1024], buf[512];
	int             ret, r, flagexitasap = 0, fd0_flag = 1;
	size_t          n;
	struct timeval  timeout;

	substdio_fdbuf(&ssin, do_read, 0, inbuf, sizeof(inbuf));                    /*- from client */
	substdio_fdbuf(&ssto, do_write, 1, outbuf, sizeof(outbuf));                 /*- to   client */
	substdio_fdbuf(&smtpin, read, clearin, smtp_inbuf, sizeof(smtp_inbuf));     /*- cleartxt in  from (smtpd, pop3d, etc) */
	substdio_fdbuf(&smtpto, write, clearout, smtp_outbuf, sizeof(smtp_outbuf)); /*- cleartxt out from (smtpd, pop3d, etc) */
	while (!flagexitasap) {
		timeout.tv_sec = dtimeout;
		timeout.tv_usec = 0;
		FD_ZERO(&rfds);
		if (fd0_flag)
			FD_SET(0, &rfds);
		FD_SET(clearin, &rfds);
		if ((r = select(clearin + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout)) < 0) {
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
			strnum1[fmt_ulong(strnum1, timeout.tv_sec)] = 0;
			if (verbosity >= 2)
				strerr_warn4(FATAL, "idle timeout reached without input [", strnum1, " sec]", 0);
			_exit(111);
		}
		if (fd0_flag && FD_ISSET(0, &rfds)) { /*- data from client */
			if (!linemode) { /*- data/retr command was issued without initiating TLS */
				if ((n = tlsread(0, buf, sizeof(buf), dtimeout)) < 0)
					strerr_die3sys(111, FATAL, "unable to read from ", stls == smtp ? "smtp client: " : "pop3 client: ");
				else
				if (!n) { /*- client closed connection */
					FD_CLR(0, &rfds);
					fd0_flag = 0;
					close(clearout); /*- make the child get EOF on read */
				} else {
					if ((n = timeoutwrite(dtimeout, clearout, buf, n)) == -1)
						strerr_die3sys(111, FATAL, "unable to write to ", stls == smtp ? "smtpd: " : "pop3d: ");
					if ((n == 3 && buf[0] == '.' && buf[1] == '\r' && buf[2] == '\n') ||
							(buf[n - 1] == '\n' && buf[n - 2] == '\r' && buf[n - 3] == '.'))
						linemode = 1;
				}
			} else {
				/*-
				 * do_commands returns only if TLS session wasn't initiated
				 * In such a case we continue to pass data from the client
				 * to child and data from child to client in clear text
				 *
				 * If TLS session is initiated, do_commands executes translate()
				 * which has it's nown select() loop to encrypt/decrypt data and
				 * pass data between client and child with the necessary conversion
				 * when translate() function returns due to exit either by client or
				 * the child, do_command calls _exit().
				 */
				if ((ret = do_commands(stls, ssl, &ssin, clearin, clearout)) <= 0) {
					/*- client closed connection / error */
					FD_CLR(0, &rfds);
					fd0_flag = 0;
					close(clearout); /*- make the child get EOF on read */
				}
			}
		}
		if (FD_ISSET(clearin, &rfds)) { /*- data from program (smtpd, pop3d) */
			if ((n = tlsread(clearin, buf, sizeof(buf), dtimeout)) < 0)
				strerr_die3sys(111, FATAL, "unable to read from ", stls == smtp ? "smtpd: " : "pop3d: ");
			else
			if (!n) { /*- child (smtp/pop3 binary) exited */
				flagexitasap = 1;
				close(1); /*- tell client no more data can be read */
			} else
			if ((n = timeoutwrite(dtimeout, 1, buf, n)) == -1)
				strerr_die3sys(111, FATAL, "unable to write to ", stls == smtp ? "smtp client: " : "pop3 client: ");
		}
	} /*- while (!flagexitasap) */
	return 0;
}

void
sigchld()
{
	int             i, wstat, pid;
	char            tmp1[FMT_ULONG], tmp2[FMT_ULONG];

	while ((pid = wait_nohang(&wstat)) > 0) {
		if (verbosity < 2)
			continue;
		tmp1[fmt_ulong(tmp1, pid)] = 0;
		if (wait_stopped(wstat) || wait_continued(wstat)) {
			i = wait_stopped(wstat) ? wait_stopsig(wstat) : SIGCONT;
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("dotls: end ", tmp1, wait_stopped(wstat) ? " stopped by signal " : " continued by signal ", tmp2, 0);
		} else
		if (wait_signaled(wstat)) {
			i = wait_termsig(wstat);
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("dotls: end ", tmp1, " killed by signal ", tmp2, 0);
		} else
		if ((i = wait_exitcode(wstat))) {
			tmp2[fmt_ulong(tmp2, i)] = 0;
			strerr_warn4("dotls: end ", tmp1, " status ", tmp2, 0);
		}
		tmp1[fmt_ulong(tmp1, pid)] = 0;
		strerr_warn5("dotls: pid ", tmp1, " from ", remoteip, " child exiting...", 0);
	}
}

int
main(int argc, char **argv)
{
	int             i, opt, pi1[2], pi2[2], client_mode = 0,
					tcpclient = 0, provide_data = 0;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	int             client_renegotiation = 0;
#endif
	pid_t           pid;
	const char     *host = NULL, *ciphers = NULL, *cipherfile = NULL, *tls_method = NULL;
	char           *ptr;
	SSL            *ssl;
	SSL_CTX        *ctx = NULL;
	enum starttls   stls = unknown;
	unsigned long   u;
	struct stat     st;
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
	EVP_PKEY       *dh_pkey;
	int             bits;
#endif

	sig_ignore(sig_pipe);
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	while ((opt = getopt(argc, argv, "qQvCTNd:s:h:t:n:c:L:f:M:D:z")) != opteof) {
#else
	while ((opt = getopt(argc, argv, "qQvCTd:s:h:t:n:c:L:f:M:D:z")) != opteof) {
#endif
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'C': /*- compatibility with original tcpclient */
			tcpclient = 1;
			/*- flow through */
		case 'T':
			client_mode = 1;
			break;
		case 'D':
			scan_ulong(optarg, &u);
			dtimeout = u;
			break;
		case 't':
			scan_ulong(optarg, &u);
			ctimeout = u;
			break;
		case 'd':
			certdir = optarg;
			break;
		case 'n':
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
		case 'f':
			cipherfile = optarg;
			break;
		case 'M':
			tls_method = optarg;
			break;
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
		case 'N':
			client_renegotiation = 1;
			break;
#endif
		case 'z':
			provide_data = 1;
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
		case 'v':
			verbosity = 2;
			break;
		case 'Q':
			verbosity = 1;
			break;
		case 'q':
			verbosity = 0;
			break;
		default:
			usage();
		}
	}
	argv += optind;
	if (!*argv)
		usage();
	if (!verbosity)
		subfderr->fd = -1;
	if (!client_mode && env_get("NOTLS")) {
		upathexec(argv);
		strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	}
	if (!(remoteip4 = env_get("TCPREMOTEIP")))
		remoteip4 = "unknown";
	if (!(remoteip = env_get("TCP6REMOTEIP")) && !(remoteip = remoteip4))
		remoteip = "unknown";
	if (!certdir && !(certdir = env_get("CERTDIR")))
		certdir = "/etc/indimail/certs";
	set_certdir(certdir);
	if (!certfile.len) {
		if (!(ptr = env_get("TLS_CERTFILE")))
			ptr = client_mode ? (char *) "clientcert.pem" : (char *) "servercert.pem";
		if (*ptr != '.' && *ptr != '/') {
			if (!stralloc_copys(&certfile, certdir) ||
					!stralloc_append(&certfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&certfile, ptr) ||
				!stralloc_0(&certfile))
			strerr_die2x(111, FATAL, "out of memory");
	}
	if (!cafile.len) {
		if (!(ptr = env_get("CLIENTCA")))
			ptr = (char *) "clientca.pem";
		if (*ptr != '.' && *ptr != '/') {
			if (!stralloc_copys(&cafile, certdir) ||
					!stralloc_append(&cafile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&cafile, ptr) ||
				!stralloc_0(&cafile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(cafile.s, R_OK))
			cafile.len = 0;
	}
	if (!crlfile.len) {
		if (!(ptr = env_get("CLIENTCRL")))
			ptr = (char *) "clientca.pem";
		if (*ptr != '.' && *ptr != '/') {
			if (!stralloc_copys(&crlfile, certdir) ||
					!stralloc_append(&crlfile, "/"))
				strerr_die2x(111, FATAL, "out of memory");
		}
		if (!stralloc_cats(&crlfile, ptr) ||
				!stralloc_0(&crlfile))
			strerr_die2x(111, FATAL, "out of memory");
		if (access(crlfile.s, R_OK))
			crlfile.len = 0;
	}
	/*-
	 * We create two pipes to read and write unencrypted data
	 * from/to child
	 * pi1[0] - child  uses this to read from parent
	 * pi2[1] - child  uses this to write to  parent
	 * pi1[1] - parent uses this to write to  child
	 * pi2[0] - parent uses this to read from child
	 */
	if (pipe(pi1) != 0 || pipe(pi2) != 0)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
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
		for (ptr = saciphers.s; *ptr; ptr++)
			if (isspace(*ptr)) {
				*ptr = 0;
				break;
			}
		ciphers = saciphers.s;
	} else
	if (!ciphers) {
		i = get_tls_method(tls_method);
		ciphers = env_get(i < 7 ? "TLS_CIPHER_LIST" : "TLS_CIPHER_SUITE");
	}
    /*- setup SSL context (load key and cert into ctx) */
	if (!(ctx = tls_init(tls_method, certfile.s,
			cafile.len ? cafile.s : NULL, crlfile.len ? crlfile.s : NULL,
			ciphers, client_mode ? client : server)))
		strerr_die2x(111, FATAL, "unable to initialize TLS");
#ifdef SSL_OP_ALLOW_CLIENT_RENEGOTIATION
	if (client_renegotiation && !client_mode)
		SSL_CTX_set_options(ctx, SSL_OP_ALLOW_CLIENT_RENEGOTIATION);
#endif
	sig_catch(sig_child, sigchld);
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
		if (stls != smtp && stls != pop3) {
			if (!stralloc_copyb(&line, "dotls, ", 7))
				strerr_die2x(111, FATAL, "out of memory");
			if (provide_data)
				read_provider_data(&line, pi2[0], pi1[1]);
			if (!stralloc_0(&line))
				strerr_die2x(111, FATAL, "out of memory");
			if (!env_put2("TLS_PROVIDER", line.s))
				strerr_die2x(111, FATAL, "out of memory");
		} else
		if (!env_put("TLS_PROVIDER=dotls, starttls"))
			strerr_die2x(111, FATAL, "out of memory");
		close(pi1[1]); /*- pi1[0] will be used for reading cleartxt */
		close(pi2[0]); /*- pi2[1] will be used for writing cleartxt */
		if ((fd_move(tcpclient ? 6 : 0, pi1[0]) == -1) || (fd_move(tcpclient ? 7 : 1, pi2[1]) == -1))
			strerr_die2sys(111, FATAL, "unable to set up descriptors: ");
		upathexec(argv);
		strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
		/*- Not reached */
		_exit(0);
	default:
		break;
	}
	if (!(ssl = tls_session(ctx, client_mode ? 6 : 0))) {
		SSL_CTX_free(ctx);
		strerr_die2x(111, FATAL, "unable to setup SSL session");
	}
	SSL_CTX_free(ctx);
	ctx = NULL;
	if (!stralloc_copys(&tls_server_version, SSL_get_version(ssl)) ||
				!stralloc_0(&tls_server_version))
		strerr_die2x(111, FATAL, "out of memory");
	tls_server_version.len--;
	if (client_mode) {
		if (tls_connect(ctimeout, 6, 7, ssl, host) == -1)
			_exit(111);
		if (!stralloc_copys(&tls_client_version, SSL_get_version(ssl)) ||
					!stralloc_0(&tls_client_version))
				strerr_die2x(111, FATAL, "out of memory");
		tls_client_version.len--;
		if (verbosity >= 2) {
			strnum1[fmt_ulong(strnum1, getpid())] = 0;
			SSL_get_cipher_bits(ssl, &i);
			strnum2[fmt_ulong(strnum2, i)] = 0;
			strerr_warn12("dotls: pid ", strnum1, " from ", remoteip, " TLS Server=",
					tls_server_version.s, ", Client=", tls_client_version.s,
					", Ciphers=", SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)),
					", bits=", strnum2, 0);
		}
		if (provide_data)
			write_provider_data(ssl, pi2[1], pi1[0]);
		close(pi1[0]); /*- pi1[1] be used for writing cleartext */
		close(pi2[1]); /*- pi2[0] be used for reading cleartext */
		i = translate(6, 7, pi1[1], pi2[0], dtimeout);
		if (verbosity >= 2) {
			strnum1[fmt_ulong(strnum1, getpid())] = 0;
			strnum2[fmt_int(strnum2, i)] = 0;
			if (i)
				strerr_warn7("dotls: pid ", strnum1, " from ", remoteip, ", status=", strnum2, " exiting...", 0);
			else
				strerr_warn5("dotls: pid ", strnum1, " from ", remoteip, " exiting...", 0);
		}
		ssl_free();
		_exit(i);
	} else { /*- server mode */
		switch (stls)
		{
		case smtp:
		case pop3:
			close(pi1[0]); /*- pi1[1] be used for writing cleartext */
			close(pi2[1]); /*- pi2[0] be used for reading cleartext */
			do_starttls(stls, ssl, pi2[0], pi1[1]);
			break;
		default:
			if ((ptr = env_get("BANNER"))) {
				char            outbuf[1024];
				substdio_fdbuf(&ssto, do_write, 1, outbuf, sizeof(outbuf)); /*- to child (smtpd, pop3d */
				if (substdio_puts(&ssto, ptr) == -1 ||
						substdio_put(&ssto, "\n", 1) == -1 ||
					substdio_flush(&ssto) == -1)
				strerr_die2(111, FATAL, "write: ", &strerr_tls);
			}
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
			getEnvConfigInt(&bits, "SSL_BITS", 2048);
			if (!get_rsakey(0, bits, certdir))
				EVP_RSA_gen(bits);
			if ((dh_pkey = get_dhkey(0, bits, certdir))) {
				SSL_set_dh_auto(ssl, 0);
				SSL_set0_tmp_dh_pkey(ssl, dh_pkey);
			} else
				SSL_set_dh_auto(ssl, 1);
#else
			SSL_set_tmp_rsa_callback(ssl, tmp_rsa_cb);
			SSL_set_tmp_dh_callback(ssl, tmp_dh_cb);
#endif
			/*- non-starttls session */
			if (tls_accept(ctimeout, 0, 1, ssl))
				strerr_die2x(111, FATAL, "unable to accept SSL connection");
			if (!stralloc_copys(&tls_client_version, SSL_get_version(ssl)) ||
						!stralloc_0(&tls_client_version))
				strerr_die2x(111, FATAL, "out of memory");
			tls_client_version.len--;
			if (verbosity >= 2) {
				SSL_get_cipher_bits(ssl, &i);
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_ulong(strnum2, i)] = 0;
				strerr_warn12("dotls: pid ", strnum1, " from ", remoteip, " TLS Server=",
						tls_server_version.s, ", Client=", tls_client_version.s,
						", Ciphers=", SSL_CIPHER_get_name(SSL_get_current_cipher(ssl)),
						", bits=", strnum2, 0);
			}
			if (provide_data)
				write_provider_data(ssl, pi2[1], pi1[0]);
			close(pi1[0]); /*- pi1[1] be used for writing cleartext */
			close(pi2[1]); /*- pi2[0] be used for reading cleartext */
			i = translate(0, 1, pi1[1], pi2[0], dtimeout);
			if (verbosity >= 2) {
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				strnum2[fmt_int(strnum2, i)] = 0;
				if (i)
					strerr_warn7("dotls: pid ", strnum1, " from ", remoteip, ", status=", strnum2, " exiting...", 0);
				else
					strerr_warn5("dotls: pid ", strnum1, " from ", remoteip, " exiting...", 0);
			}
			ssl_free();
			_exit(i);
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

/*
 * $Log: dotls.c,v $
 * Revision 1.24  2023-08-20 15:16:12+05:30  Cprogrammer
 * use TLS_CIPHER_LIST, TLS_CIPHER_SUITE to set ciphers
 *
 * Revision 1.23  2023-08-08 00:23:03+05:30  Cprogrammer
 * use strerr_tls for tls errors
 *
 * Revision 1.22  2023-02-13 20:15:42+05:30  Cprogrammer
 * added error message for tls_init failure
 *
 * Revision 1.21  2023-01-11 08:19:33+05:30  Cprogrammer
 * added -N option to allow client side renegotiation
 *
 * Revision 1.20  2023-01-08 19:17:15+05:30  Cprogrammer
 * added -q, -v, -Q option to control verbosity
 *
 * Revision 1.19  2023-01-08 12:11:33+05:30  Cprogrammer
 * added -D, -t option for timeoutdata, timeoutconn
 * added -z option to turn on setting of TLS_PROVIDER env variable
 * log ssl cipher bits
 * replace internal TLS function with TLS functions from libqmail
 *
 * Revision 1.18  2022-12-27 08:57:23+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.17  2022-12-27 07:40:21+05:30  Cprogrammer
 * added -L option to specify CRL
 * added sigchild handler
 * set TLS_PROVIDER env variable
 *
 * Revision 1.16  2022-12-25 21:52:58+05:30  Cprogrammer
 * include error.h for errno
 *
 * Revision 1.15  2022-12-25 19:26:44+05:30  Cprogrammer
 * added BANNER functionality for clients to initiate STARTTLS
 *
 * Revision 1.14  2022-12-24 22:14:01+05:30  Cprogrammer
 * added -d option to specify certdir
 * set RSA/DH parameters
 *
 * Revision 1.13  2022-12-23 18:00:17+05:30  Cprogrammer
 * fixed non-tls incorrect linemode setting
 *
 * Revision 1.12  2022-12-23 10:35:06+05:30  Cprogrammer
 * added -M option to set TLS / SSL client/server method
 *
 * Revision 1.11  2022-12-23 00:21:21+05:30  Cprogrammer
 * bypass SSL/TLS if NOTLS is set
 *
 * Revision 1.10  2022-12-22 22:18:56+05:30  Cprogrammer
 * added -f option to load tls ciphers from a file
 * log client, server tls version on connect
 *
 * Revision 1.9  2022-07-01 09:15:37+05:30  Cprogrammer
 * use TLS_CERTFILE env variable to set client certificate filename
 *
 * Revision 1.8  2022-06-05 07:48:17+05:30  Cprogrammer
 * BUG \r not copied, extra \0 copied. Thanks Stefan Berger
 * Report line too long error instead of clubbing it with 'out of mem' error
 * Return error for pop3 substdio failure
 * fix DATA/RETR commands not getting passed to child
 * handle eof from network gracefully
 *
 * Revision 1.7  2021-08-30 12:47:59+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.6  2021-05-12 21:01:23+05:30  Cprogrammer
 * replace pathexec() with upathexec()
 *
 * Revision 1.5  2021-03-09 08:15:56+05:30  Cprogrammer
 * removed unnecessary initializations and type casts
 *
 * Revision 1.4  2021-03-09 00:55:32+05:30  Cprogrammer
 * SSL argument to translate replaced with fd
 *
 * Revision 1.3  2021-03-07 21:37:12+05:30  Cprogrammer
 * fixed usage
 *
 * Revision 1.2  2021-03-07 18:11:04+05:30  Cprogrammer
 * added opportunistic TLS for pop3
 *
 * Revision 1.1  2021-03-07 08:13:57+05:30  Cprogrammer
 * Initial revision
 *
 */
