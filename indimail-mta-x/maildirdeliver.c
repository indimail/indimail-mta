/*
 * $Log: maildirdeliver.c,v $
 * Revision 1.1  2021-05-16 23:01:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <env.h>
#include <strerr.h>
#include <stralloc.h>
#include "maildir_child.h"

#define FATAL "maildirdeliver: fatal: "

static stralloc rpline = { 0 };
static stralloc dtline = { 0 };

int
main(int argc, char **argv)
{
	char           *ptr, *dir;
	int             i;

	if (argc != 2)
		strerr_die1x(100, "usage: maildirdeliver dir");
	dir = argv[1];
	if (!*dir)
		strerr_die1x(100, "usage: maildirdeliver dir");
	umask(077);
	if ((ptr = env_get("RPLINE"))) {
		if (!stralloc_copys(&rpline, ptr) || !stralloc_0(&rpline))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
	}
	if ((ptr = env_get("DTLINE"))) {
		if (!stralloc_copys(&dtline, ptr) || !stralloc_0(&dtline))
			strerr_die1x(111, "Out of memory. (#4.3.0)");
	}
	ptr = env_get("QQEH");
	i = maildir_child(dir, &rpline, &dtline, ptr);
	switch (i)
	{
	case 0:
		break;
	case 2:
		strerr_die1x(111, "Unable to chdir to maildir. (#4.2.1)");
	case 3:
		strerr_die1x(111, "Timeout on maildir delivery. (#4.3.0)");
	case 4:
		strerr_die1x(111, "Unable to read message. (#4.3.0)");
	case 5:
		strerr_die1x(100, "Recipient's mailbox is full");
	default:
		strerr_die1x(111, "Temporary error on maildir delivery. (#4.3.0)");
	}
	_exit(0);
}

void
getversion_maildirdeliver_c()
{
	static char    *x = "$Id: maildirdeliver.c,v 1.1 2021-05-16 23:01:48+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
