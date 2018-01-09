/*
 * $Log: cdbmake.h,v $
 * Revision 1.3  2004-10-11 13:49:51+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:58:01+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CDBMAKE_H
#define CDBMAKE_H

#include "uint32.h"

#define CDBMAKE_HPLIST 1000

struct cdbmake_hp
{
	uint32          h;
	uint32          p;
};

struct cdbmake_hplist
{
	struct cdbmake_hp hp[CDBMAKE_HPLIST];
	struct cdbmake_hplist *next;
	int             num;
};

struct cdbmake
{
	char            final[2048];
	uint32          count[256];
	uint32          start[256];
	struct cdbmake_hplist *head;
	struct cdbmake_hp *split;	/*- includes space for hash */
	struct cdbmake_hp *hash;
	uint32          numentries;
};

#define CDBMAKE_HASHSTART ((uint32) 5381)
void            cdbmake_pack(unsigned char *, uint32);
uint32          cdbmake_hashadd(uint32, unsigned int);
void            cdbmake_init(struct cdbmake *);
int             cdbmake_add(struct cdbmake *, uint32, uint32, char *(*alloc)());
int             cdbmake_split(struct cdbmake *, char *(*alloc)());
uint32          cdbmake_throw(struct cdbmake *, uint32, int);

#endif
