/*
 * $Log: cdb.h,v $
 * Revision 1.4  2008-09-16 08:24:45+05:30  Cprogrammer
 * mods for adding cdbmake
 *
 * Revision 1.3  2004-10-11 13:49:29+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 22:57:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CDB_H
#define CDB_H

#include "uint32.h"
#define CDB_HASHSTART 5381
#define cdb_datapos(c) ((c)->dpos)
#define cdb_datalen(c) ((c)->dlen)

struct cdb
{
	char           *map;		/* 0 if no map is available */
	int             fd;
	uint32          size;		/* initialized if map is nonzero */
	uint32          loop;		/* number of hash slots searched under this key */
	uint32          khash;		/* initialized if loop is nonzero */
	uint32          kpos;		/* initialized if loop is nonzero */
	uint32          hpos;		/* initialized if loop is nonzero */
	uint32          hslots;		/* initialized if loop is nonzero */
	uint32          dpos;		/* initialized if cdb_findnext() returns 1 */
	uint32          dlen;		/* initialized if cdb_findnext() returns 1 */
};

uint32          cdb_hash(unsigned char *, unsigned int);
uint32          cdb_hashadd(uint32, unsigned char);
uint32          cdb_unpack(unsigned char *);
int             cdb_bread(int, char *, int);
int             cdb_seek(int, char *, unsigned int, uint32 *);
int             cdb_find(struct cdb *,char *,unsigned int);
void            cdb_findstart(struct cdb *);
int             cdb_findnext(struct cdb *,char *,unsigned int);
void            _cdb_init(struct cdb *,int fd);
int             cdb_read(struct cdb *,char *,unsigned int,uint32);

#endif
