/*
 * $Log: qmail-newu.c,v $
 * Revision 1.9  2016-05-18 15:20:59+05:30  Cprogrammer
 * use env variable ASSIGNDIR or auto_assign for users/cdb and user/assign file
 *
 * Revision 1.8  2008-06-20 16:00:34+05:30  Cprogrammer
 * added argument to process different directory for assign file
 *
 * Revision 1.7  2005-08-23 17:35:01+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.6  2004-10-22 20:28:34+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:37:04+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-07-17 21:20:57+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "stralloc.h"
#include "subfd.h"
#include "getln.h"
#include "substdio.h"
#include "cdbmss.h"
#include "exit.h"
#include "byte.h"
#include "env.h"
#include "open.h"
#include "error.h"
#include "case.h"
#include "auto_assign.h"

int             rename(const char *, const char *);

void
die_temp()
{
	_exit(111);
}

void
die_chdir(char *home)
{
	substdio_puts(subfderr, "qmail-newu: fatal: unable to chdir to ");
	substdio_puts(subfderr, home);
	substdio_putsflush(subfderr, "\n");
	die_temp();
}

void
die_nomem()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: out of memory\n");
	die_temp();
}

void
die_opena()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to open users/assign\n");
	die_temp();
}

void
die_reada()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to read users/assign\n");
	die_temp();
}

void
die_format()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: bad format in users/assign\n");
	die_temp();
}

void
die_opent()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to open users/cdb.tmp\n");
	die_temp();
}

void
die_writet()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to write users/cdb.tmp\n");
	die_temp();
}

void
die_rename()
{
	substdio_putsflush(subfderr, "qmail-newu: fatal: unable to move users/cdb.tmp to users/cdb\n");
	die_temp();
}

struct cdbmss   cdbmss;
stralloc        key = { 0 };
stralloc        data = { 0 };

char            inbuf[1024];
substdio        ssin;

int             fd;
int             fdtemp;

stralloc        line = { 0 };
int             match;

stralloc        wildchars = { 0 };

int
main(int argc, char **argv)
{
	int             i;
	int             numcolons;
	char           *assigndir;

	assigndir = (assigndir = env_get("ASSIGNDIR")) ? assigndir : auto_assign;
	if (chdir(argc == 1 ? assigndir : argv[1]) == -1)
		die_chdir(argc == 1 ? assigndir : argv[1]);
	if ((fd = open_read("assign")) == -1)
		die_opena();
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof(inbuf));
	if ((fdtemp = open_trunc("cdb.tmp")) == -1)
		die_opent();
	if (cdbmss_start(&cdbmss, fdtemp) == -1)
		die_writet();
	if (!stralloc_copys(&wildchars, ""))
		die_nomem();
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') != 0)
			die_reada();
		if (line.len && (line.s[0] == '.'))
			break;
		if (!match)
			die_format();
		if (byte_chr(line.s, line.len, '\0') < line.len)
			die_format();
		i = byte_chr(line.s, line.len, ':');
		if (i == line.len)
			die_format();
		if (i == 0)
			die_format();
		if (!stralloc_copys(&key, "!"))
			die_nomem();
		if (line.s[0] == '+')
		{
			if (!stralloc_catb(&key, line.s + 1, i - 1))
				die_nomem();
			case_lowerb(key.s, key.len);
			if (i >= 2)
			{
				if (byte_chr(wildchars.s, wildchars.len, line.s[i - 1]) == wildchars.len)
				{
					if (!stralloc_append(&wildchars, line.s + i - 1))
						die_nomem();
				}
			}
		} else
		{
			if (!stralloc_catb(&key, line.s + 1, i - 1))
				die_nomem();
			if (!stralloc_0(&key))
				die_nomem();
			case_lowerb(key.s, key.len);
		}
		if (!stralloc_copyb(&data, line.s + i + 1, line.len - i - 1))
			die_nomem();
		numcolons = 0;
		for (i = 0; i < data.len; ++i)
		{
			if (data.s[i] == ':')
			{
				data.s[i] = 0;
				if (++numcolons == 6)
					break;
			}
		}
		if (numcolons < 6)
			die_format();
		data.len = i;

		if (cdbmss_add(&cdbmss, (unsigned char *) key.s, key.len, (unsigned char *) data.s, data.len) == -1)
			die_writet();
	}
	close(fd);
	if (cdbmss_add(&cdbmss, (unsigned char *) "", 0, (unsigned char *) wildchars.s, wildchars.len) == -1)
		die_writet();
	if (cdbmss_finish(&cdbmss) == -1)
		die_writet();
	if (fsync(fdtemp) == -1)
		die_writet();
	if (close(fdtemp) == -1)
		die_writet();/*- NFS stupidity */
	if (rename("cdb.tmp", "cdb") == -1)
		die_rename();
	return(0);
}

void
getversion_qmail_newu_c()
{
	static char    *x = "$Id: qmail-newu.c,v 1.9 2016-05-18 15:20:59+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
