/*
 * $Log: prioq.h,v $
 * Revision 1.4  2021-06-03 12:44:11+05:30  Cprogrammer
 * allow prioq to be ordered from max to min
 *
 * Revision 1.3  2004-10-11 13:59:54+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:16+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef PRIOQ_H
#define PRIOQ_H

#include "datetime.h"
#include "gen_alloc.h"

struct prioq_elt
{
	datetime_sec    dt;
	unsigned long   id;
};
typedef enum {
	min,
	max,
} prioq_type;

GEN_ALLOC_typedef(prioq, struct prioq_elt, p, len, a)

int             prioq_insert(prioq_type, prioq *, struct prioq_elt *);
int             prioq_test(prioq *, struct prioq_elt *);
void            prioq_del(prioq_type, prioq *);

#endif
