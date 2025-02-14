/*
 * $Id: inewaliases.c,v 1.9 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/stat.h>
#include <substdio.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <open.h>
#include <token822.h>
#include <byte.h>
#include <case.h>
#include <cdbmss.h>
#include <noreturn.h>
#include "control.h"
#include "variables.h"

#define FATAL "newaliases: fatal: "

int             rename(const char *, const char *);

static stralloc me = { 0 };
static stralloc defaulthost = { 0 };
static stralloc defaultdomain = { 0 };
static stralloc plusdomain = { 0 };
static stralloc target = { 0 };
static stralloc fulltarget = { 0 };
static stralloc instr = { 0 };
static stralloc cbuf = { 0 };
static stralloc address = { 0 };
static stralloc line = { 0 };
static stralloc newline = { 0 };
static stralloc key = { 0 };
static token822_alloc toks = { 0 };
static token822_alloc tokaddr = { 0 };
static int      match;
static char     inbuf[1024];
static substdio ssin;
static struct cdbmss cdbmss;


no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
nulbyte()
{
	strerr_die2x(100, FATAL, "NUL bytes are not permitted");
}

no_return void
longaddress()
{
	strerr_die2x(100, FATAL, "addresses over 800 bytes are not permitted");
}

no_return void
writeerr()
{
	strerr_die2sys(111, FATAL, "unable to write to /etc/aliases.tmp: ");
}

no_return void
readerr()
{
	strerr_die2sys(111, FATAL, "unable to read /etc/aliases: ");
}

no_return void
die_control()
{
	strerr_die2sys(111, FATAL, "unable to read controls: ");
}

void
readcontrols()
{
	int             r;

	if ((r = control_readline(&me, "me")) == -1)
		die_control();
	if (!r && !stralloc_copys(&me, "me"))
		nomem();
	if ((r = control_readline(&defaultdomain, "defaultdomain")) == -1)
		die_control();
	if (!r && !stralloc_copy(&defaultdomain, &me))
		nomem();
	if ((r = control_readline(&defaulthost, "defaulthost")) == -1)
		die_control();
	if (!r && !stralloc_copy(&defaulthost, &me))
		nomem();
	if ((r = control_readline(&plusdomain, "plusdomain")) == -1)
		die_control();
	if (!r && !stralloc_copy(&plusdomain, &me))
		nomem();
}

void
gotincl()
{
	token822_reverse(&tokaddr);
	if (token822_unquote(&address, &tokaddr) != 1)
		nomem();
	tokaddr.len = 0;
	if (!address.len)
		strerr_die2x(111, FATAL, "empty :include: filenames not permitted");
	if (byte_chr(address.s, address.len, '\0') < address.len)
		strerr_die2x(111, FATAL, "NUL not permitted in :include: filenames");
	if (address.s[0] != '.' && address.s[0] != '/' && !stralloc_cats(&instr, "./"))
		nomem();
	if (!stralloc_cat(&instr, &address) ||
			!stralloc_cats(&instr, ".bin") ||
			!stralloc_0(&instr))
		nomem();
}

void
gotaddr()
{
	int             i;
	int             j;
	int             flaghasat;

	token822_reverse(&tokaddr);
	if (token822_unquote(&address, &tokaddr) != 1)
		nomem();
	if (!address.len)
		strerr_die2x(111, FATAL, "empty recipient addresses not permitted");
	flaghasat = 0;
	for (i = 0; i < tokaddr.len; ++i) {
		if (tokaddr.t[i].type == TOKEN822_AT)
			flaghasat = 1;
	}
	tokaddr.len = 0;
	if (!address.len)
		return;
	if (!flaghasat && address.s[0] == '/') {
		if (!stralloc_0(&address))
			nomem();
		strerr_die4x(111, FATAL, "file delivery ", address.s, " not supported");
	}
	if (!flaghasat && address.s[0] == '|') {
		if (byte_chr(address.s, address.len, '\0') < address.len)
			strerr_die2x(111, FATAL, "NUL not permitted in program names");
		if (!stralloc_cats(&instr, "!") ||
				!stralloc_catb(&instr, address.s + 1, address.len - 1) ||
				!stralloc_0(&instr))
			nomem();
		return;
	}
	if (target.len) {
		if (!stralloc_cats(&instr, "&") ||
				!stralloc_cat(&instr, &fulltarget) ||
				!stralloc_0(&instr))
			nomem();
	}
	if ((!flaghasat && !stralloc_cats(&address, "@")) ||
			!stralloc_copy(&target, &address) ||
			!stralloc_copy(&fulltarget, &address) ||
			(fulltarget.s[fulltarget.len - 1] == '@' && !stralloc_cat(&fulltarget, &defaulthost)))
		nomem();
	if (fulltarget.s[fulltarget.len - 1] == '+') {
		fulltarget.s[fulltarget.len - 1] = '.';
		if (!stralloc_cat(&fulltarget, &plusdomain))
			nomem();
	}
	j = 0;
	for (i = 0; i < fulltarget.len; ++i) {
		if (fulltarget.s[i] == '@')
			j = i;
	}
	for (i = j; i < fulltarget.len; ++i) {
		if (fulltarget.s[i] == '.')
			break;
	}
	if (i == fulltarget.len) {
		if (!stralloc_cats(&fulltarget, ".") ||
				!stralloc_cat(&fulltarget, &defaultdomain))
			nomem();
	}
	if (fulltarget.len > 800)
		longaddress();
	if (byte_chr(fulltarget.s, fulltarget.len, '\0') < fulltarget.len)
		strerr_die2x(111, FATAL, "NUL not permitted in recipient addresses");
}

no_return void
parseerr()
{
	if (!stralloc_0(&line))
		nomem();
	strerr_die3x(111, FATAL, "unable to parse this line: ", line.s);
}

void
parseline()
{
	int             wordok;
	struct token822 *t;
	struct token822 *beginning;

	switch (token822_parse(&toks, &line, &cbuf))
	{
	case -1:
		nomem();
	case 0:
		parseerr();
	}
	beginning = toks.t;
	t = toks.t + toks.len;
	wordok = 1;
	if (!token822_readyplus(&tokaddr, 1))
		nomem();
	tokaddr.len = 0;
	while (t > beginning) {
		switch ((--t)->type)
		{
		case TOKEN822_SEMI:
			/*XXX*/
			break;
		case TOKEN822_COLON:
			if (t >= beginning + 2) {
				if (t[-2].type == TOKEN822_COLON && t[-1].type == TOKEN822_ATOM) {
					if (t[-1].slen == 7 && !byte_diff(t[-1].s, 7, "include")) {
						gotincl();
						t -= 2;
					}
				}
			}
			break;
			/*XXX*/
		case TOKEN822_RIGHT:
			if (tokaddr.len)
				gotaddr();
			while ((t > beginning) && (t[-1].type != TOKEN822_LEFT)) {
				if (!token822_append(&tokaddr, --t))
					nomem();
			}
			gotaddr();
			if (t <= beginning)
				parseerr();
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
				nomem();
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
				nomem();
			continue;
		}
	} /*- while (t > beginning) */
	if (tokaddr.len)
		gotaddr();
}

