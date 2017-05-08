#ifndef CDB_H
#define CDB_H

#include "uint32.h"

uint32 cdb_hash(unsigned char *, unsigned int);
uint32 cdb_unpack(unsigned char *);

int cdb_bread(int, char *, int);
int cdb_seek(int, unsigned char *, unsigned int, uint32 *);

#endif
