/*
 * $Log: open.h,v $
 * Revision 1.2  2005-05-13 23:45:57+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef OPEN_H
#define OPEN_H

int             open_read(char *);
int             open_excl(char *);
extern int      open_append(char *);
int             open_trunc(char *);
int             open_write(char *);

#endif
