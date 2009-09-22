/*
 * $Log: cdb.h,v $
 * Revision 1.2  2005-05-13 23:43:59+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
/*- Public domain. */

#ifndef CDB_H
#define CDB_H

#include "uint32.h"

#define CDB_HASHSTART 5381
uint32          cdb_hashadd(uint32, unsigned char);
uint32          cdb_hash(char *, unsigned int);

struct cdb
{
	char           *map;	/*- 0 if no map is available */
	int             fd;
	uint32          size;	/*- initialized if map is nonzero */
	uint32          loop;	/*- number of hash slots searched under this key */
	uint32          khash;	/*- initialized if loop is nonzero */
	uint32          kpos;	/*- initialized if loop is nonzero */
	uint32          hpos;	/*- initialized if loop is nonzero */
	uint32          hslots;	/*- initialized if loop is nonzero */
	uint32          dpos;	/*- initialized if cdb_findnext() returns 1 */
	uint32          dlen;	/*- initialized if cdb_findnext() returns 1 */
};

void            cdb_free(struct cdb *);
void            cdb_init(struct cdb *, int fd);

int             cdb_read(struct cdb *, char *, unsigned int, uint32);

void            cdb_findstart(struct cdb *);
int             cdb_findnext(struct cdb *, char *, unsigned int);
int             cdb_find(struct cdb *, char *, unsigned int);

#define cdb_datapos(c) ((c)->dpos)
#define cdb_datalen(c) ((c)->dlen)

#endif
