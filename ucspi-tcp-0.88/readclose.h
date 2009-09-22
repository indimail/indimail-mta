/*
 * $Log: readclose.h,v $
 * Revision 1.2  2005-05-13 23:46:15+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef READCLOSE_H
#define READCLOSE_H

#include "stralloc.h"

int             readclose_append(int, stralloc *, unsigned int);
int             readclose(int, stralloc *, unsigned int);

#endif
