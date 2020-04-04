/*
 * $Log: fastforward.c,v $
 * Revision 1.8  2020-04-04 11:17:10+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.7  2019-06-07 11:26:18+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.6  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.5  2010-06-08 21:59:13+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.4  2008-07-15 19:51:02+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.3  2004-10-22 20:25:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:35:04+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-10-21 22:46:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "auto_qmail.h"
#include "auto_control.h"
#include "envdir.h"
#include "pathexec.h"
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "open.h"
#include "cdb.h"
#include "str.h"
#include "byte.h"
#include "slurpclose.h"
#include "strset.h"
#include "exit.h"
#include "strerr.h"
#include "env.h"
#include "sig.h"
#include "qmail.h"
#include "fmt.h"
#include "case.h"
#include "alloc.h"
#include "coe.h"
#include "seek.h"
#include "wait.h"
#include "sgetopt.h"
#include "variables.h"

#define FATAL "fastforward: fatal: "

void
usage()
{
	strerr_die1x(100, "fastforward: usage: fastforward [ -nNpP ] data.cdb");
}

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
print(s)
	char           *s;
{
	char            ch;

	while ((ch = *s++)) {
		substdio_put(subfderr, &ch, 1);
	}
}

void
printsafe(s)
	char           *s;
{
	char            ch;

	while ((ch = *s++)) {
		if (ch < 32)
			ch = '_';
		substdio_put(subfderr, &ch, 1);
	}
}

struct qmail    qq;
char            qp[FMT_ULONG];
char            qqbuf[1];

ssize_t
qqwrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	qmail_put(&qq, buf, len);
	return len;
}

substdio        ssqq = SUBSTDIO_FDBUF(qqwrite, -1, qqbuf, sizeof qqbuf);
char            messbuf[4096];
substdio        ssmess = SUBSTDIO_FDBUF(read, 0, messbuf, sizeof messbuf);
int             flagdeliver = 1;
int             flagpassthrough = 0;
char           *dtline;
stralloc        sender = { 0 };
stralloc        programs = { 0 };
stralloc        forward = { 0 };
strset          done;
stralloc        todo = { 0 };
stralloc        mailinglist = { 0 };

void
dofile(fn)
	char           *fn;
{
	int             fd;
	struct stat     st;
	int             i;
	int             j;

	if (!stralloc_copys(&mailinglist, ""))
		nomem();
	if ((fd = open_read(fn)) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", fn, ": ");
	if (fstat(fd, &st) == -1)
		strerr_die4sys(111, FATAL, "unable to stat ", fn, ": ");
	if ((st.st_mode & 0444) != 0444)
		strerr_die3x(111, FATAL, fn, " is not world-readable");
	if (slurpclose(fd, &mailinglist, 1024) == -1)
		strerr_die4sys(111, FATAL, "unable to read ", fn, ": ");
	i = 0;
	for (j = 0; j < mailinglist.len; ++j) {
		if (!mailinglist.s[j]) {
			if ((mailinglist.s[i] == '.') || (mailinglist.s[i] == '/')) {
				if (!stralloc_cats(&todo, mailinglist.s + i))
					nomem();
				if (!stralloc_0(&todo))
					nomem();
			} else
			if ((mailinglist.s[i] == '&') && (j - i < 900)) {
				if (!stralloc_cats(&todo, mailinglist.s + i))
					nomem();
				if (!stralloc_0(&todo))
					nomem();
			}
			i = j + 1;
		}
	} /*- for (j = 0; j < mailinglist.len; ++j) */
}

char           *fncdb;
int             fdcdb;
stralloc        key = { 0 };
uint32          dlen;
stralloc        data = { 0 };

void
cdbreaderror()
{
	strerr_die4sys(111, FATAL, "unable to read ", fncdb, ": ");
}

