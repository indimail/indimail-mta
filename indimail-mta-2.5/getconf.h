/*
 * $Log: getconf.h,v $
 * Revision 1.2  2004-10-11 13:54:04+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-18 22:59:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef GETCONF_H
#define GETCONF_H

int             getconf(stralloc *, char *, int, char *, char *);
int             getconf_line(stralloc *, char *, int, char *, char *);

#endif
