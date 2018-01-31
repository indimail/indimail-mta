/*
 * $Log: alloc.h,v $
 * Revision 1.3  2004-10-11 13:48:20+05:30  Cprogrammer
 * added prototypes
 *
 * Revision 1.2  2004-06-18 22:52:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef ALLOC_H
#define ALLOC_H

/* @null@ *//* @out@ */
char           *alloc(unsigned int);
void            alloc_free(char *);
#ifdef _ALLOC_
int             alloc_re();
#else
int             alloc_re(char *, unsigned int, unsigned int);
#endif

#endif
