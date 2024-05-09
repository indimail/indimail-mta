/*
 * $Log: quote.h,v $
 * Revision 1.3  2004-10-11 14:00:27+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:32+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef QUOTE_H
#define QUOTE_H

int             quote_need(const char *, unsigned int);
int             quote(stralloc *, stralloc *);
int             quote2(stralloc *, const char *);

#endif