int
findtarget(flagwild, prepend, addr)
	int             flagwild;
	char           *prepend;
	char           *addr;
{
	int             r;
	int             at;

	if (!stralloc_copys(&key, prepend))
		nomem();
	if (!stralloc_cats(&key, addr))
		nomem();
	case_lowerb(key.s, key.len);
	if ((r = cdb_seek(fdcdb, key.s, key.len, &dlen)) == -1)
		cdbreaderror();
	if (r)
		return 1;
	if (!flagwild)
		return 0;
	at = str_rchr(addr, '@');
	if (!addr[at])
		return 0;
	if (!stralloc_copys(&key, prepend))
		nomem();
	if (!stralloc_cats(&key, addr + at))
		nomem();
	case_lowerb(key.s, key.len);
	if ((r = cdb_seek(fdcdb, key.s, key.len, &dlen)) == -1)
		cdbreaderror();
	if (r)
		return 1;
	if (!stralloc_copys(&key, prepend))
		nomem();
	if (!stralloc_catb(&key, addr, at + 1))
		nomem();
	case_lowerb(key.s, key.len);
	if ((r = cdb_seek(fdcdb, key.s, key.len, &dlen)) == -1)
		cdbreaderror();
	if (r)
		return 1;
	return 0;
}

int
gettarget(flagwild, prepend, addr)
	int             flagwild;
	char           *prepend;
	char           *addr;
{
	if (!findtarget(flagwild, prepend, addr))
		return 0;
	if (!stralloc_ready(&data, (unsigned int) dlen))
		nomem();
	data.len = dlen;
	if (cdb_bread(fdcdb, data.s, data.len) == -1)
		cdbreaderror();
	return 1;
}

