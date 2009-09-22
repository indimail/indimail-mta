/*
 * $Log: error_temp.c,v $
 * Revision 1.4  2004-10-22 20:24:54+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:18:38+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "error.h"

#define X(n) if (e == n) return 1;

int error_temp(e)
int e;
{
  X(error_intr)
  X(error_nomem)
  X(error_txtbsy)
  X(error_io)
  X(error_timeout)
  X(error_wouldblock)
  X(error_again)
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
	static char    *x = "$Id: error_temp.c,v 1.4 2004-10-22 20:24:54+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
