/*
 * $Log: tai.h,v $
 * Revision 1.2  2005-05-13 23:53:54+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef TAI_H
#define TAI_H

#include "uint64.h"

struct tai
{
	uint64          x;
};

#define tai_unix(t,u) ((void) ((t)->x = 4611686018427387914ULL + (uint64) (u)))

void            tai_now(struct tai *);

#define tai_approx(t) ((double) ((t)->x))

void            tai_add(struct tai *, struct tai *, struct tai *);
void            tai_sub(struct tai *, struct tai *, struct tai *);
#define tai_less(t,u) ((t)->x < (u)->x)

#define TAI_PACK 8
void            tai_pack(char *, struct tai *);
void            tai_unpack(char *, struct tai *);

void            tai_uint(struct tai *, unsigned int);

#endif
