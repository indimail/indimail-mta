/*
 * $Log: headerbody.h,v $
 * Revision 1.4  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.3  2004-10-11 13:54:15+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:00:00+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef HEADERBODY_H
#define HEADERBODY_H
#include "substdio.h"
#include "stralloc.h"

int             headerbody(substdio *, void (*) (stralloc *), void (*) (void), void (*) (stralloc *));

#endif
