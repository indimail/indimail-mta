/*
 * $Log: rblsmtpd.c,v $
 * Revision 1.21  2021-01-28 18:22:39+05:30  Cprogrammer
 * added ehlo() function
 * change greeting using RBLGREETING env variable
 *
 * Revision 1.20  2020-09-16 20:49:57+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 1.19  2020-08-03 17:25:31+05:30  Cprogrammer
 * replaced buffer with substdio
 *
 * Revision 1.18  2020-06-08 22:48:38+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.17  2018-06-30 16:34:04+05:30  Cprogrammer
 * replace environ with envp to pass full environment variable list in tcpserver to rbl()
 *
 * Revision 1.16  2017-03-30 22:59:08+05:30  Cprogrammer
 * prefix rbl with ip6_scan(), dns_txt(), smtp_mail(), smtp_rcpt(), smtpcommands - avoid duplicate symb in rblsmtpd.so with qmail_smtpd.so
 *
 * Revision 1.15  2017-03-01 22:52:13+05:30  Cprogrammer
 * reset optind as it gets called in tcpserver and rblsmtpd.so might be loaded as shared object
 *
 * Revision 1.14  2016-05-15 22:41:42+05:30  Cprogrammer
 * rblsmtpd() function for use as shared object
 *
 * Revision 1.13  2015-08-27 00:25:28+05:30  Cprogrammer
 * removed tohex() function
 *
 * Revision 1.12  2014-01-10 00:06:57+05:30  Cprogrammer
 * BUG - flagip6 was not initialized
 *
 * Revision 1.11  2013-08-06 11:02:30+05:30  Cprogrammer
 * support for IPv4-mapped IPv6 addresses and supports the inverse IPv6 nibble format for rblsmtpd RBL and anti-RBL lookups.
 *
 * Revision 1.10  2010-02-22 20:48:16+05:30  Cprogrammer
 * fixed SIGSEGV when RBLSMTPD was set and empty
 *
 * Revision 1.9  2010-02-22 15:21:54+05:30  Cprogrammer
 * log sender, recipients
 *
 * Revision 1.8  2009-08-12 10:09:55+05:30  Cprogrammer
 * IPV6 Modifications
 *
 * Revision 1.7  2009-01-03 13:05:40+05:30  Cprogrammer
 * added greetdelay functionality, %IP% substitution
 *
 * Revision 1.6  2008-07-25 16:49:33+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.5  2008-07-17 23:04:02+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2007-06-10 10:16:03+05:30  Cprogrammer
 * fixed usage description
 *
 * Revision 1.3  2005-06-10 12:11:22+05:30  Cprogrammer
 * conditional ipv6 compilation
 *
 * Revision 1.2  2005-06-03 09:07:55+05:30  Cprogrammer
 * removed default base rbl.maps.vix.com
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <byte.h>
#include <str.h>
#include <scan.h>
#include <fmt.h>
#include <env.h>
#include <sig.h>
#include <substdio.h>
#include <subfd.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <strerr.h>
#include <stralloc.h>
#include "commands.h"
#include "pathexec.h"
#include "dns.h"
#ifdef IPV6
#include "ip6.h"
#endif

#define FATAL "rblsmtpd: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: rblsmtpd.c,v 1.21 2021-01-28 18:22:39+05:30 Cprogrammer Exp mbhangui $";
#endif

void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
usage(void)
{
	strerr_die1x(100, "usage: rblsmtpd -r base [ -b ] [ -R ] [ -t timeout ] [ -a base ] [-W] [-w delay] smtpd [ arg ... ]");
}

char           *ip_env, *rbl_greeting, *rbl_ehlo;
char            pid_str[FMT_ULONG] = "?PID?";
static stralloc addr = { 0 };
static stralloc ip_reverse;

/*
 * Idea from Andrew Richards http://free.acrconsulting.co.uk
 */
void
rbl_out(int should_flush, char *arg)
{
	substdio_puts(subfderr, "rblsmtpd: ");
	substdio_puts(subfderr, " pid ");
	if (*pid_str == '?')
		pid_str[fmt_ulong(pid_str, getpid())] = 0;
	substdio_puts(subfderr, pid_str);
	substdio_puts(subfderr, " from ");
	substdio_puts(subfderr, ip_env);
	if (arg && *arg) {
		substdio_puts(subfderr, ": ");
		substdio_puts(subfderr, arg);
	}
	if (should_flush)
		substdio_flush(subfderr);
}

#ifdef IPV6
char           *tcp_proto;
#endif

