/*
 * $Log: qmail-pw2u.c,v $
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.9  2020-05-11 11:09:34+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.8  2016-05-18 15:31:28+05:30  Cprogrammer
 * use auto_assign dir for files include, exclude, mailnames, subusers, append
 *
 * Revision 1.7  2004-10-22 20:28:39+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:37:18+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.5  2004-07-15 23:32:53+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.4  2003-10-23 01:24:39+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.3  2003-10-01 19:05:27+05:30  Cprogrammer
 * changed return type to int
 *
 * Revision 1.2  2003-10-01 01:06:10+05:30  Cprogrammer
 * corrected path i.e. control/../users
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <substdio.h>
#include <byte.h>
#include <subfd.h>
#include <sgetopt.h>
#include <constmap.h>
#include <stralloc.h>
#include <fmt.h>
#include <str.h>
#include <scan.h>
#include <open.h>
#include <error.h>
#include <getln.h>
#include <noreturn.h>
#include "control.h"
#include "auto_break.h"
#include "auto_assign.h"
#include "auto_usera.h"

static char    *dashcolon = "-:";
static int      flagalias = 0;
static int      flagnoupper = 1;
static int      okincl;
static int      okexcl;
static int      okmana;
/*- 2: skip if home does not exist; skip if home is not owned by user */
/*- 1: stop if home does not exist; skip if home is not owned by user */
/*- 0: don't worry about home */
static int      homestrategy = 2;
static stralloc incl = { 0 };
static stralloc excl = { 0 };
static stralloc mana = { 0 };
static stralloc allusers = { 0 };
static stralloc uugh = { 0 };
static stralloc user = { 0 };
static stralloc uidstr = { 0 };
static stralloc gidstr = { 0 };
static stralloc home = { 0 };
static stralloc line = { 0 };
static struct constmap mapincl;
static struct constmap mapexcl;
static struct constmap mapmana;
static struct constmap mapuser;
static unsigned long   uid;

no_return void
die_chdir()
{
	substdio_putsflush(subfderr, "qmail-pw2u: fatal: unable to chdir\n");
	_exit(111);
}

no_return void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-pw2u: fatal: out of memory\n");
	_exit(111);
}

no_return void
die_read()
{
	substdio_putsflush(subfderr, "qmail-pw2u: fatal: unable to read input\n");
	_exit(111);
}

no_return void
die_write()
{
	substdio_putsflush(subfderr, "qmail-pw2u: fatal: unable to write output\n");
	_exit(111);
}

no_return void
die_control()
{
	substdio_putsflush(subfderr, "qmail-pw2u: fatal: unable to read controls\n");
	_exit(111);
}

no_return void
die_alias()
{
	substdio_puts(subfderr, "qmail-pw2u: fatal: unable to find ");
	substdio_puts(subfderr, auto_usera);
	substdio_puts(subfderr, " user\n");
	substdio_flush(subfderr);
	_exit(111);
}

no_return void
die_home(char *fn)
{
	substdio_puts(subfderr, "qmail-pw2u: fatal: unable to stat ");
	substdio_puts(subfderr, fn);
	substdio_puts(subfderr, "\n");
	substdio_flush(subfderr);
	_exit(111);
}

no_return void
die_user(char *s, unsigned int len)
{
	substdio_puts(subfderr, "qmail-pw2u: fatal: unable to find ");
	substdio_put(subfderr, s, len);
	substdio_puts(subfderr, " user for subuser\n");
	substdio_flush(subfderr);
	_exit(111);
}

