/*
 * $Id: fastforward.c,v 1.15 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <envdir.h>
#include <pathexec.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <open.h>
#include <cdb.h>
#include <str.h>
#include <byte.h>
#include <strerr.h>
#include <env.h>
#include <sig.h>
#include <fmt.h>
#include <case.h>
#include <alloc.h>
#include <coe.h>
#include <seek.h>
#include <wait.h>
#include <sgetopt.h>
#include <noreturn.h>
#include "qmail.h"
#include "strset.h"
#include "slurpclose.h"
#include "set_environment.h"
#include "buffer_defs.h"

#define FATAL "fastforward: fatal: "
#define WARN  "fastforward: warn: "

ssize_t         qqwrite(int, char *, int);

struct qmail    qq;
char            qp[FMT_ULONG], qqbuf[1], messbuf[BUFSIZE_REMOTE];
substdio        ssqq = SUBSTDIO_FDBUF(qqwrite, -1, qqbuf, sizeof qqbuf);
substdio        ssmess = SUBSTDIO_FDBUF(read, 0, messbuf, sizeof messbuf);
int             flagdeliver = 1, flagpassthrough = 0, fdcdb, flagdefault = 0;
const char     *dtline, *fncdb;
stralloc        sender, programs, forward, todo, mailinglist, key, data, recipient;
strset          done;
uint32          dlen;

no_return void
usage()
{
	strerr_die1x(100, "fastforward: usage: fastforward [ -nNpP ] data.cdb");
}

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
print(const char *s)
{
	char            ch;

	while ((ch = *s++))
		substdio_put(subfderr, &ch, 1);
}

void
printsafe(const char *s)
{
	char            ch;

	while ((ch = *s++)) {
		if (ch < 32)
			ch = '_';
		substdio_put(subfderr, &ch, 1);
	}
}

ssize_t
qqwrite(int fd, char *buf, int len)
{
	qmail_put(&qq, buf, len);
	return len;
}

void
dofile(const char *fn)
{
	int             fd, i, j;
	struct stat     st;

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
	for (i = j = 0; j < mailinglist.len; ++j) {
		if (!mailinglist.s[j]) {
			if ((mailinglist.s[i] == '.') || (mailinglist.s[i] == '/')) {
				if (!stralloc_cats(&todo, mailinglist.s + i) ||
						!stralloc_0(&todo))
					nomem();
			} else
			if ((mailinglist.s[i] == '&') && (j - i < 900)) {
				if (!stralloc_cats(&todo, mailinglist.s + i) ||
						!stralloc_0(&todo))
					nomem();
			}
			i = j + 1;
		}
	} /*- for (i = j = 0; j < mailinglist.len; ++j) */
}

no_return void
cdbreaderror()
{
	strerr_die4sys(111, FATAL, "unable to read ", fncdb, ": ");
}

int
findtarget(int flagwild, const char *prepend, const char *addr)
{
	int             r, at;

	if (!stralloc_copys(&key, prepend) ||
			!stralloc_cats(&key, addr))
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
	if (!stralloc_copys(&key, prepend) ||
			!stralloc_cats(&key, addr + at))
		nomem();
	case_lowerb(key.s, key.len);
	if ((r = cdb_seek(fdcdb, key.s, key.len, &dlen)) == -1)
		cdbreaderror();
	if (r)
		return 1;
	if (!stralloc_copys(&key, prepend) ||
			!stralloc_catb(&key, addr, at + 1))
		nomem();
	case_lowerb(key.s, key.len);
	if ((r = cdb_seek(fdcdb, key.s, key.len, &dlen)) == -1)
		cdbreaderror();
	if (r)
		return 1;
	return 0;
}

int
gettarget(int flagwild, const char *prepend, const char *addr)
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
doprogram(const char *arg)
{
	char           *args[5];
	int             child, wstat;

	if (!flagdeliver) {
		print("run ");
		printsafe(arg);
		print("\n");
		substdio_flush(subfderr);
		return;
	}
	if (*arg == '!') {
		args[0] = (char *) "preline";
		args[1] = (char *) "sh";
		args[2] = (char *) "-c";
		args[3] = (char *) (arg + 1);
		args[4] = 0;
	} else {
		args[0] = (char *) "sh";
		args[1] = (char *) "-c";
		args[2] = (char *) (arg + 1);
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
	int             i, j;

	i = 0;
	for (j = 0; j < data.len; ++j) {
		if (!data.s[j]) {
			if ((data.s[i] == '|') || (data.s[i] == '!'))
				doprogram(data.s + i);
			else
			if ((data.s[i] == '.') || (data.s[i] == '/')) {
				if (!stralloc_cats(&todo, data.s + i) ||
						!stralloc_0(&todo))
					nomem();
			} else
			if ((data.s[i] == '&') && (j - i < 900)) {
				if (!stralloc_cats(&todo, data.s + i) ||
						!stralloc_0(&todo))
					nomem();
			}
			i = j + 1;
		}
	} /*- for (j = 0; j < data.len; ++j) */
}

void
dorecip(const char *addr)
{

	if (!findtarget(0, "?", addr) && gettarget(0, ":", addr)) {
		dodata();
		return;
	}
	if (!stralloc_cats(&forward, addr) ||
			!stralloc_0(&forward))
		nomem();
}

void
doorigrecip(const char *addr)
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

int
main(int argc, char **argv)
{
	int             opt, i;
	char           *x;
	const char     *qqx;

	sig_pipeignore();
	if (!(dtline = env_get("DTLINE")))
		dtline = "";
	if (!(x = env_get("SENDER")))
		x = (char *) "original envelope sender";
	if (!stralloc_copys(&sender, x) ||
			!stralloc_copys(&forward, "") ||
			!strset_init(&done))
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
		if (!stralloc_copys(&recipient, x) ||
				!stralloc_cats(&recipient, "@"))
			nomem();
		if (!(x = env_get("HOST")))
			strerr_die2x(100, FATAL, "$HOST must be set");
		if (!stralloc_cats(&recipient, x) ||
				!stralloc_0(&recipient))
			nomem();
		x = recipient.s;
	} else
	if (!(x = env_get("RECIPIENT")))
		strerr_die2x(100, FATAL, "$RECIPIENT must be set");
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
	set_environment(WARN, FATAL, 0);
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
	qqx = qmail_close(&qq);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(flagpassthrough ? 99 : 0, "fastforward: qp ", qp);
	/*- Not reached */
	return(0);
}

void
getversion_fastforward_c()
{
	const char     *x = "$Id: fastforward.c,v 1.15 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: fastforward.c,v $
 * Revision 1.15  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.14  2024-01-23 01:26:53+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.13  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.12  2021-07-05 21:10:17+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.11  2021-05-13 14:43:09+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.10  2020-11-24 13:45:14+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.9  2020-04-04 11:41:41+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
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
