/*
 * $Log: getip.h,v $
 * Revision 1.1  2024-10-05 22:55:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef GETIP_H
#define GETIP_H

#include <stralloc.h>

int             getip(const char *, stralloc *);
int             isip(const char *);

#endif
