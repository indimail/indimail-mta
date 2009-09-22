/*
 * $Log: alloc.h,v $
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef ALLOC_H
#define ALLOC_H

extern /*@null@*//*@out@*/char *alloc();
extern void alloc_free();
extern int alloc_re();

#endif
