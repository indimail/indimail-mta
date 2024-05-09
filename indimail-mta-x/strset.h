/*
 * $Log: strset.h,v $
 * Revision 1.2  2021-03-28 23:49:51+05:30  Cprogrammer
 * minor code beautification
 *
 * Revision 1.1  2004-10-21 22:48:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef STRSET_H
#define STRSET_H

#include "uint32.h"

typedef struct strset_list
{
	uint32          h;
	int             next;
} strset_list;

typedef struct
{
	int             mask;		/*- mask + 1 is power of 2, size of hash table */
	int             n;			/*- number of entries used in list and x */
	int             a;			/*- number of entries allocated in list and x */
	int            *first;		/*- first[h] is front of hash list h */
	strset_list    *p;			/*- p[i].next is next; p[i].h is hash of x[i] */
	char          **x;			/*- x[i] is entry i */
} strset;

uint32          strset_hash(const char *);
int             strset_init(strset *);
char           *strset_in(strset *, const char *);
int             strset_add(strset *, const char *);

#endif
