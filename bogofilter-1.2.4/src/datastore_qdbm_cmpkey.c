/* $Id: datastore_qdbm_cmpkey.c 4915 2004-11-01 10:17:15Z m-a $ */

#include "datastore_qdbm.h"

int cmpkey(const char *aptrin, int asiz, const char *bptrin, int bsiz)
{
    int aiter, biter;
    const unsigned char *aptr = (const unsigned char *)aptrin;
    const unsigned char *bptr = (const unsigned char *)bptrin;

    for (aiter = 0, biter = 0; aiter < asiz && biter < bsiz; ++aiter, ++biter) {
	if (aptr[aiter] != bptr[biter])
	    return (aptr[aiter] < bptr[biter]) ? -1 : 1;
    }

    if (aiter == asiz && biter == bsiz)
	return 0;

    return (aiter == asiz) ? -1 : 1;
}


