/*
 * $Log: mini-smtpd.c,v $
 * Revision 1.6  2023-09-24 00:23:57+05:30  Cprogrammer
 * minor code style changes
 *
 * Revision 1.5  2022-10-22 13:07:14+05:30  Cprogrammer
 * added program identifier to Received header
 *
 * Revision 1.4  2021-10-22 15:51:41+05:30  Cprogrammer
 * removed extra arguments to err_size()
 *
 * Revision 1.3  2021-09-11 18:58:03+05:30  Cprogrammer
 * pass null remotehost to received when remotehost is unknown
 *
 * Revision 1.2  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.1  2021-07-09 21:50:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sig.h>
#include <stralloc.h>
#include <substdio.h>
#include <alloc.h>
#include <constmap.h>
#include <error.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <commands.h>
#include <str.h>
#include <fmt.h>
#include <scan.h>
#include <byte.h>
#include <case.h>
#include <env.h>
#include <now.h>
#include <noreturn.h>
#include "auto_qmail.h"
#include "control.h"
#include "received.h"
#include "ipme.h"
#include "ip.h"
#include "qmail.h"
#include "rcpthosts.h"

#define MAXHOPS 100
static int      databytes = 0;
static int      timeout = 1200;
static int      liphostok = 0;
static int      seenmail = 0;
static int      flagsize = 0;
static stralloc greeting = { 0 };
static stralloc helohost = { 0 };
static stralloc liphost = { 0 };
static stralloc addr = { 0 };	/* will be 0-terminated, if addrparse returns 1 */
static stralloc mailfrom = { 0 };
static stralloc rcptto = { 0 };
static stralloc mfparms = { 0 };
static char    *remoteip;
static char    *remotehost;
static char    *remoteinfo;
static char    *local;
static char    *relayclient;
static char    *fakehelo;		/* pointer into helohost, or 0 */
static char     ssinbuf[1024];
static char     ssoutbuf[512];
static ssize_t  safewrite(int, char *, size_t);
static ssize_t  saferead(int, char *, size_t);
static substdio ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
static struct qmail    qqt;
static unsigned int    bytestooverflow = 0;

ssize_t
safewrite(int fd, char *buf, size_t len)
{
	ssize_t         r;
	if ((r = timeoutwrite(timeout, fd, buf, len)) <= 0)
		_exit(1);
	return r;
}

void
flush()
{
	substdio_flush(&ssout);
}

void
out(s)
	char           *s;
{
	substdio_puts(&ssout, s);
}

no_return void
die_read()
{
	_exit(1);
}

no_return void
die_alarm()
{
	out("451 timeout (#4.4.2)\r\n");
	flush();
	_exit(1);
}

no_return void
die_nomem()
{
	out("421 out of memory (#4.3.0)\r\n");
	flush();
	_exit(1);
}

no_return void
die_control()
{
	out("421 unable to read controls (#4.3.0)\r\n");
	flush();
	_exit(1);
}

no_return void
die_ipme()
{
	out("421 unable to figure out my IP addresses (#4.3.0)\r\n");
	flush();
	_exit(1);
}

no_return void
straynewline()
{
	out("451 See http://pobox.com/~djb/docs/smtplf.html.\r\n");
	flush();
	_exit(1);
}

void
err_nogateway()
{
	out("553 sorry, that domain isn't in my list of allowed rcpthosts (#5.7.1)\r\n");
}

void
err_size()
{
	out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n");
}

void
err_unimpl(arg)
	char           *arg;
{
	out("502 unimplemented (#5.5.1)\r\n");
}

void
err_syntax()
{
	out("555 syntax error (#5.5.4)\r\n");
}

void
err_wantmail()
{
	out("503 MAIL first (#5.5.1)\r\n");
}

void
err_wantrcpt()
{
	out("503 RCPT first (#5.5.1)\r\n");
}

void
err_noop(arg)
	char           *arg;
{
	out("250 ok\r\n");
}

void
err_vrfy(arg)
	char           *arg;
{
	out("252 send some mail, i'll try my best\r\n");
}

void
err_qqt()
{
	out("451 qqt failure (#4.3.0)\r\n");
}

void
smtp_greet(char *code)
{
	substdio_puts(&ssout, code);
	substdio_put(&ssout, greeting.s, greeting.len);
}

