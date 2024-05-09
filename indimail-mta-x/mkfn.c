/*
 * $Log: mkfn.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:27:33+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-19 18:54:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include "fmt.h"
#include "fmt_xpid.h"
#include "timestamp.h"
#include "auto_pidt.h"

void
mkfn(char *const buf, const unsigned short count)
{
	char           *fn = buf;
	timestamp(fn);
	fn += TIMESTAMP;
	*fn++ = '.';
	fn += fmt_xpid(fn, getppid(), 2 * PID_BYTES);
	*fn++ = '.';
	fn += fmt_xpid(fn, (pid_t) count, sizeof(unsigned short));
	*fn = '\0';
}

void
getversion_mkfn_c()
{
	const char     *x = "$Id: mkfn.c,v 1.3 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
