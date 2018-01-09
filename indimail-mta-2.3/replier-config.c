/*
 * $Log: replier-config.c,v $
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
#include "strerr.h"
#include "substdio.h"
#include "stralloc.h"
#include "open.h"
#include "exit.h"
#include "auto_ezmlm.h"
#include "auto_qmail.h"
#include "str.h"

#define FATAL "replier-config: fatal: "

void
usage(void)
{
	strerr_die1x(100, "replier-config: usage: replier-config dir dot local host [ outlocal [ outhost ]]");
}

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

char           *dir;
char           *dot;
char           *local;
char           *host;
char           *outlocal;
char           *outhost;
char           *extnum;

char           *fn;
char            buf[1024];
int             fd;
substdio        ss;

void
fail(void)
{
	strerr_die6sys(111, FATAL, "unable to create ", dir, "/", fn, ": ");
}

void
makedir(char *s)
{
	fn = s;
	if (mkdir(fn, 0700) == -1)
		fail();
}

void
start(char *s)
{
	fn = s;
	fd = open_trunc(fn);
	if (fd == -1)
		fail();
	substdio_fdbuf(&ss, write, fd, buf, sizeof buf);
}

void
outs(char *s)
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

stralloc        dirplus = { 0 };
stralloc        dotplus = { 0 };
stralloc        outadmin = { 0 };

void
dirplusmake(slash)
	char           *slash;
{
	if (!stralloc_copys(&dirplus, dir))
		nomem();
	if (!stralloc_cats(&dirplus, slash))
		nomem();
	if (!stralloc_0(&dirplus))
		nomem();
}

void
linkdotdir(dash, slash)
	char           *dash;
	char           *slash;
{
	if (!stralloc_copys(&dotplus, dot))
		nomem();
	if (!stralloc_cats(&dotplus, dash))
		nomem();
	if (!stralloc_0(&dotplus))
		nomem();
	dirplusmake(slash);
	if (symlink(dirplus.s, dotplus.s) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", dotplus.s, ": ");
}

int
main(int argc, char **argv)
{
	umask(077);

	dir = argv[1];
	if (!dir)
		usage();
	if (dir[0] != '/')
		usage();
	dot = argv[2];
	if (!dot)
		usage();
	local = argv[3];
	if (!local)
		usage();
	host = argv[4];
	if (!host)
		usage();
	outlocal = argv[5];
	if (!outlocal)
	{
		outlocal = local;
		outhost = host;
	} else
	{
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
	outs(auto_qmail);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(auto_qmail);
	outs("/bin/822bodyfilter ");
	outs(dir);
	outs("/bodyfilter\n|");
	outs(auto_qmail);
	outs("/bin/bouncesaying 'Cannot filter message'\n");
	finish();
	perm(0600);

	start("qmail-headerfilter");
	outs("|");
	outs(auto_ezmlm);
	outs("/ezmlm-weed\n|");
	outs(auto_qmail);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(auto_qmail);
	outs("/bin/822headerfilter ");
	outs(dir);
	outs("/headerfilter\n|");
	outs(auto_qmail);
	outs("/bin/bouncesaying 'Cannot filter message'\n");
	finish();
	perm(0600);

	start("qmail-msgfilter");
	outs("|");
	outs(auto_ezmlm);
	outs("/ezmlm-weed\n|");
	outs(auto_qmail);
	outs("/bin/replier ");
	outs(dir);
	outs(" \"$SENDER\" ");
	outs(dir);
	outs("/msgfilter\n|");
	outs(auto_qmail);
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
	static char    *x = "$Id: replier-config.c,v 1.3 2004-10-22 20:29:58+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
