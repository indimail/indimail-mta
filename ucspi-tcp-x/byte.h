/*
 * $Log: byte.h,v $
 * Revision 1.2  2005-05-13 23:32:13+05:30  Cprogrammer
 * code indented
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef BYTE_H
#define BYTE_H

unsigned int    byte_chr();
unsigned int    byte_rchr();
void            byte_copy();
void            byte_copyr();
int             byte_diff();
void            byte_zero();

#define byte_equal(s,n,t) (!byte_diff((s),(n),(t)))

#endif
