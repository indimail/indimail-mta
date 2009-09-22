/*
 * $Log: getln.h,v $
 * Revision 1.2  2005-05-13 23:45:33+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef GETLN_H
#define GETLN_H

#include "buffer.h"
#include "stralloc.h"

int             getln(buffer *, stralloc *, int *, int);
int             getln2(buffer *, stralloc *, char **, unsigned int *, int);

#endif