void
smtp_help(char *arg)
{
	out("214 netqmail home page: http://qmail.org/netqmail\r\n");
}

no_return void
smtp_quit(char *arg)
{
	smtp_greet("221 ");
	out("\r\n");
	flush();
	_exit(0);
}

void
dohelo(char *arg)
{
	if (!stralloc_copys(&helohost, arg) ||
			!stralloc_0(&helohost))
		die_nomem();
	fakehelo = case_diffs(remotehost, helohost.s) ? helohost.s : 0;
}

void
setup()
{
	char           *x;
	unsigned long   u;

	if (control_init() == -1 ||
			control_rldef(&greeting, "smtpgreeting", 1, (char *) 0) != 1 ||
			(liphostok = control_rldef(&liphost, "localiphost", 1, (char *) 0)) == -1 ||
			control_readint(&timeout, "timeoutsmtpd") == -1 ||
			rcpthosts_init() == -1 ||
			control_readint(&databytes, "databytes") == -1)
		die_control();
	if (timeout <= 0)
		timeout = 1;
	if ((x = env_get("DATABYTES"))) {
		scan_ulong(x, &u);
		databytes = u;
	}
	if (!(databytes + 1))
		--databytes;

	if (!(remoteip = env_get("TCPREMOTEIP")))
		remoteip = "unknown";
	if (!(local = env_get("TCPLOCALHOST")))
		local = env_get("TCPLOCALIP");
	if (!local)
		local = "unknown";
	if (!(remotehost = env_get("TCPREMOTEHOST")))
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	relayclient = env_get("RELAYCLIENT");
	dohelo(remotehost);
}

int
addrparse(char *arg)
{
	int             i;
	char            ch;
	char            terminator;
	struct ip_address ip;
	int             flagesc;
	int             flagquoted;

	terminator = '>';
	i = str_chr(arg, '<');
	if (arg[i])
		arg += i + 1;
	else {	/* partner should go read rfc 821 */
		terminator = ' ';
		arg += str_chr(arg, ':');
		if (*arg == ':')
			++arg;
		while (*arg == ' ')
			++arg;
	}

	/*- strip source route */
	if (*arg == '@')
		while (*arg)
			if (*arg++ == ':')
				break;

	if (!stralloc_copys(&addr, ""))
		die_nomem();
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) {	/* copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				die_nomem();
			flagesc = 0;
		} else {
			if (!flagquoted && (ch == terminator))
				break;
			switch (ch)
			{
			case '\\':
				flagesc = 1;
				break;
			case '"':
				flagquoted = !flagquoted;
				break;
			default:
				if (!stralloc_append(&addr, &ch))
					die_nomem();
			}
		}
	}
	/*
	 * could check for termination failure here, but why bother?
	 */
	if (!stralloc_append(&addr, ""))
		die_nomem();
	if (liphostok) {
		i = byte_rchr(addr.s, addr.len, '@');
		if (i < addr.len) /* if not, partner should go read rfc 821 */
			if (addr.s[i + 1] == '[')
				if (!addr.s[i + 1 + ip4_scanbracket(addr.s + i + 1, &ip)])
					if (ipme_is(&ip)) {
						addr.len = i + 1;
						if (!stralloc_cat(&addr, &liphost))
							die_nomem();
						if (!stralloc_0(&addr))
							die_nomem();
					}
	}

	if (addr.len > 900)
		return 0;
	return 1;
}

int
addrallowed()
{
	int             r;
	if ((r = rcpthosts(addr.s, str_len(addr.s), 0)) == -1)
		die_control();
	return r;
}

void
smtp_helo(char *arg)
{
	smtp_greet("250 ");
	out("\r\n");
	seenmail = 0;
	dohelo(arg);
}

void
smtp_ehlo(char *arg)
{
	char            size_buf[FMT_ULONG]; /*- needed for SIZE CMD */

	smtp_greet("250-");
	out("\r\n250-PIPELINING\r\n");
	if (databytes) {
		size_buf[fmt_ulong(size_buf, (unsigned long) databytes)] = 0;
		out("250-SIZE ");
		out(size_buf);
		out("\r\n");
	}
	out("250 8BITMIME\r\n");
	seenmail = 0;
	dohelo(arg);
}

void
smtp_rset(char *arg)
{
	seenmail = 0;
	out("250 flushed\r\n");
}

