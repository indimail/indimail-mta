/*
 * $Log: qquote.c,v $
 * Revision 1.2  2004-10-22 20:29:43+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:58+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "mess822.h"
#include "subfd.h"
#include "substdio.h"
#include "strerr.h"

#define FATAL "quote: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

stralloc        quoted = { 0 };
stralloc        addr = { 0 };
char           *comment = 0;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	if (!stralloc_copys(&addr, "@"))
		nomem();

	if (argv[1])
	{
		if (!stralloc_copys(&addr, argv[1]))
			nomem();
		if (!stralloc_cats(&addr, "@"))
			nomem();

		if (argv[2])
		{
			if (!stralloc_cats(&addr, argv[2]))
				nomem();

			comment = argv[3];
		}
	}

	if (!stralloc_0(&addr))
		nomem();
	if (!mess822_quote(&quoted, addr.s, comment))
		nomem();

	if (!stralloc_append(&quoted, "\n"))
		nomem();

	substdio_putflush(subfdout, quoted.s, quoted.len);

	_exit(0);
}

void
getversion_qquote_c()
{
	static char    *x = "$Id: qquote.c,v 1.2 2004-10-22 20:29:43+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
