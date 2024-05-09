/*
 * $Log: tablematch.h,v $
 * Revision 1.2  2009-08-29 15:27:05+05:30  Cprogrammer
 * added flag argument to tablematch()
 *
 * Revision 1.1  2004-05-15 00:04:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _TABLEMAT_H
#define _TABLEMAT_H
int             tablematch(const char *, const char *, const char *);
int             matchinet(const char *, const char *, char);
#endif
