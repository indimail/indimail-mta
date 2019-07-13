/*
 * $Log: dot-forward.c,v $
 * Revision 1.7  2019-06-07 11:26:04+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.6  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.5  2010-06-08 21:57:35+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.4  2008-07-15 19:50:45+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.3  2004-10-22 20:24:43+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:34:58+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-10-21 22:46:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "envdir.h"
#include "pathexec.h"
#include "stralloc.h"
#include "getln.h"
#include "strerr.h"
#include "error.h"
#include "exit.h"
#include "open.h"
#include "fd.h"
#include "sig.h"
#include "case.h"
#include "wait.h"
#include "seek.h"
#include "variables.h"
#include "env.h"
#include "str.h"
#include "fmt.h"
#include "token822.h"
#include "control.h"
#include "qmail.h"
#include "sgetopt.h"
#include "auto_qmail.h"
#include "auto_control.h"

#define FATAL "dot-forward: fatal: "
#define INFO "dot-forward: info: "

ssize_t         mywrite(int, char *, int);

stralloc        line = { 0 };
int             flagdoit = 1;
int             flagacted;
int             flagdirect;
char           *ufline;
char           *rpline;
char           *dtline;
char           *sender;
char           *user;
int             userlen;
char           *host;
int             hostlen;
char            messbuf[1024];
substdio        ssmess;
char            childbuf[1024];
substdio        sschild;
stralloc        targets = { 0 };
stralloc        me = { 0 };
stralloc        defaulthost = { 0 };
stralloc        defaultdomain = { 0 };
stralloc        plusdomain = { 0 };
stralloc        cbuf = { 0 };
token822_alloc  toks = { 0 };
token822_alloc  tokaddr = { 0 };
stralloc        address = { 0 };
struct qmail    qq;
unsigned long   qp;
char           *qqx;
char            strnum[FMT_ULONG];
char            qqbuf[256];
substdio        ssqq = SUBSTDIO_FDBUF(mywrite, -1, qqbuf, sizeof qqbuf);
char            inbuf[256];

void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
die_control()
{
	strerr_die2sys(111, FATAL, "unable to read controls: ");
}

void
die_qq()
{
	strerr_die2sys(111, FATAL, "unable to run qq: ");
}

void
die_readmess()
{
	strerr_die2sys(111, FATAL, "unable to read message: ");
}

void
die_parse()
{
	if (!stralloc_0(&line))
		die_nomem();
	strerr_die3x(111, FATAL, "unable to parse this line: ", line.s);
}

ssize_t
blindwrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	write(fd, buf, len);
	return len;
}

void
run(cmd)
	char           *cmd;
{
	int             child;
	int             pi[2];
	char           *args[4];
	int             wstat;

	if (!flagdoit)
	{
		strerr_warn2("pipe through ", cmd, 0);
		return;
	}
	if (pipe(pi) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	switch (child = fork())
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
	case 0:
		close(pi[1]);
		if (fd_move(0, pi[0]) == -1)
			strerr_die2sys(111, FATAL, "unable to set fd: ");
		args[0] = "/bin/sh";
		args[1] = "-c";
		args[2] = cmd;
		args[3] = 0;
		sig_pipedefault();
		execv(*args, args);
		strerr_die2sys(111, FATAL, "unable to run /bin/sh: ");
	}
	close(pi[0]);
	substdio_fdbuf(&ssmess, read, 0, messbuf, sizeof messbuf);
	substdio_fdbuf(&sschild, blindwrite, pi[1], childbuf, sizeof childbuf);
	substdio_puts(&sschild, ufline);
	substdio_puts(&sschild, rpline);
	substdio_puts(&sschild, dtline);
	if (substdio_copy(&sschild, &ssmess) != 0)
		die_readmess();
	substdio_flush(&sschild);
	close(pi[1]);
	wait_pid(&wstat, child);
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 100:
	case 64:
	case 65:
	case 70:
	case 76:
	case 77:
	case 78:
	case 112:
		_exit(100);
	case 0:
		break;
	default:
		_exit(111);
	}
	if (seek_begin(0) == -1)
		strerr_die2sys(111, FATAL, "unable to rewind input: ");
}

void
readcontrols()
{
	int             r, fddir;
	char           *qbase;
	char          **e;

	if ((fddir = open_read(".")) == -1)
		strerr_die2sys(111, FATAL, "unable to open current directory: ");
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");
	if (!(qbase = env_get("QUEUE_BASE")))
	{
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (chdir(controldir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", controldir, ": ");
		if (!access("defaultqueue", X_OK))
		{
			envdir_set("defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		}
	}
	if ((r = control_readline(&me, "me")) == -1)
		die_control();
	if (!r && !stralloc_copys(&me, "me"))
		die_nomem();
	if ((r = control_readline(&defaultdomain, "defaultdomain")) == -1)
		die_control();
	if (!r && !stralloc_copy(&defaultdomain, &me))
		die_nomem();
	if ((r = control_readline(&defaulthost, "defaulthost")) == -1)
		die_control();
	if (!r && !stralloc_copy(&defaulthost, &me))
		die_nomem();
	if ((r = control_readline(&plusdomain, "plusdomain")) == -1)
		die_control();
	if (!r && !stralloc_copy(&plusdomain, &me))
		die_nomem();
	if (fchdir(fddir) == -1)
		strerr_die2sys(111, FATAL, "unable to set current directory: ");
}

void
gotaddr()
{
	int             i;
	int             j;
	int             flaghasat;

	token822_reverse(&tokaddr);
	if (token822_unquote(&address, &tokaddr) != 1)
		die_nomem();
	flaghasat = 0;
	for (i = 0; i < tokaddr.len; ++i)
	{
		if (tokaddr.t[i].type == TOKEN822_AT)
			flaghasat = 1;
	}
	tokaddr.len = 0;
	if (!address.len)
		return;
	if (!flaghasat && address.len == userlen && !case_diffb(address.s, address.len, user))
	{
		flagacted = 1;
		flagdirect = 1;
		return;
	}
	if (flaghasat && address.len == userlen + 1 + hostlen)
	{
		if (!case_diffb(address.s, userlen, user) && address.s[userlen] == '@')
		{
			if (!case_diffb(address.s + userlen + 1, hostlen, host))
			{
				flagacted = 1;
				flagdirect = 1;
				return;
			}
		}
	}
	if (!flaghasat && address.s[0] == '/')
	{
		if (!stralloc_0(&address))
			die_nomem();
		strerr_die4x(111, FATAL, "file delivery ", address.s, " not supported");
	}
	if (!flaghasat && address.s[0] == '|')
	{
		if (!stralloc_0(&address))
			die_nomem();
		flagacted = 1;
		run(address.s + 1);
		return;
	}
	if (!flaghasat)
	{
		if (!stralloc_cats(&address, "@"))
			die_nomem();
		if (!stralloc_cat(&address, &defaulthost))
			die_nomem();
	}
	if (address.s[address.len - 1] == '+')
	{
		address.s[address.len - 1] = '.';
		if (!stralloc_cat(&address, &plusdomain))
			die_nomem();
	}
	j = 0;
	for (i = 0; i < address.len; ++i)
		if (address.s[i] == '@')
			j = i;
	for (i = j; i < address.len; ++i)
		if (address.s[i] == '.')
			break;
	if (i == address.len)
	{
		if (!stralloc_cats(&address, "."))
			die_nomem();
		if (!stralloc_cat(&address, &defaultdomain))
			die_nomem();
	}
	if (!stralloc_0(&address))
		die_nomem();
	if (!stralloc_cats(&targets, "T"))
		die_nomem();
	if (!stralloc_cats(&targets, address.s))
		die_nomem();
	if (!stralloc_0(&targets))
		die_nomem();
	if (!flagdoit)
		strerr_warn2("forward ", address.s, 0);
}

void
parseline()
{
	int             wordok;
	struct token822 *t;
	struct token822 *beginning;
	int             r;

	if ((r = token822_parse(&toks, &line, &cbuf)) == -1)
		die_nomem();
	if (r == 0)
		die_parse();
	beginning = toks.t;
	t = toks.t + toks.len;
	wordok = 1;
	if (!token822_readyplus(&tokaddr, 1))
		die_nomem();
	tokaddr.len = 0;
	while (t > beginning)
	{
		switch ((--t)->type)
		{
		case TOKEN822_SEMI:
			 /*XXX*/ break;
		case TOKEN822_COLON:
			 /*XXX*/ break;
		case TOKEN822_RIGHT:
			if (tokaddr.len)
				gotaddr();
			while ((t > beginning) && (t[-1].type != TOKEN822_LEFT))
			{
				if (!token822_append(&tokaddr, --t))
					die_nomem();
			}
			gotaddr();
			if (t <= beginning)
				die_parse();
			--t;
			while ((t > beginning) &&
				   ((t[-1].type == TOKEN822_COMMENT) || (t[-1].type == TOKEN822_ATOM) || (t[-1].type == TOKEN822_QUOTE) ||
					(t[-1].type == TOKEN822_AT) || (t[-1].type == TOKEN822_DOT)))
				--t;
			wordok = 0;
			continue;
		case TOKEN822_ATOM:
		case TOKEN822_QUOTE:
		case TOKEN822_LITERAL:
			if (!wordok && tokaddr.len)
				gotaddr();
			wordok = 0;
			if (!token822_append(&tokaddr, t))
				die_nomem();
			continue;
		case TOKEN822_COMMENT:
			/*- comment is lexically a space; shouldn't affect wordok */
			break;
		case TOKEN822_COMMA:
			if (tokaddr.len)
				gotaddr();
			wordok = 1;
			break;
		default:
			wordok = 1;
			if (!token822_append(&tokaddr, t))
				die_nomem();
			continue;
		}
	} /*- while (t > beginning) */
	if (tokaddr.len)
		gotaddr();
}

