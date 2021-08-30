/*
 * $Log: columnt.c,v $
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.5  2020-11-24 13:44:34+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.4  2004-10-22 20:23:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:34:31+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-01-03 13:42:59+05:30  Cprogrammer
 * merged two if statements
 *
 * Revision 1.1  2004-01-02 23:51:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <stralloc.h>
#include <alloc.h>
#include <strerr.h>
#include <substdio.h>
#include "slurpclose.h"
#include <noreturn.h>

#define FATAL "columnt: fatal: "

char            outbuf[4096];
substdio        ssout = SUBSTDIO_FDBUF(write, 1, outbuf, sizeof outbuf);

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
die_read()
{
	strerr_die2sys(111, FATAL, "unable to read input: ");
}

no_return void
die_write()
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

stralloc        file = { 0 };
int            *width;
int             maxfield = 0;

void
nothing()
{
	;
}

void
printline()
{
	if (substdio_put(&ssout, "\n", 1) == -1)
		die_write();
}

void
maxfield_check(fieldnum, buf, len)
	int             fieldnum;
	char           *buf;
	int             len;
{
	if (fieldnum > maxfield)
		maxfield = fieldnum;
}

void
width_check(fieldnum, buf, len)
	int             fieldnum;
	char           *buf;
	int             len;
{
	if (len > width[fieldnum])
		width[fieldnum] = len;
}

void
width_init()
{
	int             i;

	width = (int *) alloc((maxfield + 1) * sizeof(int));
	if (!width)
		nomem();
	for (i = 0; i <= maxfield; ++i)
		width[i] = 0;
}

void
printfield(fieldnum, buf, len)
	int             fieldnum;
	char           *buf;
	int             len;
{
	int             i;

	if (fieldnum < maxfield)
		for (i = len; i < width[fieldnum]; ++i)
			if (substdio_put(&ssout, " ", 1) == -1)
				die_write();

	if (substdio_put(&ssout, buf, len) == -1)
		die_write();

	if (fieldnum < maxfield)
		if (substdio_put(&ssout, "  ", 2) == -1)
			die_write();
}

void
split(dofield, doline)
	void            (*dofield) ();
	void            (*doline) ();
{
	int             i;
	int             j;
	int             fieldpos;
	int             fieldnum;

	for (j = i = 0; j < file.len; ++j)
		if (file.s[j] == '\n')
		{
			fieldnum = 0;
			for (;;)
			{
				while ((file.s[i] == ' ') || (file.s[i] == '\t'))
					++i;
				if (i == j)
					break;
				fieldpos = i;
				while ((file.s[i] != ' ') && (file.s[i] != '\t') && (file.s[i] != '\n'))
					++i;
				dofield(fieldnum++, file.s + fieldpos, i - fieldpos);
			}
			doline();
			i = j + 1;
		}
}

int
main()
{
	if (slurpclose(0, &file, 4096) == -1)
		die_read();
	if (!file.len)
		_exit(0);
	if (file.s[file.len - 1] != '\n' && !stralloc_append(&file, "\n"))
		nomem();

	split(maxfield_check, nothing);
	width_init();
	split(width_check, nothing);
	split(printfield, printline);

	if (substdio_flush(&ssout) == -1)
		die_write();
	_exit(0);
	/*- Not reached */
	return (0);
}

void
getversion_columnt_c()
{
	static char    *x = "$Id: columnt.c,v 1.6 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
