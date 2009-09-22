/*
 * $Log: gen_alloc.h,v $
 * Revision 1.2  2004-06-18 22:59:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef GEN_ALLOC_H
#define GEN_ALLOC_H

#define GEN_ALLOC_typedef(ta,type,field,len,a) \
  typedef struct ta { type *field; unsigned int len; unsigned int a; } ta;

#endif