int
mailfrom_size(char *arg)
{
	long            r;
	unsigned long   sizebytes = 0;

	scan_ulong(arg, (unsigned long *) &r);
	sizebytes = r;
	if (databytes && (sizebytes > databytes))
		return 1;
	return 0;
}

void
mailfrom_parms(char *arg)
{
	int             i;
	int             len;

	len = str_len(arg);
	mfparms.len = 0;
	i = byte_chr(arg, len, '>');
	if (i > 4 && i < len) {
		while (len) {
			arg++;
			len--;
			if (*arg == ' ' || *arg == '\0') {
				if (case_starts(mfparms.s, "SIZE=")) {
					mfparms.s[mfparms.len] = 0;
					if (mailfrom_size(mfparms.s + 5)) {
						flagsize = 1;
						return;
					}
				}
				mfparms.len = 0;
			} else
			if (!stralloc_catb(&mfparms, arg, 1))
				die_nomem();
		}
	}
}

void
smtp_mail(char *arg)
{
	if (!addrparse(arg)) {
		err_syntax();
		return;
	}
	flagsize = 0;
	mailfrom_parms(arg);
	seenmail = 1;
	if (!stralloc_copys(&rcptto, ""))
		die_nomem();
	if (!stralloc_copys(&mailfrom, addr.s))
		die_nomem();
	if (!stralloc_0(&mailfrom))
		die_nomem();
	out("250 ok\r\n");
}

void
smtp_rcpt(char *arg)
{
	if (!seenmail) {
		err_wantmail();
		return;
	}
	if (!addrparse(arg)) {
		err_syntax();
		return;
	}
	if (relayclient) {
		--addr.len;
		if (!stralloc_cats(&addr, relayclient) ||
				!stralloc_0(&addr))
			die_nomem();
	} else
	if (!addrallowed()) {
		err_nogateway();
		return;
	}
	if (!stralloc_cats(&rcptto, "T") ||
			!stralloc_cats(&rcptto, addr.s) ||
			!stralloc_0(&rcptto))
		die_nomem();
	out("250 ok\r\n");
}

ssize_t
saferead(int fd, char *buf, size_t len)
{
	int             r;

	flush();
	if ((r = timeoutread(timeout, fd, buf, len)) == -1) {
		if (errno == error_timeout)
			die_alarm();
	}
	if (r <= 0)
		die_read();
	return r;
}

void
put(char *ch)
{
	if (bytestooverflow && !--bytestooverflow)
		qmail_fail(&qqt);
	qmail_put(&qqt, ch, 1);
}

