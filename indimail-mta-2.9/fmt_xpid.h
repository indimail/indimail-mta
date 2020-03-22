/*
 * $Log: fmt_xpid.h,v $
 * Revision 1.2  2004-10-11 13:53:59+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-09-19 18:54:26+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef FMT_XPID_H
#define FMT_XPID_H
#include <sys/types.h>

unsigned int    fmt_xpid(char *, const pid_t, const int);
#endif
