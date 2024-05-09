/*
 * $Log: variables.c,v $
 * Revision 1.5  2018-05-30 23:26:40+05:30  Cprogrammer
 * moved noipv6 variable to variables.c
 *
 * Revision 1.4  2017-03-21 15:40:38+05:30  Cprogrammer
 * added certdir variable
 *
 * Revision 1.3  2004-10-22 20:32:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:25:02+05:30  Cprogrammer
 * added RCS log
 *
 */
const char     *queuedir;
const char     *controldir;
const char     *certdir;

#include "haveip6.h"
#ifdef LIBC_HAS_IP6
int             noipv6 = 0;
#else
int             noipv6 = 1;
#endif

void
getversion_variables_c()
{
	const char     *x = "$Id: variables.c,v 1.5 2018-05-30 23:26:40+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
