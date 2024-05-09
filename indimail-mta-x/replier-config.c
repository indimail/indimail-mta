/*
 * $Log: replier-config.c,v $
 * Revision 1.7  2022-03-05 13:37:29+05:30  Cprogrammer
 * use auto_prefix/bin for binary paths
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2021-06-14 01:08:57+05:30  Cprogrammer
 * removed dependency on auto_qmail.h
 *
 * Revision 1.4  2020-11-24 13:47:56+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.3  2004-10-22 20:29:58+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:38:38+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-07-17 21:00:37+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <strerr.h>
#include <substdio.h>
#include <stralloc.h>
#include <open.h>
#include <str.h>
#include <noreturn.h>
#include "auto_ezmlm.h"
#include "auto_prefix.h"

#define FATAL "replier-config: fatal: "

typedef const char c_char;
static int      fd;
static substdio ss;
static stralloc dirplus = { 0 };
static stralloc dotplus = { 0 };
static c_char  *fn, *dir, *dot, *local, *host, *outlocal, *outhost;
static char     buf[1024];

no_return void
usage(void)
{
	strerr_die1x(100, "replier-config: usage: replier-config dir dot local host [ outlocal [ outhost ]]");
}

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
fail(void)
{
	strerr_die6sys(111, FATAL, "unable to create ", dir, "/", fn, ": ");
}

void
makedir(const char *s)
{
	fn = s;
	if (mkdir(fn, 0700) == -1)
		fail();
}

void
start(const char *s)
{
	fn = s;
	if ((fd = open_trunc(fn)) == -1)
		fail();
	substdio_fdbuf(&ss, write, fd, buf, sizeof buf);
}

void
outs(const char *s)
{
	if (substdio_puts(&ss, s) == -1)
		fail();
}

void
finish(void)
{
	if (substdio_flush(&ss) == -1)
		fail();
	if (fsync(fd) == -1)
		fail();
	close(fd);
}

void
perm(int mode)
{
	if (chmod(fn, mode) == -1)
		fail();
}

void
dirplusmake(const char *slash)
{
	if (!stralloc_copys(&dirplus, dir) ||
			!stralloc_cats(&dirplus, slash) ||
			!stralloc_0(&dirplus))
		nomem();
}

void
linkdotdir(const char *dash, const char *slash)
{
	if (!stralloc_copys(&dotplus, dot) ||
			!stralloc_cats(&dotplus, dash) ||
			!stralloc_0(&dotplus))
		nomem();
	dirplusmake(slash);
	if (symlink(dirplus.s, dotplus.s) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", dotplus.s, ": ");
}

int
main(int argc, char **argv)
{
	umask(077);

	if (!(dir = argv[1]))
		usage();
	if (dir[0] != '/')
		usage();
	if (!(dot = argv[2]))
		usage();
	if (!(local = argv[3]))
		usage();
	if (!(host = argv[4]))
		usage();
	if (!(outlocal = argv[5])) {
		outlocal = local;
		outhost = host;
	} else {
		outhost = argv[6];
		if (!outhost)
			outhost = host;
	}

	if (mkdir(dir, 0700) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", dir, ": ");
	if (chdir(dir) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", dir, ": ");
	if (mkdir("text", 0700) == -1)
		strerr_die2sys(111, FATAL, "unable to create text: ");

	start("inlocal");
	outs(local);
	outs("\n");
	finish();
	perm(0600);

	start("inhost");
	outs(host);
	outs("\n");
	finish();
	perm(0600);

	start("outlocal");
	outs(outlocal);
	if (*outlocal)
		outs("-");
	outs("\n");
	finish();
	perm(0600);

	start("outhost");
	outs(outhost);
	outs("\n");
	finish();
	perm(0600);

	start("mailinglist");
	outs("contact ");
	outs(outlocal);
	if (*outlocal)
		outs("-");
	outs("help@");
	outs(outhost);
	outs("; run by replier\n");
	finish();
	perm(0600);

	start("headerremove");
	outs("return-path\nreturn-receipt-to\ncontent-length\n");
	finish();
	perm(0600);

	start("headeradd");
	outs("Precedence: bulk");
	finish();
	perm(0600);

	start("text/help");
#include "help.t"
	finish();
	perm(0600);

	start("qmail-bodyfilter");
	outs("|");
	outs(auto_ezmlm);
	outs("/ezmlm-weed\n|");
	outs(auto_prefix);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(auto_prefix);
	outs("/bin/822bodyfilter ");
	outs(dir);
	outs("/bodyfilter\n|");
	outs(auto_prefix);
	outs("/bin/bouncesaying 'Cannot filter message'\n");
	finish();
	perm(0600);

	start("qmail-headerfilter");
	outs("|");
	outs(auto_ezmlm);
	outs("/ezmlm-weed\n|");
	outs(auto_prefix);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(auto_prefix);
	outs("/bin/822headerfilter ");
	outs(dir);
	outs("/headerfilter\n|");
	outs(auto_prefix);
	outs("/bin/bouncesaying 'Cannot filter message'\n");
	finish();
	perm(0600);

	start("qmail-msgfilter");
	outs("|");
	outs(auto_ezmlm);
	outs("/ezmlm-weed\n|");
	outs(auto_prefix);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(dir);
	outs("/msgfilter\n|");
	outs(auto_prefix);
	outs("/bin/bouncesaying 'Cannot filter message'\n");
	finish();
	perm(0600);

	start("bouncer");
	outs("#\n");
	finish();
	perm(0600);

	start("bodyfilter");
	outs("#!/bin/sh\ncase \"$REQUEST");
	outs("\" in\n");
	outs("  *) echo \"unknown filter '$REQUEST");
	outs("'\" 1>&2 ;;\n");
	outs("esac\nexit 100\n");
	finish();
	perm(0700);

	start("headerfilter");
	outs("#!/bin/sh\ncase \"$REQUEST");
	outs("\" in\n");
	outs("  *) echo \"unknown filter '$REQUEST");
	outs("'\" 1>&2 ;;\n");
	outs("esac\nexit 100\n");
	finish();
	perm(0700);

	start("msgfilter");
	outs("#!/bin/sh\ncase \"$REQUEST");
	outs("\" in\n");
	outs("  help)\n    echo \"To: $SENDER\"\n    echo \"Subject: replier response\"\n    echo \"\"\n");
	outs("    cat ");
	outs(dir);
	outs("/text/help -\n    exit 0\n   ;;\n");
	outs("  *) \"unknown filter '$REQUEST");
	outs("'\" 1>&2 ;;\n");
	outs("esac\nexit 100\n");
	finish();
	perm(0700);

	linkdotdir("-help", "/qmail-msgfilter");
	linkdotdir("-return-default", "/bouncer");

	_exit(0);
}

void
getversion_replier_config_c()
{
	const char     *x = "$Id: replier-config.c,v 1.7 2022-03-05 13:37:29+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
