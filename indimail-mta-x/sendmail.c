/*
 * $Log: sendmail.c,v $
 * Revision 1.15  2023-12-20 11:14:46+05:30  Cprogrammer
 * added -r option to set mailx return path
 *
 * Revision 1.14  2022-03-05 13:38:12+05:30  Cprogrammer
 * use auto_prefix/sbin for qmail-smtpd, auto_prefix/bin for qmail-inject, qmail-qread path
 *
 * Revision 1.13  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.12  2020-11-24 13:48:05+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.11  2020-05-15 11:00:31+05:30  Cprogrammer
 * use unsigned int to store return value of str_len
 *
 * Revision 1.10  2018-01-31 14:25:07+05:30  Cprogrammer
 * moved qmail-smtpd to sbin
 *
 * Revision 1.9  2010-06-11 16:32:36+05:30  Cprogrammer
 * added more sendmail compatible options (but to be ignored)
 *
 * Revision 1.8  2008-06-01 15:32:47+05:30  Cprogrammer
 * added -N dsn option
 *
 * Revision 1.7  2005-01-22 00:58:56+05:30  Cprogrammer
 * added braces
 *
 * Revision 1.6  2004-10-22 20:30:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-07-17 21:22:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sgetopt.h>
#include <substdio.h>
#include <subfd.h>
#include <alloc.h>
#include <env.h>
#include <str.h>
#include <stralloc.h>
#include <noreturn.h>
#include "auto_prefix.h"

static stralloc q = {0};

no_return void
nomem()
{
	substdio_putsflush(subfderr, "sendmail: fatal: out of memory\n");
	_exit(111);
}

no_return void
die_usage()
{
	substdio_putsflush(subfderr, "sendmail: usage: sendmail [ -t ] [ -fsender ] [ -Fname ] [ -bp ] [ -bs ] [ arg ... ]\n");
	_exit(100);
}

no_return void
smtpd()
{
	char           *smtpdarg[] = { 0, NULL };

	if (!env_get("PROTO")) {
		if (!env_put("RELAYCLIENT="))
			nomem();
		if (!env_put("DATABYTES=0"))
			nomem();
		if (!env_put("PROTO=TCP"))
			nomem();
		if (!env_put("TCPLOCALIP=127.0.0.1"))
			nomem();
		if (!env_put("TCPLOCALHOST=localhost"))
			nomem();
		if (!env_put("TCPREMOTEIP=127.0.0.1"))
			nomem();
		if (!env_put("TCPREMOTEHOST=localhost"))
			nomem();
		if (!env_put("TCPREMOTEINFO=sendmail-bs"))
			nomem();
	}
	if (!stralloc_catb(&q, "/sbin/qmail-smtpd", 17) || !stralloc_0(&q))
		nomem();
	smtpdarg[0] = q.s;
	execv(*smtpdarg, smtpdarg);
	substdio_putsflush(subfderr, "sendmail: fatal: unable to run qmail-smtpd\n");
	_exit(111);
}

no_return void
mailq()
{
	char           *qreadarg[] = { 0, NULL };

	if (!stralloc_catb(&q, "/bin/qmail-qread", 16) || !stralloc_0(&q))
		nomem();
	qreadarg[0] = q.s;
	execv(*qreadarg, qreadarg);
	substdio_putsflush(subfderr, "sendmail: fatal: unable to run qmail-qread\n");
	_exit(111);
}

void
do_sender(const char *s)
{
	char           *x;
	unsigned int    n, a, i;

	env_unset("QMAILNAME");
	env_unset("MAILNAME");
	env_unset("NAME");
	env_unset("QMAILHOST");
	env_unset("MAILHOST");
	n = str_len((char *) s);
	a = str_rchr((char *) s, '@');
	if (a == n) {
		env_put2("QMAILUSER", (char *) s);
		return;
	}
	env_put2("QMAILHOST", (char *) s + a + 1);

	if(!(x = (char *) alloc((a + 1) * sizeof(char))))
		nomem();
	for (i = 0; i < a; i++)
		x[i] = s[i];
	x[i] = 0;
	env_put2("QMAILUSER", x);
	alloc_free(x);
}

int
main(int argc, char **argv)
{
	int             opt, i, flagh;
	char          **qiargv, **arg;
	char           *sender;

	if (chdir("/") == -1) {
		substdio_putsflush(subfderr, "sendmail: fatal: unable to switch to root directory\n");
		_exit(111);
	}
	if (!stralloc_copys(&q, auto_prefix))
		nomem();

	flagh = 0;
	sender = 0;
	while ((opt = getopt(argc, argv, "ULhnIAvimte:f:r:p:o:B:F:EJxb:")) != opteof) {
		switch (opt)
		{
		case 'B': /*- ignored */
			break;
		case 'A':
			switch (optarg[0])
			{
			case 'm': /*- ignored */
				break;
			case 'c': /*- ignored */
				break;
			default:
				die_usage();
			}
			break;
		case 'L': /*- ignored */
		case 'h': /*- ignored */
		case 'n': /*- ignored */
		case 'U': /*- ignored */
			break;
		case 'I':
			substdio_putsflush(subfderr, "sendmail: fatal: please use fastforward/newaliases instead\n");
			_exit(100);
		case 't':
			flagh = 1;
			break;
		case 'f':
		case 'r':
			sender = optarg;
			break;
		case 'N': /* ignore DSN option */
			break;
		case 'F':
			if (!env_put2("MAILNAME", optarg))
				nomem();
			break;
		case 'p':
			break;	/*- could generate a Received line from optarg */
		case 'v':
			break;
		case 'i':
			break;	/*- what an absurd concept */
		case 'x':
			break;	/*- SVR4 stupidity */
		case 'm':
			break;	/*- twisted-paper-path blindness, incompetent design */
		case 'e':
			break;	/*- qmail has only one error mode */
		case 'o':
			switch (optarg[0])
			{
			case 'd':
				break;	/*- qmail has only one delivery mode */
			case 'e':
				break;	/*- see 'e' above */
			case 'i':
				break;	/*- see 'i' above */
			case 'm':
				break;	/*- see 'm' above */
			case 'o':
			case '7':   /*- ignored */
			case '8':
				break;
			}
			break;
		case 'E':
		case 'J':	/*- Sony NEWS-OS */
			while (argv[optind][optpos])
				++optpos;		/* skip optional argument */
			break;
		case 'b':
			switch (optarg[0])
			{
			case 'h':
			case 'H':
			case 'm':
				break;
			case 'p':
				mailq();
			case 's':
				smtpd();
			case 'i':
				substdio_putsflush(subfderr, "sendmail: fatal: please use fastforward/newaliases instead\n");
				_exit(100);
			default:
				die_usage();
			}
			break;
		default:
			die_usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (str_equal(optprogname, "mailq"))
		mailq();
	if (str_equal(optprogname, "newaliases")) {
		substdio_putsflush(subfderr, "sendmail: fatal: please use fastforward/newaliases instead\n");
		_exit(100);
	}
	qiargv = (char **) alloc((argc + 10) * sizeof(char *));
	if (!qiargv)
		nomem();
	arg = qiargv;
	if (!stralloc_catb(&q, "/bin/qmail-inject", 17) || !stralloc_0(&q))
		nomem();
	*arg++ = q.s;
	*arg++ = (flagh ? "-H" : "-a");
	if (sender) {
		*arg++ = "-f";
		*arg++ = sender;
    	do_sender(sender);
	}
	*arg++ = "--";
	for (i = 0; i < argc; ++i)
		*arg++ = argv[i];
	*arg = 0;
	execv(*qiargv, qiargv);
	substdio_putsflush(subfderr, "sendmail: fatal: unable to run qmail-inject\n");
	_exit(111);
	/*- Not reached */
	return(0);
}

void
getversion_sendmail_c()
{
	static char    *x = "$Id: sendmail.c,v 1.15 2023-12-20 11:14:46+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