void
ip_init(void)
{
	unsigned int    i;
	unsigned int    j;
	int             flagip6 = 0;
#ifdef IPV6
	unsigned char   remoteip[16];
	char            hexval;
#endif

#ifdef IPV6
	if (!(tcp_proto = env_get("PROTO")))
		tcp_proto = "";
#endif
	if (!(ip_env = env_get("TCPREMOTEIP")))
		ip_env = "";
	if (!stralloc_copys(&ip_reverse, ""))
		nomem();
#ifdef IPV6
	if (str_diff(tcp_proto, "TCP6") == 0) {
		if (byte_equal(ip_env, 7, (char *) V4MAPPREFIX))
			ip_env = ip_env + 7;
		else
			flagip6 = 1;
	}
	if (flagip6) {
		if ((rblip6_scan(ip_env, (char *) remoteip)) == 0)
			return;
		for (j = 16; j > 0; j--) {
			hexval = tohex(remoteip[j - 1] & 15);
			if (!stralloc_catb(&ip_reverse, &hexval, 1))
				nomem();
			if (!stralloc_cats(&ip_reverse, "."))
				nomem();
			hexval = tohex(remoteip[j - 1] >> 4);
			if (!stralloc_catb(&ip_reverse, &hexval, 1))
				nomem();
			if (!stralloc_cats(&ip_reverse, "."))
				nomem();
		}
	} else {
		/*- IPV4 */
		i = str_len(ip_env);
		while (i) {
			for (j = i; j > 0; --j)
				if (ip_env[j - 1] == '.')
					break;
			if (!stralloc_catb(&ip_reverse, ip_env + j, i - j))
				nomem();
			if (!stralloc_cats(&ip_reverse, "."))
				nomem();
			if (!j)
				break;
			i = j - 1;
		}
	}
#else
	i = str_len(ip_env);
	while (i) {
		for (j = i; j > 0; --j)
			if (ip_env[j - 1] == '.')
				break;
		if (!stralloc_catb(&ip_reverse, ip_env + j, i - j))
			nomem();
		if (!stralloc_cats(&ip_reverse, "."))
			nomem();
		if (!j)
			break;
		i = j - 1;
	}
#endif
}

unsigned long   timeout = 60;
int             flagrblbounce = 0;
int             flagfailclosed = 0;
int             flagmustnotbounce = 0;

int             decision = 0;	/*- 0 undecided, 1 accept, 2 reject */
static stralloc text;			/*- 3 bounce defined if decision is 2 or 3 */
static stralloc tmp;

void
rbl(char *base)
{
	int             i;
	char           *altreply = 0;

	if (decision)
		return;
	if (!stralloc_copy(&tmp, &ip_reverse))
		nomem();
	i = str_chr(base, ':');
	if (base[i]) {
		base[i] = 0;
		altreply = base + i + 1;
	}
	if (!stralloc_cats(&tmp, base))
		nomem();
	if (altreply) {
#ifdef IPV6
		if (dns_ip6(&text, &tmp) == -1)
#else
		if (dns_ip4(&text, &tmp) == -1)
#endif
		{
			flagmustnotbounce = 1;
			if (flagfailclosed) {
				if (!stralloc_copys(&text, "temporary RBL lookup error"))
					nomem();
				decision = 2;
			}
			return;
		}
		if (text.len) {
			if (!stralloc_copys(&text, ""))
				nomem();
			while (*altreply) {
				i = str_chr(altreply, '%');
				if (!stralloc_catb(&text, altreply, i))
					nomem();
				if (altreply[i] && altreply[i + 1] == 'I' && altreply[i + 2] == 'P' && altreply[i + 3] == '%') {
					if (!stralloc_catb(&text, ip_env, str_len(ip_env)))
						nomem();
					altreply += i + 4;
				} else
				if (altreply[i]) {
					if (!stralloc_cats(&text, "%"))
						nomem();
					altreply += i + 1;
				} else
					altreply += i;
			}
		}
	} else {
		if (rbl_dns_txt(&text, &tmp) == -1) {
			flagmustnotbounce = 1;
			if (flagfailclosed) {
				if (!stralloc_copys(&text, "temporary RBL lookup error"))
					nomem();
				decision = 2;
			}
			return;
		}
	}
	if (text.len) {
		if (flagrblbounce)
			decision = 3;
		else
			decision = 2;
	}
}

void
antirbl(char *base)
{
	if (decision)
		return;
	if (!stralloc_copy(&tmp, &ip_reverse))
		nomem();
	if (!stralloc_cats(&tmp, base))
		nomem();
#ifdef IPV6
	if (dns_ip6(&text, &tmp) == -1)
#else
	if (dns_ip4(&text, &tmp) == -1)
#endif
	{
		flagmustnotbounce = 1;
		if (!flagfailclosed)
			decision = 1;
		return;
	}
	if (text.len)
		decision = 1;
}

