/*
 * $Log: testzero.c,v $
 * Revision 1.1  2008-09-16 09:09:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "uint32.h"
#include "scan.h"
#include "exit.h"
#include "strerr.h"
#include "cdb_make.h"

#define FATAL "testzero: fatal: "

void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write: ");
}

static char     key[4];
static char     data[65536];
struct cdb_make c;

int
main(int argc, char **argv)
{
	unsigned long   loop;

	if (!*argv)
		_exit(0);
	if (!*++argv)
		_exit(0);
	scan_ulong(*argv, &loop);
	if (cdb_make_start(&c, 1) == -1)
		die_write();
	while (loop) {
		uint32_pack(key, --loop);
		if (cdb_make_add(&c, key, 4, data, sizeof data) == -1)
			die_write();
	}
	if (cdb_make_finish(&c) == -1)
		die_write();
	_exit(0);
}

void
getversion_testzero_c()
{
	static char    *x = "$Id: testzero.c,v 1.1 2008-09-16 09:09:05+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
