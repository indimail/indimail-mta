/*
 * $Log: setforward.c,v $
 * Revision 1.4  2005-08-23 17:35:51+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.3  2004-10-22 20:30:15+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:39:10+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-10-21 22:47:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"
#include "strerr.h"
#include "stralloc.h"
#include "open.h"
#include "case.h"
#include "cdbmss.h"

#define FATAL "setforward: fatal: "

int             rename(const char *, const char *);

void
usage()
{
	strerr_die1x(100, "setforward: usage: setforward data.cdb data.tmp");
}

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
missingsemicolon()
{
	strerr_die2x(100, FATAL, "final instruction must end with semicolon");
}

void
extracolon()
{
	strerr_die2x(100, FATAL, "double colons are not permitted");
}

void
extracomma()
{
	strerr_die2x(100, FATAL, "commas are not permitted before colons");
}

void
nulbyte()
{
	strerr_die2x(100, FATAL, "NUL bytes are not permitted");
}

void
longaddress()
{
	strerr_die2x(100, FATAL, "addresses over 800 bytes are not permitted");
}

char           *fncdb;
char           *fntmp;
int             fd;
struct cdbmss   cdbmss;
stralloc        key = { 0 };
stralloc        target = { 0 };		/*- always initialized; no NUL */
stralloc        command = { 0 };	/*- always initialized; no NUL */
stralloc        instr = { 0 };		/*- always initialized */
int             flagtarget = 0;

/*
 * 0: reading target; command is empty; instr is empty 
 * 1: target is complete; instr still has to be written; reading command 
 */

void
writeerr()
{
	strerr_die4sys(111, FATAL, "unable to write to ", fntmp, ": ");
}

void
doit(prepend, data, datalen)
	char           *prepend;
	char           *data;
	int             datalen;
{
	if (!stralloc_copys(&key, prepend))
		nomem();
	if (!stralloc_cat(&key, &target))
		nomem();
	case_lowerb(key.s, key.len);
	if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) data, datalen) == -1)
		writeerr();
}

int
getch(ch)
	char           *ch;
{
	int             r;

	if ((r = substdio_get(subfdinsmall, ch, 1)) == -1)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	return r;
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char            ch;

	if (!stralloc_copys(&target, ""))
		nomem();
	if (!stralloc_copys(&command, ""))
		nomem();
	if (!stralloc_copys(&instr, ""))
		nomem();
	if (!(fncdb = argv[1]))
		usage();
	if (!(fntmp = argv[2]))
		usage();
	if ((fd = open_trunc(fntmp)) == -1)
		strerr_die4sys(111, FATAL, "unable to create ", fntmp, ": ");
	if (cdbmss_start(&cdbmss, fd) == -1)
		writeerr();
	for (;;)
	{
		if (!getch(&ch))
			goto eof;
		if (ch == '#')
		{
			while (ch != '\n')
				if (!getch(&ch))
					goto eof;
			continue;
		}
		if (ch == ' ' || ch == '\n' || ch == '\t')
			continue;
		if (ch == ':')
		{
			if (flagtarget)
				extracolon();
			flagtarget = 1;
			continue;
		}
		if ((ch == ',') || (ch == ';'))
		{
			if (!flagtarget)
				extracomma();
			if (command.len)
			{
				if (command.s[0] == '?')
					doit("?", command.s + 1, command.len - 1);
				else
				if ((command.s[0] == '|') || (command.s[0] == '!'))
				{
					if (!stralloc_cat(&instr, &command))
						nomem();
					if (!stralloc_0(&instr))
						nomem();
				} else
				if ((command.s[0] == '.') || (command.s[0] == '/'))
				{
					if (!stralloc_cat(&instr, &command))
						nomem();
					if (!stralloc_0(&instr))
						nomem();
				} else
				{
					if (command.len > 800)
						longaddress();
					if (command.s[0] != '&' && !stralloc_cats(&instr, "&"))
						nomem();
					if (!stralloc_cat(&instr, &command))
						nomem();
					if (!stralloc_0(&instr))
						nomem();
				}
			}
			if (!stralloc_copys(&command, ""))
				nomem();
			if (ch == ';')
			{
				if (instr.len)
					doit(":", instr.s, instr.len);
				if (!stralloc_copys(&target, ""))
					nomem();
				if (!stralloc_copys(&instr, ""))
					nomem();
				flagtarget = 0;
			}
			continue;
		}
		if (ch == '\\' && !getch(&ch))
			goto eof;
		if (ch == 0)
			nulbyte();
		if (!stralloc_append(flagtarget ? &command : &target, &ch))
			nomem();
	}
eof:
	if (flagtarget || target.len)
		missingsemicolon();
	if (cdbmss_finish(&cdbmss) == -1)
		writeerr();
	if (fsync(fd) == -1)
		writeerr();
	if (close(fd) == -1)
		writeerr();				/*- NFS stupidity */
	if (rename(fntmp, fncdb) == -1)
		strerr_die6sys(111, FATAL, "unable to move ", fntmp, " to ", fncdb, ": ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_setforward_c()
{
	static char    *x = "$Id: setforward.c,v 1.4 2005-08-23 17:35:51+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