char            strnum[FMT_ULONG];
static stralloc message;

char            inspace[64];
substdio        in = SUBSTDIO_FDBUF(read, 0, inspace, sizeof inspace);
char            outspace[1];
substdio        out = SUBSTDIO_FDBUF(write, 1, outspace, sizeof outspace);

void
delay(unsigned long delay)
{
	unsigned long   u;
	char           *x;
	static stralloc info;

	if ((x = env_get("GREETDELAY"))) {
		scan_ulong(x, &u);
		delay= u;
	}
	if (!stralloc_copys(&info, "greetdelay: "))
		nomem();
	rbl_out(0, 0);
	substdio_puts(subfderr, ": ");
	substdio_put(subfderr, info.s, info.len);
	substdio_put(subfderr, strnum, fmt_ulong(strnum,delay));
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	if (!stralloc_cats(&info, "\r\n"))
		nomem();
	if (delay)
		sleep(delay);
}

void
reject()
{
	substdio_putflush(&out, message.s, message.len);
}

void
accept()
{
  substdio_put(&out, "250 ", 4);
  substdio_puts(&out, rbl_greeting);
  substdio_putsflush(&out, "\r\n");
}

void
smtp_ehlo()
{
  if (rbl_ehlo) {
  substdio_put(&out, "250-", 4);
  substdio_puts(&out, rbl_greeting);
  substdio_putsflush(&out, 
    "\r\n250-PIPELINING\r\n250-8BITMIME\r\n250-STARTTLS\r\n250 HELP\r\n");
  } else
    accept();
}

void
verify()
{
  substdio_put(&out, "252 ", 4);
  substdio_puts(&out, rbl_greeting);
  substdio_putsflush(&out, "\r\n");
}

static int
addrparse(char *arg)
{
	int             i;
	char            ch;
	char            terminator;
	int             flagesc;
	int             flagquoted;

	terminator = '>';
	i = str_chr(arg, '<');
	if (arg[i])
		arg += i + 1;
	else {
		/*- partner should go read rfc 821 */
		terminator = ' ';
		arg += str_chr(arg, ':');
		if (*arg == ':')
			++arg;
		if (!*arg)
			return (0);
		while (*arg == ' ')
			++arg;
	}
	/*- strip source route */
	if (*arg == '@') {
		while (*arg) {
			if (*arg++ == ':')
				break;
		}
	}
	if (!stralloc_copys(&addr, ""))
		nomem();
	flagesc = 0;
	flagquoted = 0;
	for (i = 0; (ch = arg[i]); ++i) {
		/*- copy arg to addr, stripping quotes */
		if (flagesc) {
			if (!stralloc_append(&addr, &ch))
				nomem();
			flagesc = 0;
		} else {
			if (!flagquoted && ch == terminator)
				break;
			switch (ch)
			{
			case '\\':
				flagesc = 1;
				break;
#ifdef STRIPSINGLEQUOTES
			case '\'':
				flagquoted = !flagquoted;
				break;
#endif
			case '"':
				flagquoted = !flagquoted;
				break;
			default:
				if (!stralloc_append(&addr, &ch))
					nomem();
			}
		}
	}
	/*- could check for termination failure here, but why bother? */
	if (!stralloc_append(&addr, ""))
		nomem();
	if (addr.len > 900)
		return 0;
	return 1;
}

void
rblsmtp_mail(char *arg)
{
	rbl_out(1, 0);
	if (!addrparse(arg))
		substdio_puts(subfderr, ": MAIL with too long address\n");
	else {
		substdio_puts(subfderr, ": Sender <");
		substdio_puts(subfderr, addr.s);
		substdio_puts(subfderr, ">\n");
	}
	substdio_flush(subfderr);
	accept();
}

void
rblsmtp_rcpt(char *arg)
{
	rbl_out(1, 0);
	if (!addrparse(arg))
		substdio_puts(subfderr, ": RCPT with too long address\n");
	else {
		substdio_puts(subfderr, ": Recipient <");
		substdio_puts(subfderr, addr.s);
		substdio_puts(subfderr, ">\n");
	}
	substdio_flush(subfderr);
	reject();
}

void
greet()
{
  substdio_put(&out, "220 ", 4);
  substdio_puts(&out, rbl_greeting);
  substdio_putsflush(&out, "\r\n");
}

