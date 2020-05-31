/*
 * $Log: alloc.h,v $
 * Revision 1.2  2019-05-26 12:03:22+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef ALLOC_H
#define ALLOC_H

extern /*@null@*//*@out@*/
char           *alloc(unsigned int);
void            alloc_free(char *);
#ifdef _ALLOC_
int             alloc_re();
#else
int             alloc_re(char *, unsigned int, unsigned int);
#endif

#endif
