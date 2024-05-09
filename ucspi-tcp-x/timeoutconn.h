/*
 * $Log: timeoutconn.h,v $
 * Revision 1.5  2023-06-18 13:24:44+05:30  Cprogrammer
 * added timeoutconn_un function
 *
 * Revision 1.4  2021-05-12 21:05:17+05:30  Cprogrammer
 * define arguments as array subscripts to fix gcc 11 warnings
 *
 * Revision 1.3  2005-06-10 09:14:04+05:30  Cprogrammer
 * added ipv6 support
 *
 * Revision 1.2  2005-05-13 23:53:58+05:30  Cprogrammer
 * code indentationm
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TIMEOUTCONN_H
#define TIMEOUTCONN_H

#include "uint16.h"
#include "haveip6.h"
#ifdef LIBC_HAS_IP6
#include "uint32.h"
#endif

int             timeoutconn(int, char ip[4], uint16, unsigned int);
#ifdef LIBC_HAS_IP6
int             timeoutconn6(int, char ip[16], uint16, unsigned int, uint32);
#endif
int             timeoutconn_un(int, const char *, unsigned int);

#endif
