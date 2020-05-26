/*
 * $Log: ip4_bit.h,v $
 * Revision 1.1  2013-08-06 07:58:20+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef IP_BIT_H
#define IP_BIT_H

#include "stralloc.h"
#define FMT_ULONG 40

extern int getaddressasbit(char *, int, stralloc *);
extern int getbitasaddress(stralloc *);

#endif
