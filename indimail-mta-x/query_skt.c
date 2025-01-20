/*
 * $Log: query_skt.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2018-04-25 22:49:59+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <errno.h>
#include "error.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "fn_handler.h"
#include "stralloc.h"

int
query_skt(int fd, const char *ipaddr, stralloc *queryp, char *responsep,
		int maxresponsesize, int timeout, void (*timeoutfn) (void), void (*errfn) (const char *))
{
	int             r = 0;

	if (timeoutwrite(timeout, fd, queryp->s, queryp->len) < 0)
		return (errfn ? fn_handler(errfn, 0, 0, "write") : -1);
	if ((r = timeoutread(timeout, fd, responsep, maxresponsesize)) == -1) {
  		if (errno == error_timeout) {
			responsep[0] = 2;
			return (errfn ? fn_handler(errfn, timeoutfn, 1, 0) : -1);
		}
		return (errfn ? fn_handler(errfn, 0, 0, ipaddr) : -1);
	}
	return (r);
}

void
getversion_query_skt_c()
{
	const char     *x = "$Id: query_skt.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