void
doaccount()
{
	struct stat     st;
	int             i;
	char           *mailnames;
	char           *x;
	unsigned int    xlen;

	if (byte_chr(line.s, line.len, '\0') < line.len)
		return;
	x = line.s;
	xlen = line.len;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	if (!stralloc_copyb(&user, x, i))
		die_nomem();
	if (!stralloc_0(&user))
		die_nomem();
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	if (!stralloc_copyb(&uidstr, x, i))
		die_nomem();
	if (!stralloc_0(&uidstr))
		die_nomem();
	scan_ulong(uidstr.s, &uid);
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	if (!stralloc_copyb(&gidstr, x, i))
		die_nomem();
	if (!stralloc_0(&gidstr))
		die_nomem();
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	if (!stralloc_copyb(&home, x, i))
		die_nomem();
	if (!stralloc_0(&home))
		die_nomem();
	if (!uid)
		return;
	if (flagnoupper)
	{
		for (i = 0; i < user.len; ++i)
			if ((user.s[i] >= 'A') && (user.s[i] <= 'Z'))
				return;
	}
	if (okincl)
	{
		if (!constmap(&mapincl, user.s, user.len - 1))
			return;
	}
	if (okexcl)
	{
		if (constmap(&mapexcl, user.s, user.len - 1))
			return;
	}
	if (homestrategy)
	{
		if (stat(home.s, &st) == -1)
		{
			if (errno != error_noent)
				die_home(home.s);
			if (homestrategy == 1)
				die_home(home.s);
			return;
		}
		if (st.st_uid != uid)
			return;
	}
	if (!stralloc_copys(&uugh, ":"))
		die_nomem();
	if (!stralloc_cats(&uugh, user.s))
		die_nomem();
	if (!stralloc_cats(&uugh, ":"))
		die_nomem();
	if (!stralloc_cats(&uugh, uidstr.s))
		die_nomem();
	if (!stralloc_cats(&uugh, ":"))
		die_nomem();
	if (!stralloc_cats(&uugh, gidstr.s))
		die_nomem();
	if (!stralloc_cats(&uugh, ":"))
		die_nomem();
	if (!stralloc_cats(&uugh, home.s))
		die_nomem();
	if (!stralloc_cats(&uugh, ":"))
		die_nomem();

	/*
	 * XXX: avoid recording in allusers unless sub actually needs it 
	 */
	if (!stralloc_cats(&allusers, user.s))
		die_nomem();
	if (!stralloc_cats(&allusers, ":"))
		die_nomem();
	if (!stralloc_catb(&allusers, uugh.s, uugh.len))
		die_nomem();
	if (!stralloc_0(&allusers))
		die_nomem();

	if (str_equal(user.s, auto_usera))
	{
		if (substdio_puts(subfdout, "+") == -1)
			die_write();
		if (substdio_put(subfdout, uugh.s, uugh.len) == -1)
			die_write();
		if (substdio_puts(subfdout, dashcolon) == -1)
			die_write();
		if (substdio_puts(subfdout, ":\n") == -1)
			die_write();
		flagalias = 1;
	}

	mailnames = 0;
	if (okmana)
		mailnames = constmap(&mapmana, user.s, user.len - 1);
	if (!mailnames)
		mailnames = user.s;

	for (;;)
	{
		while (*mailnames == ':')
			++mailnames;
		if (!*mailnames)
			break;

		i = str_chr(mailnames, ':');

		if (substdio_puts(subfdout, "=") == -1)
			die_write();
		if (substdio_put(subfdout, mailnames, i) == -1)
			die_write();
		if (substdio_put(subfdout, uugh.s, uugh.len) == -1)
			die_write();
		if (substdio_puts(subfdout, "::\n") == -1)
			die_write();

		if (*auto_break)
		{
			if (substdio_puts(subfdout, "+") == -1)
				die_write();
			if (substdio_put(subfdout, mailnames, i) == -1)
				die_write();
			if (substdio_put(subfdout, auto_break, 1) == -1)
				die_write();
			if (substdio_put(subfdout, uugh.s, uugh.len) == -1)
				die_write();
			if (substdio_puts(subfdout, dashcolon) == -1)
				die_write();
			if (substdio_puts(subfdout, ":\n") == -1)
				die_write();
		}
		mailnames += i;
	}
}

stralloc        sub = { 0 };

