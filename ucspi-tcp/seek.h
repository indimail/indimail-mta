/*
 * $Log: seek.h,v $
 * Revision 1.2  2005-05-13 23:46:49+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef SEEK_H
#define SEEK_H

typedef unsigned long seek_pos;

unsigned long   seek_cur(int);
int             seek_set(int, seek_pos);
int             seek_end(int);
int             seek_trunc(int, seek_pos);

#define seek_begin(fd) (seek_set((fd),(seek_pos) 0))

#endif
