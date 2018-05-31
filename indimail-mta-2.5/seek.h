/*
 * $Log: seek.h,v $
 * Revision 1.3  2004-10-11 14:01:51+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:46+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef SEEK_H
#define SEEK_H

#define seek_begin(fd) (seek_set((fd),(seek_pos) 0))

typedef unsigned long seek_pos;

seek_pos        seek_cur(int);
int             seek_set(int, seek_pos);
int             seek_end(int);
int             seek_trunc(int, seek_pos);

#endif
