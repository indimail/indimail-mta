/*
 * $Log: getln.h,v $
 * Revision 1.3  2004-10-11 13:54:09+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:59:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef GETLN_H
#define GETLN_H
#include "substdio.h"
#include "stralloc.h"

int             getln(substdio *, stralloc *, int *, int);
int             getln2(substdio *, stralloc *, char **, unsigned int *, int);

#endif
