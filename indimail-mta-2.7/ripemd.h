/*
 * $Log: ripemd.h,v $
 * Revision 2.2  2019-07-18 10:54:03+05:30  Cprogrammer
 * mode change - removed x bit
 *
 * Revision 2.1  2011-10-27 14:31:03+05:30  Cprogrammer
 * header for ripemd.c
 *
 */
#ifndef QP_RMD_H
#define QP_RMD_H

#include "typesx.h"
#include <string.h>

#ifndef	lint
static char     sccsidripemdh[] = "$Id: ripemd.h,v 2.2 2019-07-18 10:54:03+05:30 Cprogrammer Exp mbhangui $";
#endif

typedef struct {
	u32 MD[5]; /* used for the digest */
	u32 X[16]; /* current 16 bit chunk */
	u32 len;   /* full length */
	u8  D[64]; /* 512 bit data block */
	u32 l;     /* current length of data block -> D[l] */
} RIPEMD160_CTX;

void RIPEMD160_Init(RIPEMD160_CTX *ctx);
void RIPEMD160_Update(RIPEMD160_CTX *ctx, const void *data, u32 size);
void RIPEMD160_Final(u8 *result, RIPEMD160_CTX *ctx);

#endif