void
doprogram(arg)
	char           *arg;
{
	char           *args[5];
	int             child;
	int             wstat;

	if (!flagdeliver) {
		print("run ");
		printsafe(arg);
		print("\n");
		substdio_flush(subfderr);
		return;
	}
	if (*arg == '!') {
		args[0] = "preline";
		args[1] = "sh";
		args[2] = "-c";
		args[3] = arg + 1;
		args[4] = 0;
	} else {
		args[0] = "sh";
		args[1] = "-c";
		args[2] = arg + 1;
		args[3] = 0;
	}
	switch (child = vfork())
	{
	case -1:
		strerr_die2sys(111, FATAL, "unable to fork: ");
	case 0:
		sig_pipedefault();
		execvp(*args, args);
		strerr_die4sys(111, FATAL, "unable to run ", arg, ": ");
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat))
		strerr_die4sys(111, FATAL, "child crashed in ", arg, ": ");
	switch (wait_exitcode(wstat))
	{
	case 64:
	case 65:
	case 70:
	case 76:
	case 77:
	case 78:
	case 112:
	case 100:
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
dodata()
{
	int             i;
	int             j;

	i = 0;
	for (j = 0; j < data.len; ++j) {
		if (!data.s[j]) {
			if ((data.s[i] == '|') || (data.s[i] == '!'))
				doprogram(data.s + i);
			else
			if ((data.s[i] == '.') || (data.s[i] == '/')) {
				if (!stralloc_cats(&todo, data.s + i))
					nomem();
				if (!stralloc_0(&todo))
					nomem();
			} else
			if ((data.s[i] == '&') && (j - i < 900)) {
				if (!stralloc_cats(&todo, data.s + i))
					nomem();
				if (!stralloc_0(&todo))
					nomem();
			}
			i = j + 1;
		}
	} /*- for (j = 0; j < data.len; ++j) */
}

void
dorecip(addr)
	char           *addr;
{

	if (!findtarget(0, "?", addr) && gettarget(0, ":", addr)) {
		dodata();
		return;
	}
	if (!stralloc_cats(&forward, addr))
		nomem();
	if (!stralloc_0(&forward))
		nomem();
}

void
doorigrecip(addr)
	char           *addr;
{
	if (sender.len) {
		if ((sender.len != 4) || byte_diff(sender.s, 4, "#@[]")) {
			if (gettarget(1, "?", addr) && !stralloc_copy(&sender, &data))
				nomem();
		}
	}
	if (!gettarget(1, ":", addr)) {
		if (flagpassthrough)
			_exit(0);
		else
			strerr_die1x(100, "Sorry, no mailbox here by that name. (#5.1.1)");
	}
	dodata();
}

stralloc        recipient = { 0 };
int             flagdefault = 0;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             opt, i;
	char           *x, *qbase, *home;
	char          **e;

	sig_pipeignore();
	if (!(dtline = env_get("DTLINE")))
		dtline = "";
	if (!(x = env_get("SENDER")))
		x = "original envelope sender";
	if (!stralloc_copys(&sender, x))
		nomem();
	if (!stralloc_copys(&forward, ""))
		nomem();
	if (!strset_init(&done))
		nomem();
	while ((opt = getopt(argc, argv, "nNpPdD")) != opteof) {
		switch (opt)
		{
		case 'n':
			flagdeliver = 0;
			break;
		case 'N':
			flagdeliver = 1;
			break;
		case 'p':
			flagpassthrough = 1;
			break;
		case 'P':
			flagpassthrough = 0;
			break;
		case 'd':
			flagdefault = 1;
			break;
		case 'D':
			flagdefault = 0;
			break;
		default:
			usage();
		}
	} /*- while ((opt = getopt(argc, argv, "nNpPdD")) != opteof) */
	argv += optind;
	if (!(fncdb = *argv))
		usage();
	if ((fdcdb = open_read(fncdb)) == -1)
		cdbreaderror();
	coe(fdcdb);
	if (flagdefault) {
		if (!(x = env_get("DEFAULT")))
			x = env_get("EXT");
		if (!x)
			strerr_die2x(100, FATAL, "$DEFAULT or $EXT must be set");
		if (!stralloc_copys(&recipient, x))
			nomem();
		if (!stralloc_cats(&recipient, "@"))
			nomem();
		if (!(x = env_get("HOST")))
			strerr_die2x(100, FATAL, "$HOST must be set");
		if (!stralloc_cats(&recipient, x))
			nomem();
		if (!stralloc_0(&recipient))
			nomem();
		x = recipient.s;
	} else {
		if (!(x = env_get("RECIPIENT")))
			strerr_die2x(100, FATAL, "$RECIPIENT must be set");
	}
	if (!strset_add(&done, x))
		nomem();
	doorigrecip(x);
	while (todo.len) {
		i = todo.len - 1;
		while ((i > 0) && todo.s[i - 1])
			--i;
		todo.len = i;
		if (strset_in(&done, todo.s + i))
			continue;
		if (!(x = alloc(str_len(todo.s + i) + 1)))
			nomem();
		str_copy(x, todo.s + i);
		if (!strset_add(&done, x))
			nomem();
		x = todo.s + i;
		if (*x == 0)
			continue;
		else
		if ((*x == '.') || (*x == '/'))
			dofile(x);
		else
			dorecip(x + 1);
	}
	if (!forward.len) {
		if (!flagdeliver) {
			print("no forwarding\n");
			substdio_flush(subfderr);
		}
		_exit(flagpassthrough ? 99 : 0);
	}
	if (!stralloc_0(&sender))
		nomem();
	if (!flagdeliver) {
		print("from <");
		printsafe(sender.s);
		print(">\n");
		while (forward.len) {
			i = forward.len - 1;
			while ((i > 0) && forward.s[i - 1])
				--i;
			forward.len = i;
			print("to <");
			printsafe(forward.s + i);
			print(">\n");
		}
		substdio_flush(subfderr);
		_exit(flagpassthrough ? 99 : 0);
	}
	if ((home = env_get("HOME"))) {
		if (chdir(home) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", home, ": ");
		if (!access(".defaultqueue", X_OK)) {
			envdir_set(".defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		} else
			home = (char *) 0;
	}
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");
	if (!(qbase = env_get("QUEUE_BASE"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (chdir(controldir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", controldir, ": ");
		if (!access("defaultqueue", X_OK)) {
			envdir_set("defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		}
		if (chdir(auto_qmail) == -1)
			strerr_die4sys(111, FATAL, "unable to chdir to ", auto_qmail, ": ");
	}
	if (qmail_open(&qq) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	qmail_puts(&qq, dtline);
	if (substdio_copy(&ssqq, &ssmess) != 0)
		strerr_die2sys(111, FATAL, "unable to read message: ");
	substdio_flush(&ssqq);
	qp[fmt_ulong(qp, qmail_qp(&qq))] = 0;
	qmail_from(&qq, sender.s);
	while (forward.len) {
		i = forward.len - 1;
		while ((i > 0) && forward.s[i - 1])
			--i;
		forward.len = i;
		qmail_to(&qq, forward.s + i);
	}
	x = qmail_close(&qq);
	if (*x)
		strerr_die2x(*x == 'D' ? 100 : 111, FATAL, x + 1);
	strerr_die2x(flagpassthrough ? 99 : 0, "fastforward: qp ", qp);
	/*- Not reached */
	return(0);
}

void
getversion_fastforward_c()
{
	static char    *x = "$Id: fastforward.c,v 1.8 2020-04-04 11:17:10+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
