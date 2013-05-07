#ifndef	_LISTSORT_H
#define	_LISTSORT_H

#define	LISTSORT

#include "bftypes.h"

typedef struct element element;
struct element {
    element *next, *prev;
    int i;
};

typedef int fcn_compare(const element *a, const element *b);

extern element *listsort(element *list, fcn_compare *compare);

#endif
