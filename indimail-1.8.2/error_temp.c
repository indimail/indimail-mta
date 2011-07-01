/*
 * $Log: error_temp.c,v $
 * Revision 2.2  2008-07-13 19:16:17+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.1  2002-12-11 10:28:09+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 1.1  2001-12-01 01:45:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <errno.h>

#ifndef	lint
static char     sccsid[] = "$Id: error_temp.c,v 2.2 2008-07-13 19:16:17+05:30 Cprogrammer Stab mbhangui $";
#endif

#define X(n) if (e == n) return 1;

int
error_temp(e)
	int             e;
{
	X(EINTR) 
#ifdef ERESTART
	X(ERESTART) 
#endif
	X(ENOMEM)
	X(ETXTBSY) 
	X(EIO) 
	X(ETIMEDOUT) 
	X(EWOULDBLOCK) 
	X(EAGAIN)
#ifdef EDEADLK
	X(EDEADLK)
#endif
#ifdef EBUSY
	X(EBUSY)
#endif
#ifdef ENFILE
	X(ENFILE)
#endif
#ifdef EMFILE
	X(EMFILE)
#endif
#ifdef EFBIG
	X(EFBIG)
#endif
#ifdef ENOSPC
	X(ENOSPC)
#endif
#ifdef ENETDOWN
	X(ENETDOWN)
#endif
#ifdef ENETUNREACH
	X(ENETUNREACH)
#endif
#ifdef ENETRESET
	X(ENETRESET)
#endif
#ifdef ECONNABORTED
	X(ECONNABORTED)
#endif
#ifdef ECONNRESET
	X(ECONNRESET)
#endif
#ifdef ENOBUFS
	X(ENOBUFS)
#endif
#ifdef ETOOMANYREFS
	X(ETOOMANYREFS)
#endif
#ifdef ECONNREFUSED
	X(ECONNREFUSED)
#endif
#ifdef EHOSTDOWN
	X(EHOSTDOWN)
#endif
#ifdef EHOSTUNREACH
	X(EHOSTUNREACH)
#endif
#ifdef EPROCLIM
	X(EPROCLIM)
#endif
#ifdef EUSERS
	X(EUSERS)
#endif
#ifdef EDQUOT
	X(EDQUOT)
#endif
#ifdef ESTALE
	X(ESTALE)
#endif
#ifdef ENOLCK
	X(ENOLCK)
#endif
	return 0;
}

void
getversion_error_temp_c()
{
	printf("%s\n", sccsid);
	return;
}