void
blast(int *hops)
{
	char            ch;
	int             err;
	int             state;
	int             flaginheader;
	int             pos;		/* number of bytes since most recent \n, if fih */
	int             flagmaybex;	/* 1 if this line might match RECEIVED, if fih */
	int             flagmaybey;	/* 1 if this line might match \r\n, if fih */
	int             flagmaybez;	/* 1 if this line might match DELIVERED, if fih */
	int             seencr;

	state = 1;
	*hops = 0;
	flaginheader = 1;
	pos = 0;
	flagmaybex = flagmaybey = flagmaybez = 1;
	seencr = 0;	/*- qmail-smtpd-newline patch */
	for (;;) {
		if ((err = substdio_get(&ssin, &ch, 1)) <= 0)
			return;
		if (ch == '\n') {
			if (seencr == 0) {
				substdio_seek(&ssin, -1);
				ch = '\r';
			}
		}
		if (ch == '\r')
			seencr = 1;
		else
			seencr = 0;
		if (flaginheader) {
			if (pos < 9) {
				if (ch != "delivered"[pos])
					if (ch != "DELIVERED"[pos])
						flagmaybez = 0;
				if (flagmaybez)
					if (pos == 8)
						++ * hops;
				if (pos < 8)
					if (ch != "received"[pos])
						if (ch != "RECEIVED"[pos])
							flagmaybex = 0;
				if (flagmaybex)
					if (pos == 7)
						++ * hops;
				if (pos < 2)
					if (ch != "\r\n"[pos])
						flagmaybey = 0;
				if (flagmaybey)
					if (pos == 1)
						flaginheader = 0;
				++pos;
			}
			if (ch == '\n') {
				pos = 0;
				flagmaybex = flagmaybey = flagmaybez = 1;
			}
		}
		switch (state)
		{
		case 0:
			if (ch == '\n')
				straynewline();
			if (ch == '\r') {
				state = 4;
				continue;
			}
			break;
		case 1: /*- \r\n */
			if (ch == '\n')
				straynewline();
			if (ch == '.') {
				state = 2;
				continue;
			}
			if (ch == '\r') {
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 2: /*- \r\n + . */
			if (ch == '\n')
				straynewline();
			if (ch == '\r') {
				state = 3;
				continue;
			}
			state = 0;
			break;
		case 3: /*- \r\n + .\r */
			if (ch == '\n')
				return;
			put(".");
			put("\r");
			if (ch == '\r') {
				state = 4;
				continue;
			}
			state = 0;
			break;
		case 4: /*- + \r */
			if (ch == '\n') {
				state = 1;
				break;
			}
			if (ch != '\r') {
				put("\r");
				state = 0;
			}
		}
		put(&ch);
	} /* for (;;) */
}

char            accept_buf[FMT_ULONG];
void
acceptmessage(unsigned long qp)
{
	datetime_sec    when;
	when = now();
	out("250 ok ");
	accept_buf[fmt_ulong(accept_buf, (unsigned long) when)] = 0;
	out(accept_buf);
	out(" qp ");
	accept_buf[fmt_ulong(accept_buf, qp)] = 0;
	out(accept_buf);
	out("\r\n");
}

void
smtp_data(char *arg)
{
	int             hops;
	unsigned long   qp;
	char           *qqx;

	if (!seenmail) {
		err_wantmail();
		return;
	}
	if (!rcptto.len) {
		err_wantrcpt();
		return;
	}
	seenmail = 0;
	/*- Return error if incoming SMTP msg exceeds DATABYTES */
	if (flagsize) {
		err_size();
		return;
	}
	if (databytes)
		bytestooverflow = databytes + 1;
	if (qmail_open(&qqt) == -1) {
		err_qqt();
		return;
	}
	qp = qmail_qp(&qqt);
	out("354 go ahead\r\n");

	received(&qqt, "mini-smtpd", "SMTP", local, remoteip,
			str_diff(remotehost, "unknown") ? remotehost : 0, remoteinfo, fakehelo);
	blast(&hops);
	hops = (hops >= MAXHOPS);
	if (hops)
		qmail_fail(&qqt);
	if (databytes && !bytestooverflow) {
		err_size();
		return;
	}
	qmail_from(&qqt, mailfrom.s);
	qmail_put(&qqt, rcptto.s, rcptto.len);

	qqx = qmail_close(&qqt);
	if (!*qqx) {
		acceptmessage(qp);
		return;
	}
	if (hops) {
		out("554 too many hops, this message is looping (#5.4.6)\r\n");
		return;
	}
	if (databytes)
		if (!bytestooverflow) {
			out("552 sorry, that message size exceeds my databytes limit (#5.3.4)\r\n");
			return;
		}
	if (*qqx == 'D')
		out("554 ");
	else
		out("451 ");
	out(qqx + 1);
	out("\r\n");
}

static struct commands smtpcommands[] = {
	{ "rcpt", smtp_rcpt, 0 },
	{ "mail", smtp_mail, 0 },
	{ "data", smtp_data, flush },
	{ "quit", smtp_quit, flush },
	{ "helo", smtp_helo, flush },
	{ "ehlo", smtp_ehlo, flush },
	{ "rset", smtp_rset, 0 },
	{ "help", smtp_help, flush },
	{ "noop", err_noop, flush },
	{ "vrfy", err_vrfy, flush },
	{ 0, err_unimpl, flush }
};

int
main(int argc, char **argv)
{
	struct commands *cmdptr;

	sig_pipeignore();
	if (chdir(auto_qmail) == -1)
		die_control();
	setup();
	if (ipme_init() != 1)
		die_ipme();
	smtp_greet("220 ");
	out(" ESMTP\r\n");
	cmdptr = smtpcommands;
	if (commands(&ssin, cmdptr) == 0)
		die_read();
	die_nomem();
}

void
getversion_mini_smtpd()
{
	static char    *x = "$Id: mini-smtpd.c,v 1.6 2023-09-24 00:23:57+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
