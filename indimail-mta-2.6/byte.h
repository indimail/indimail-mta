/*
 * $Log: byte.h,v $
 * Revision 1.4  2004-10-09 23:20:04+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-08-14 01:49:18+05:30  Cprogrammer
 * added code for SPF
 *
 * Revision 1.2  2004-06-18 22:57:22+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef BYTE_H
#define BYTE_H

unsigned int    byte_chr(char *, unsigned int, int);
unsigned int    byte_rchr(char *, unsigned int, int);
unsigned int    byte_cspn(char *, unsigned int, char *);
unsigned int    byte_rcspn(char *, unsigned int, char *);
void            byte_copy(char *, unsigned int, char *);
void            byte_copyr(char *, unsigned int, char *);
int             byte_diff(char *, unsigned int, char *);
void            byte_zero(char *, unsigned int);

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
