/*
 * $Log: rcpthosts.h,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2004-10-11 14:00:29+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:34+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef RCPTHOSTS_H
#define RCPTHOSTS_H

int             rcpthosts_init(void);
int             rcpthosts(const char *, int, int);

#endif
