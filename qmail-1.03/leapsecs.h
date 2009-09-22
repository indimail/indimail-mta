/*
 * $Log: leapsecs.h,v $
 * Revision 1.2  2004-10-11 13:54:54+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.1  2004-06-16 01:20:22+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef LEAPSECS_H
#define LEAPSECS_H
#include "tai.h"

int             leapsecs_init(void);
int             leapsecs_read(void);
void            leapsecs_add(struct tai *, int);
int             leapsecs_sub(struct tai *);

#endif
