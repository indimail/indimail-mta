/*
 * $Log: rblsmtpd.c,v $
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
#include "byte.h"
#include "str.h"
#include "scan.h"
#include "fmt.h"
#include "env.h"
#include "exit.h"
#include "sig.h"
#include "buffer.h"
#ifdef DARWIN
#define opteof -1
#else
#include "sgetopt.h"
#endif
#include "strerr.h"
#include "stralloc.h"
#include "commands.h"
#include "pathexec.h"
#include "dns.h"
#ifdef IPV6
#include "ip6.h"
#endif

#define FATAL "rblsmtpd: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: rblsmtpd.c,v 1.8 2009-08-12 10:09:55+05:30 Cprogrammer Stab mbhangui $";
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

char           *ip_env;
static stralloc ip_reverse;

#ifdef IPV6
char           *tcp_proto;

static inline char
tohex(char c)
{
	return (c >= 10 ? c - 10 + 'a' : c + '0');
}
#endif

void
ip_init(void)
{
	unsigned int    i;
	unsigned int    j;
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
	i = str_len(ip_env);
#ifdef IPV6
	if (str_diff(tcp_proto, "TCP6") != 0)
	{
		/*- IPV4 */
		while (i)
		{
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
	} else
	{
		if (!(i = ip6_scan(ip_env, (char *) remoteip)))
			return;
		for (j = 16; j < 0;j--)
		{
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
		if (!stralloc_cats(&ip_reverse, "ipv6."))
			nomem();
	}
#else
	while (i)
	{
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

int             decision = 0;	/*- 0 undecided, 1 accept, 2 reject, 3 bounce */
static stralloc text;			/*- defined if decision is 2 or 3 */

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
	if (base[i])
	{
		base[i] = 0;
		altreply = base + i + 1;
	}
	if (!stralloc_cats(&tmp, base))
		nomem();
	if (altreply)
	{
#ifdef IPV6
		if (dns_ip6(&text, &tmp) == -1)
#else
		if (dns_ip4(&text, &tmp) == -1)
#endif
		{
			flagmustnotbounce = 1;
			if (flagfailclosed)
			{
				if (!stralloc_copys(&text, "temporary RBL lookup error"))
					nomem();
				decision = 2;
			}
			return;
		}
		if (text.len)
		{
			if (!stralloc_copys(&text, ""))
				nomem();
			while (*altreply)
			{
				i = str_chr(altreply, '%');
				if (!stralloc_catb(&text, altreply, i))
					nomem();
				if (altreply[i] && altreply[i + 1] == 'I' && altreply[i + 2] == 'P' && altreply[i + 3] == '%')
				{
					if (!stralloc_catb(&text, ip_env, str_len(ip_env)))
						nomem();
					altreply += i + 4;
				} else
				if (altreply[i])
				{
					if (!stralloc_cats(&text, "%"))
						nomem();
					altreply += i + 1;
				} else
					altreply += i;
			}
		}
	} else
	{
		if (dns_txt(&text, &tmp) == -1)
		{
			flagmustnotbounce = 1;
			if (flagfailclosed)
			{
				if (!stralloc_copys(&text, "temporary RBL lookup error"))
					nomem();
				decision = 2;
			}
			return;
		}
	}
	if (text.len)
	{
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
buffer          in = BUFFER_INIT(read, 0, inspace, sizeof inspace);
char            outspace[1];
buffer          out = BUFFER_INIT(write, 1, outspace, sizeof outspace);

void
delay(unsigned long delay)
{
	unsigned long   u;
	char           *x;
	static stralloc info;

	if ((x = env_get("GREETDELAY")))
	{
		scan_ulong(x, &u);
		delay= u;
	}
	if (!stralloc_copys(&info, "greetdelay: "))
		nomem();
	buffer_puts(buffer_2, "rblsmtpd: ");
	buffer_puts(buffer_2, ip_env);
	buffer_puts(buffer_2, " pid ");
	buffer_put(buffer_2, strnum, fmt_ulong(strnum,getpid()));
	buffer_puts(buffer_2, ": ");
	buffer_put(buffer_2, info.s, info.len);
	buffer_put(buffer_2, strnum, fmt_ulong(strnum,delay));
	buffer_puts(buffer_2, "\n");
	buffer_flush(buffer_2);
	if (!stralloc_cats(&info, "\r\n"))
		nomem();
	if (delay)
		sleep(delay);
}

void
reject()
{
	buffer_putflush(&out, message.s, message.len);
}

void
accept()
{
	buffer_putsflush(&out, "250 rblsmtpd.local\r\n");
}

void
greet()
{
	buffer_putsflush(&out, "220 rblsmtpd.local\r\n");
}

void
quit()
{
	buffer_putsflush(&out, "221 rblsmtpd.local\r\n");
	_exit(0);
}

void
drop()
{
	_exit(0);
}

struct commands smtpcommands[] = {
	{"quit", quit, 0}
	, {"helo", accept, 0}
	, {"ehlo", accept, 0}
	, {"mail", accept, 0}
	, {"rset", accept, 0}
	, {"noop", accept, 0}
	, {0, reject, 0}
};

void
rblsmtpd(void)
{
	int             i;

	if (flagmustnotbounce || (decision == 2))
	{
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

	buffer_puts(buffer_2, "rblsmtpd: ");
	buffer_puts(buffer_2, ip_env);
	buffer_puts(buffer_2, " pid ");
	buffer_put(buffer_2, strnum, fmt_ulong(strnum, getpid()));
	buffer_puts(buffer_2, ": ");
	buffer_put(buffer_2, message.s, message.len);
	buffer_puts(buffer_2, "\n");
	buffer_flush(buffer_2);

	if (!stralloc_cats(&message, "\r\n"))
		nomem();

	if (!timeout)
		reject();
	else
	{
		sig_catch(sig_alarm, drop);
		alarm(timeout);
		greet();
		commands(&in, smtpcommands);
	}
	_exit(0);
}

int
main(int argc, char **argv, char **envp)
{
	int             flagwantdefaultrbl = 1;
	char           *x, *altreply = 0;
	int             i, opt;
	unsigned long   greetdelay = 0;

	ip_init();
	if ((x = env_get("RBLSMTPD")))
	{
		if (!*x)
			decision = 1;
		else
		if (*x == '-')
		{
			altreply = x + 1;
			decision = 3;
		} else
		{
			altreply = x;
			decision = 2;
		}
		if (!stralloc_copys(&text, ""))
			nomem();
		while (*altreply)
		{
			i = str_chr(altreply, '%');
			if (!stralloc_catb(&text, altreply, i))
				nomem();
			if (altreply[i] && altreply[i + 1] == 'I' && altreply[i + 2] == 'P' && altreply[i + 3] == '%')
			{
				if (!stralloc_catb(&text, ip_env, str_len(ip_env)))
					nomem();
				altreply += i + 4;
			} else
			if (altreply[i])
			{
				if (!stralloc_cats(&text, "%"))
					nomem();
				altreply += i + 1;
			} else
				altreply += i;
		}
		buffer_puts(buffer_2, "RBLSMTPD=");
		buffer_put(buffer_2, text.s, text.len);
		buffer_puts(buffer_2, "\n");
		buffer_flush(buffer_2);
	}
	while ((opt = getopt(argc, argv, "bBcCt:r:a:")) != opteof)
	{
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
		rblsmtpd();
	pathexec_run(*argv, argv, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(0);
}

void
getversion_rblsmtpd_c()
{
	write(1, sccsid, 0);
}