ssize_t
mywrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	qmail_put(&qq, buf, len);
	return len;
}

void
try(fn)
	char           *fn;
{
	int             fd;
	int             match;
	substdio        ss;

	if ((fd = open_read(fn)) == -1)
	{
		if (errno == error_noent)
			return;
		strerr_die4sys(111, FATAL, "unable to open ", fn, ": ");
	}
	if (!stralloc_copys(&targets, ""))
		die_nomem();
	flagacted = 0;
	flagdirect = 0;
	substdio_fdbuf(&ss, read, fd, inbuf, sizeof inbuf);
	for (;;)
	{
		if (getln(&ss, &line, &match, '\n') == -1)
			strerr_die4sys(111, FATAL, "unable to read ", fn, ": ");
		if (!line.len)
			break;
		if (line.s[0] != '#')
			parseline();
		if (!match)
			break;
	}
	close(fd);
	if (targets.len)
	{
		flagacted = 1;
		if (flagdoit)
		{
			if (qmail_open(&qq) == -1)
				strerr_die2sys(111, FATAL, "unable to run qmail-queue: ");
			qp = qmail_qp(&qq);
			qmail_puts(&qq, dtline);
			substdio_fdbuf(&ssmess, read, 0, messbuf, sizeof messbuf);
			if (substdio_copy(&ssqq, &ssmess) != 0)
				die_readmess();
			substdio_flush(&ssqq);
			qmail_from(&qq, sender);
			qmail_put(&qq, targets.s, targets.len);
			qqx = qmail_close(&qq);
			if (*qqx == 'D')
				strerr_die3x(100, FATAL, "unable to forward message: ", qqx + 1);
			if (*qqx)
				strerr_die3x(111, FATAL, "unable to forward message: ", qqx + 1);
			strnum[fmt_ulong(strnum, qp)] = 0;
			strerr_warn3(INFO, "qp ", strnum, 0);
		}
	}
	if (flagdirect)
	{
		if (!flagdoit)
			strerr_warn1("direct delivery", 0);
		_exit(0);
	}
	if (!flagacted)
	{
		if (!flagdoit)
			strerr_warn2("skipping empty file ", fn, 0);
		return;
	}
	_exit(99);
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             opt;

	sig_pipeignore();
	while ((opt = getopt(argc, argv, "nN")) != opteof)
	{
		switch (opt)
		{
		case 'n':
			flagdoit = 0;
			break;
		case 'N':
			flagdoit = 1;
			break;
		default:
			strerr_die1x(100, "dot-forward: usage: dot-forward [ -nN ] file ...");
		}
	}
	argv += optind;
	if (!(ufline = env_get("UFLINE")))
		ufline = "";
	if (!(rpline = env_get("RPLINE")))
		rpline = "";
	if (!(dtline = env_get("DTLINE")))
		dtline = "";
	if (!(sender = env_get("NEWSENDER")))
		sender = "";
	if (!(user = env_get("USER")))
		user = "";
	userlen = str_len(user);
	if (!(host = env_get("HOST")))
		host = "";
	hostlen = str_len(host);
	readcontrols();
	while (*argv)
		try(*argv++);
	if (!flagdoit)
		strerr_warn1("direct delivery", 0);
	_exit(0);
	/*- Not reached */
	return (0);
}

void
getversion_dot_forward_c()
{
	static char    *x = "$Id: dot-forward.c,v 1.7 2019-06-07 11:26:04+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
