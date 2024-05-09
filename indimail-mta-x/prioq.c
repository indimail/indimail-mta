/*
 * $Log: prioq.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2021-06-03 18:03:39+05:30  Cprogrammer
 * allow prioq to be ordered from max to min
 *
 * Revision 1.6  2020-11-22 23:12:00+05:30  Cprogrammer
 * removed supression of ANSI C proto
 *
 * Revision 1.5  2020-05-10 17:46:56+05:30  Cprogrammer
 * GEN_ALLOC refactoring (by Rolf Eike Beer) to fix memory overflow reported by Qualys Security Advisory
 *
 * Revision 1.4  2004-10-22 20:28:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-11 13:57:14+05:30  Cprogrammer
 * prevent inclusion of alloc.h with prototypes
 *
 * Revision 1.2  2004-07-17 21:20:28+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "alloc.h"
#include "gen_allocdefs.h"
#include "prioq.h"

GEN_ALLOC_readyplus(prioq, struct prioq_elt, p, len, a, 100, prioq_readyplus)
static prioq_type type = min;

/*-
 * prioq_insert()
 * 1. insert into pq member with pq->p[0].dt have the min value
 * or
 * 1. insert into pq member with pq->p[0].dt have the max value
 * 2. increment length of pq by 1.
 *
 * You can easily sort in increasing order of timestamp
 * by doing
 *
 * struct prioq_elt pe;
 *
 * while (prioq_get(&pq, &pe)) {
 *   prioq_del(&pq);
 *   process(pe.id, pe.dt);
 * }
 */
int
prioq_insert(prioq_type t, prioq *pq, struct prioq_elt *pe)
{
	int             i, j;

	if (!prioq_readyplus(pq, 1))
		return 0;
	if (t != min && t != max)
		return -1;
	type = t;
	j = pq->len++;
	while (j) {
		i = (j - 1) / 2;
		if (t == min && pq->p[i].dt <= pe->dt)
			break;
		else
		if (t == max && pq->p[i].dt >= pe->dt)
			break;
		pq->p[j] = pq->p[i];
		j = i;
	}
	pq->p[j] = *pe;
	return 1;
}

/*
 * find the structure having lowest or greatest timestamp (pq->p[0].dt)
 * depending on whether you called prioq_insert(min, ..) or prioq_insert(max, ..)
 * and store it in pe
 */
int
prioq_get(prioq *pq, struct prioq_elt *pe)
{
	if (!pq->p || !pq->len)
		return 0;
	*pe = pq->p[0];
	return 1;
}

/*
 * 1. delete structure having lowest  timestamp (pq->p[0].dt)
 * or
 * 1. delete structure having highest timestamp (pq->p[0].dt)
 * 2. store the member having the next lowest timestamp
 *    in pq->p[0].dt
 * 3. decrement length of pq by 1
 */
void
prioq_del(prioq *pq)
{
	int             i, j, n;

	if (!pq->p || !(n = pq->len))
		return;
	if (type != min && type != max)
		return;
	i = 0;
	--n;
	for (;;) {
		j = i + i + 2;
		if (j > n)
			break;
		if (type == min) {
			if (pq->p[j - 1].dt <= pq->p[j].dt)
				--j;
			if (pq->p[n].dt <= pq->p[j].dt)
				break;
		} else
		if (type == max) {
			if (pq->p[j - 1].dt >= pq->p[j].dt)
				--j;
			if (pq->p[n].dt >= pq->p[j].dt)
				break;
		}
		pq->p[i] = pq->p[j];
		i = j;
	}
	pq->p[i] = pq->p[n];
	pq->len = n;
}

void
getversion_prioq_c()
{
	const char     *x = "$Id: prioq.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
