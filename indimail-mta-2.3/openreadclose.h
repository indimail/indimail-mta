/*
 * $Log: openreadclose.h,v $
 * Revision 1.2  2004-10-11 13:57:05+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-18 22:59:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef OPENREADCLOSE_H
#define OPENREADCLOSE_H

#include "stralloc.h"

int             openreadclose(char *, stralloc *, unsigned int);
int             readclose_append(int, stralloc *, unsigned int);
int             readclose(int, stralloc *, unsigned int);

#endif
