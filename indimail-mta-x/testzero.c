/*
 * $Log: testzero.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.2  2020-11-24 13:48:40+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.1  2008-09-16 09:09:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <uint32.h>
#include <scan.h>
#include <strerr.h>
#include <cdb_make.h>
#include <noreturn.h>

#define FATAL "testzero: fatal: "

no_return void
die_write(void)
{
	strerr_die2sys(111, FATAL, "unable to write: ");
}


int
main(int argc, char **argv)
{
	unsigned long   loop;
	char            key[4];
	char            data[65536];
	struct          cdb_make c;

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
	const char     *x = "$Id: testzero.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
