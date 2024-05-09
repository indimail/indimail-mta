/*
 * $Log: getconf.h,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-11 13:54:04+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-18 22:59:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef GETCONF_H
#define GETCONF_H

int             getconf(stralloc *, const char *, int, const char *, const char *);
int             getconf_line(stralloc *, const char *, int, const char *, const char *);

#endif