void
quit()
{
  substdio_put(&out, "221 ", 4);
  substdio_puts(&out, rbl_greeting);
  substdio_putsflush(&out, "\r\n");
  _exit(0);
}

void
drop()
{
	_exit(0);
}

struct commands rbl_smtpcommands[] = {
	{"quit", quit, 0},
	{"helo", accept, 0},
	{"ehlo", smtp_ehlo, 0},
	{"mail", rblsmtp_mail, 0},
	{"rcpt", rblsmtp_rcpt, 0},
	{"rset", accept, 0},
	{"vrfy", verify, 0},
	{"noop", accept, 0},
	{0, reject, 0}
};

void
rblsmtpd_f(void)
{
	int             i;

	if (flagmustnotbounce || (decision == 2)) {
		if (!stralloc_copys(&message, "451 "))
			nomem();
	} else
	if (!stralloc_copys(&message, "553 "))
		nomem();
	if (text.len > 200)
		text.len = 200;
	if (!stralloc_cat(&message, &text))
		nomem();
	for (i = 0; i < message.len; ++i)
		if ((message.s[i] < 32) || (message.s[i] > 126))
			message.s[i] = '?';
	rbl_out(0, 0);
	substdio_puts(subfderr, ": ");
	substdio_put(subfderr, message.s, message.len);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	if (!stralloc_cats(&message, "\r\n"))
		nomem();
	if (!timeout)
		reject();
	else {
		sig_catch(sig_alarm, drop);
		alarm(timeout);
		greet();
		commands(&in, rbl_smtpcommands);
	}
	rbl_out(1, ": Session terminated: quitting\n");
	_exit(0);
}

int
rblsmtpd(int argc, char **argv, char **envp)
{
	int             flagwantdefaultrbl = 1;
	char           *x, *altreply = 0;
	char          **e;
	int             i, opt;
	unsigned long   greetdelay = 0;

	e = environ;
	environ = envp;
	ip_init();
	if (!(rbl_greeting = env_get("RBLGREETING")))
		rbl_greeting = "rblsmtpd.indimail";
	rbl_ehlo = env_get("RBLEHLO");
	if ((x = env_get("RBLSMTPD"))) {
		if (!*x)
			decision = 1;
		else
		if (*x == '-') {
			altreply = x + 1;
			decision = 3;
			if (!stralloc_copys(&text, ""))
				nomem();
		} else {
			altreply = x;
			decision = 2;
			if (!stralloc_copys(&text, ""))
				nomem();
		}
		while (altreply && *altreply) {
			i = str_chr(altreply, '%');
			if (!stralloc_catb(&text, altreply, i))
				nomem();
			if (altreply[i] && altreply[i + 1] == 'I' && altreply[i + 2] == 'P' && altreply[i + 3] == '%') {
				if (!stralloc_catb(&text, ip_env, str_len(ip_env)))
					nomem();
				altreply += i + 4;
			} else
			if (altreply[i]) {
				if (!stralloc_cats(&text, "%"))
					nomem();
				altreply += i + 1;
			} else
				altreply += i;
		}
#if 0
		substdio_puts(subfderr, "RBLSMTPD=");
		substdio_put(subfderr, text.s, text.len);
		substdio_puts(subfderr, "\n");
		substdio_flush(subfderr);
#endif
	}
	environ = e;
	/*- reset optind as it gets called in tcpserver and rblsmtpd.so might be loaded as shared object */
	optind = 1;
	while ((opt = getopt(argc, argv, "bBcCt:r:a:")) != opteof) {
		switch (opt)
		{
		case 'b':
			flagrblbounce = 1;
			break;
		case 'B':
			flagrblbounce = 0;
			break;
		case 'c':
			flagfailclosed = 1;
			break;
		case 'C':
			flagfailclosed = 0;
			break;
		case 't':
			scan_ulong(optarg, &timeout);
			break;
		case 'r':
			rbl(optarg);
			flagwantdefaultrbl = 0;
			break;
		case 'W':
			delay(greetdelay);
			break;
		case 'w':
			scan_ulong(optarg, &greetdelay);
			delay(greetdelay);
			break;
		case 'a':
			antirbl(optarg);
			break;
		default:
			usage();
		}
	}
	argv += optind;
	if (!*argv)
		usage();
	if (flagwantdefaultrbl)
		usage();
	if (decision >= 2)
		rblsmtpd_f();
	pathexec_run(*argv, argv, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return (0);
}

#ifdef MAIN
int
main(int argc, char **argv, char **envp)
{
	return (rblsmtpd(argc, argv, envp));
}
#endif

void
getversion_rblsmtpd_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
