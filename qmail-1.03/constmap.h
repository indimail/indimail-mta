/*
 * $Log: constmap.h,v $
 * Revision 1.3  2004-10-11 13:51:38+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:16+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CONSTMAP_H
#define CONSTMAP_H

typedef unsigned long constmap_hash;

struct constmap
{
	int             num;
	constmap_hash   mask;
	constmap_hash  *hash;
	int            *first;
	int            *next;
	char          **input;
	int            *inputlen;
};

int             constmap_init(struct constmap *, char *, int, int);
void            constmap_free(struct constmap *);
char           *constmap(struct constmap *, char *, int);

#endif