void
dosubuser()
{
	int             i;
	char           *x;
	unsigned int    xlen;
	char           *u;

	x = line.s;
	xlen = line.len;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	if (!stralloc_copyb(&sub, x, i))
		die_nomem();
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;
	u = constmap(&mapuser, x, i);
	if (!u)
		die_user(x, i);
	++i;
	x += i;
	xlen -= i;
	i = byte_chr(x, xlen, ':');
	if (i == xlen)
		return;

	if (substdio_puts(subfdout, "=") == -1)
		die_write();
	if (substdio_put(subfdout, sub.s, sub.len) == -1)
		die_write();
	if (substdio_puts(subfdout, u) == -1)
		die_write();
	if (substdio_puts(subfdout, dashcolon) == -1)
		die_write();
	if (substdio_put(subfdout, x, i) == -1)
		die_write();
	if (substdio_puts(subfdout, ":\n") == -1)
		die_write();

	if (*auto_break)
	{
		if (substdio_puts(subfdout, "+") == -1)
			die_write();
		if (substdio_put(subfdout, sub.s, sub.len) == -1)
			die_write();
		if (substdio_put(subfdout, auto_break, 1) == -1)
			die_write();
		if (substdio_puts(subfdout, u) == -1)
			die_write();
		if (substdio_puts(subfdout, dashcolon) == -1)
			die_write();
		if (substdio_put(subfdout, x, i) == -1)
			die_write();
		if (substdio_puts(subfdout, "-:\n") == -1)
			die_write();
	}
}

int             fd;
substdio        ss;
char            ssbuf[SUBSTDIO_INSIZE];

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             opt;
	int             match;

	while ((opt = getopt(argc, argv, "/ohHuUc:C")) != opteof)
		switch (opt)
		{
		case '/':
			dashcolon = "-/:";
			break;
		case 'o':
			homestrategy = 2;
			break;
		case 'h':
			homestrategy = 1;
			break;
		case 'H':
			homestrategy = 0;
			break;
		case 'u':
			flagnoupper = 0;
			break;
		case 'U':
			flagnoupper = 1;
			break;
		case 'c':
			*auto_break = *optarg;
			break;
		case 'C':
			*auto_break = 0;
			break;
		case '?':
		default:
			_exit(100);
		}

	if (chdir(auto_assign) == -1)
		die_chdir();

	/*
	 * no need for control_init() 
	 * use ./ in path to avoid use control directory
	 */

	if ((okincl = control_readfile(&incl, "./include", 0)) == -1)
		die_control();
	if (okincl && !constmap_init(&mapincl, incl.s, incl.len, 0))
		die_nomem();

	if ((okexcl = control_readfile(&excl, "./exclude", 0)) == -1)
		die_control();
	if (okexcl && !constmap_init(&mapexcl, excl.s, excl.len, 0))
		die_nomem();

	if ((okmana = control_readfile(&mana, "./mailnames", 0)) == -1)
		die_control();
	if (okmana && !constmap_init(&mapmana, mana.s, mana.len, 1))
		die_nomem();

	if (!stralloc_copys(&allusers, ""))
		die_nomem();
	for (;;) {
		if (getln(subfdin, &line, &match, '\n') == -1)
			die_read();
		doaccount();
		if (!match)
			break;
	}
	if (!flagalias)
		die_alias();

	if ((fd = open_read("subusers")) == -1) {
		if (errno != error_noent)
			die_control();
	} else {
		substdio_fdbuf(&ss, read, fd, ssbuf, sizeof(ssbuf));

		if (!constmap_init(&mapuser, allusers.s, allusers.len, 1))
			die_nomem();

		for (;;) {
			if (getln(&ss, &line, &match, '\n') == -1)
				die_read();
			dosubuser();
			if (!match)
				break;
		}

		close(fd);
	}

	if ((fd = open_read("append")) == -1) {
		if (errno != error_noent)
			die_control();
	} else {
		substdio_fdbuf(&ss, read, fd, ssbuf, sizeof(ssbuf));
		for (;;) {
			if (getln(&ss, &line, &match, '\n') == -1)
				die_read();
			if (substdio_put(subfdout, line.s, line.len) == -1)
				die_write();
			if (!match)
				break;
		}
	}
	if (substdio_puts(subfdout, ".\n") == -1)
		die_write();
	if (substdio_flush(subfdout) == -1)
		die_write();
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_pw2u_c()
{
	static char    *x = "$Id: qmail-pw2u.c,v 1.10 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