void
doit()
{
	if (!instr.len) {
		if (target.len)
			parseerr();
		return;
	}
	if (!target.len)
		parseerr();
	if (stralloc_starts(&target, "owner-")) {
		if (!stralloc_copys(&key, "?") ||
				!stralloc_catb(&key, target.s + 6, target.len - 6))
			nomem();
		case_lowerb(key.s, key.len);
		if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) fulltarget.s, fulltarget.len) == -1)
			writeerr();
	}
	if (!stralloc_copys(&key, ":") ||
			!stralloc_cat(&key, &target))
		nomem();
	case_lowerb(key.s, key.len);
	if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) instr.s, instr.len) == -1)
		writeerr();
}

int
main()
{
	int             fd;

	umask(033);
	readcontrols();
	if ((fd = open_read("/etc/aliases")) == -1)
		readerr();
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof inbuf);
	if ((fd = open_trunc("/etc/aliases.tmp")) == -1)
		strerr_die2sys(111, FATAL, "unable to create /etc/aliases.tmp: ");
	if (cdbmss_start(&cdbmss, fd) == -1)
		writeerr();
	if (!stralloc_copys(&line, ""))
		nomem();
	for (;;) {
		if (getln(&ssin, &newline, &match, '\n') != 0)
			readerr();
		if (match && (newline.s[0] == '\n'))
			continue;
		if (match && ((newline.s[0] == ' ') || (newline.s[0] == '\t'))) {
			if (!stralloc_cat(&line, &newline))
				nomem();
			continue;
		}
		if (line.len && line.s[0] != '#') {
			if (!stralloc_copys(&target, "") ||
					!stralloc_copys(&fulltarget, "") ||
					!stralloc_copys(&instr, ""))
				nomem();
			parseline();
			doit();
		}
		if (!match)
			break;
		if (!stralloc_copy(&line, &newline))
			nomem();
	}
	if (cdbmss_finish(&cdbmss) == -1)
		writeerr();
	if (fsync(fd) == -1)
		writeerr();
	if (close(fd) == -1)
		writeerr();				/*- NFS stupidity */
	if (rename("/etc/aliases.tmp", "/etc/aliases.cdb") == -1)
		strerr_die2sys(111, FATAL, "unable to move /etc/aliases.tmp to /etc/aliases.cdb: ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_newaliases_c()
{
	const char     *x = "$Id: inewaliases.c,v 1.9 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: inewaliases.c,v $
 * Revision 1.9  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.6  2021-06-15 11:38:13+05:30  Cprogrammer
 * moved token822 to libqmail
 *
 * Revision 1.5  2021-06-14 00:44:02+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.4  2005-08-23 17:33:17+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 20:27:41+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:36:04+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-10-21 22:46:48+05:30  Cprogrammer
 * Initial revision
 *
 */
