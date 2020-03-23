/*
 * $Log: ripemd.c,v $
 * Revision 2.2  2011-12-05 15:10:07+05:30  Cprogrammer
 * added version information
 *
 * Revision 2.1  2011-10-27 14:29:53+05:30  Cprogrammer
 * ripemd routine
 *
 */
#include "ripemd.h"

static void compress(RIPEMD160_CTX *ctx);

/* ROL(x, n) cyclically rotates x over n bits to the left */
/* x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROL(x, n)   (((x) << (n)) | ((x) >> (32-(n))))

/* the five basic functions F(), G() and H() */
#define F(x, y, z)  ((x) ^ (y) ^ (z))
#define G(x, y, z)  (((x) & (y)) | (~(x) & (z)))
#define H(x, y, z)  (((x) | ~(y)) ^ (z))
#define I(x, y, z)  (((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z)  ((x) ^ ((y) | ~(z)))

/* the ten basic operations FF() through III() */
#define FF(a, b, c, d, e, x, s) {\
 (a) += F((b), (c), (d)) + (x);\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define GG(a, b, c, d, e, x, s)  {\
 (a) += G((b), (c), (d)) + (x) + 0x5a827999UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define HH(a, b, c, d, e, x, s)  {\
 (a) += H((b), (c), (d)) + (x) + 0x6ed9eba1UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define II(a, b, c, d, e, x, s)  {\
 (a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcUL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define JJ(a, b, c, d, e, x, s)  {\
 (a) += J((b), (c), (d)) + (x) + 0xa953fd4eUL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define FFF(a, b, c, d, e, x, s) {\
 (a) += F((b), (c), (d)) + (x);\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define GGG(a, b, c, d, e, x, s) {\
 (a) += G((b), (c), (d)) + (x) + 0x7a6d76e9UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define HHH(a, b, c, d, e, x, s) {\
 (a) += H((b), (c), (d)) + (x) + 0x6d703ef3UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define III(a, b, c, d, e, x, s) {\
 (a) += I((b), (c), (d)) + (x) + 0x5c4dd124UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }
#define JJJ(a, b, c, d, e, x, s) {\
 (a) += J((b), (c), (d)) + (x) + 0x50a28be6UL;\
 (a) = ROL((a), (s)) + (e);\
 (c) = ROL((c), 10); }

/* collect four u8s into one word: */
#define bytes_TO_u32(strptr)    \
 (((u32) *((strptr)+3) << 24) | \
  ((u32) *((strptr)+2) << 16) | \
  ((u32) *((strptr)+1) <<  8) | \
  ((u32) *(strptr)))

static void compress(RIPEMD160_CTX *ctx)
{
	u32 aa = ctx->MD[0], aaa = ctx->MD[0];
	u32 bb = ctx->MD[1], bbb = ctx->MD[1];
	u32 cc = ctx->MD[2], ccc = ctx->MD[2];
	u32 dd = ctx->MD[3], ddd = ctx->MD[3];
	u32 ee = ctx->MD[4], eee = ctx->MD[4];

	/* round 1 */
	FF(aa, bb, cc, dd, ee, ctx->X[ 0], 11);
	FF(ee, aa, bb, cc, dd, ctx->X[ 1], 14);
	FF(dd, ee, aa, bb, cc, ctx->X[ 2], 15);
	FF(cc, dd, ee, aa, bb, ctx->X[ 3], 12);
	FF(bb, cc, dd, ee, aa, ctx->X[ 4],  5);
	FF(aa, bb, cc, dd, ee, ctx->X[ 5],  8);
	FF(ee, aa, bb, cc, dd, ctx->X[ 6],  7);
	FF(dd, ee, aa, bb, cc, ctx->X[ 7],  9);
	FF(cc, dd, ee, aa, bb, ctx->X[ 8], 11);
	FF(bb, cc, dd, ee, aa, ctx->X[ 9], 13);
	FF(aa, bb, cc, dd, ee, ctx->X[10], 14);
	FF(ee, aa, bb, cc, dd, ctx->X[11], 15);
	FF(dd, ee, aa, bb, cc, ctx->X[12],  6);
	FF(cc, dd, ee, aa, bb, ctx->X[13],  7);
	FF(bb, cc, dd, ee, aa, ctx->X[14],  9);
	FF(aa, bb, cc, dd, ee, ctx->X[15],  8);

	/* round 2 */
	GG(ee, aa, bb, cc, dd, ctx->X[ 7],  7);
	GG(dd, ee, aa, bb, cc, ctx->X[ 4],  6);
	GG(cc, dd, ee, aa, bb, ctx->X[13],  8);
	GG(bb, cc, dd, ee, aa, ctx->X[ 1], 13);
	GG(aa, bb, cc, dd, ee, ctx->X[10], 11);
	GG(ee, aa, bb, cc, dd, ctx->X[ 6],  9);
	GG(dd, ee, aa, bb, cc, ctx->X[15],  7);
	GG(cc, dd, ee, aa, bb, ctx->X[ 3], 15);
	GG(bb, cc, dd, ee, aa, ctx->X[12],  7);
	GG(aa, bb, cc, dd, ee, ctx->X[ 0], 12);
	GG(ee, aa, bb, cc, dd, ctx->X[ 9], 15);
	GG(dd, ee, aa, bb, cc, ctx->X[ 5],  9);
	GG(cc, dd, ee, aa, bb, ctx->X[ 2], 11);
	GG(bb, cc, dd, ee, aa, ctx->X[14],  7);
	GG(aa, bb, cc, dd, ee, ctx->X[11], 13);
	GG(ee, aa, bb, cc, dd, ctx->X[ 8], 12);

	/* round 3 */
	HH(dd, ee, aa, bb, cc, ctx->X[ 3], 11);
	HH(cc, dd, ee, aa, bb, ctx->X[10], 13);
	HH(bb, cc, dd, ee, aa, ctx->X[14],  6);
	HH(aa, bb, cc, dd, ee, ctx->X[ 4],  7);
	HH(ee, aa, bb, cc, dd, ctx->X[ 9], 14);
	HH(dd, ee, aa, bb, cc, ctx->X[15],  9);
	HH(cc, dd, ee, aa, bb, ctx->X[ 8], 13);
	HH(bb, cc, dd, ee, aa, ctx->X[ 1], 15);
	HH(aa, bb, cc, dd, ee, ctx->X[ 2], 14);
	HH(ee, aa, bb, cc, dd, ctx->X[ 7],  8);
	HH(dd, ee, aa, bb, cc, ctx->X[ 0], 13);
	HH(cc, dd, ee, aa, bb, ctx->X[ 6],  6);
	HH(bb, cc, dd, ee, aa, ctx->X[13],  5);
	HH(aa, bb, cc, dd, ee, ctx->X[11], 12);
	HH(ee, aa, bb, cc, dd, ctx->X[ 5],  7);
	HH(dd, ee, aa, bb, cc, ctx->X[12],  5);

	/* round 4 */
	II(cc, dd, ee, aa, bb, ctx->X[ 1], 11);
	II(bb, cc, dd, ee, aa, ctx->X[ 9], 12);
	II(aa, bb, cc, dd, ee, ctx->X[11], 14);
	II(ee, aa, bb, cc, dd, ctx->X[10], 15);
	II(dd, ee, aa, bb, cc, ctx->X[ 0], 14);
	II(cc, dd, ee, aa, bb, ctx->X[ 8], 15);
	II(bb, cc, dd, ee, aa, ctx->X[12],  9);
	II(aa, bb, cc, dd, ee, ctx->X[ 4],  8);
	II(ee, aa, bb, cc, dd, ctx->X[13],  9);
	II(dd, ee, aa, bb, cc, ctx->X[ 3], 14);
	II(cc, dd, ee, aa, bb, ctx->X[ 7],  5);
	II(bb, cc, dd, ee, aa, ctx->X[15],  6);
	II(aa, bb, cc, dd, ee, ctx->X[14],  8);
	II(ee, aa, bb, cc, dd, ctx->X[ 5],  6);
	II(dd, ee, aa, bb, cc, ctx->X[ 6],  5);
	II(cc, dd, ee, aa, bb, ctx->X[ 2], 12);

	/* round 5 */
	JJ(bb, cc, dd, ee, aa, ctx->X[ 4],  9);
	JJ(aa, bb, cc, dd, ee, ctx->X[ 0], 15);
	JJ(ee, aa, bb, cc, dd, ctx->X[ 5],  5);
	JJ(dd, ee, aa, bb, cc, ctx->X[ 9], 11);
	JJ(cc, dd, ee, aa, bb, ctx->X[ 7],  6);
	JJ(bb, cc, dd, ee, aa, ctx->X[12],  8);
	JJ(aa, bb, cc, dd, ee, ctx->X[ 2], 13);
	JJ(ee, aa, bb, cc, dd, ctx->X[10], 12);
	JJ(dd, ee, aa, bb, cc, ctx->X[14],  5);
	JJ(cc, dd, ee, aa, bb, ctx->X[ 1], 12);
	JJ(bb, cc, dd, ee, aa, ctx->X[ 3], 13);
	JJ(aa, bb, cc, dd, ee, ctx->X[ 8], 14);
	JJ(ee, aa, bb, cc, dd, ctx->X[11], 11);
	JJ(dd, ee, aa, bb, cc, ctx->X[ 6],  8);
	JJ(cc, dd, ee, aa, bb, ctx->X[15],  5);
	JJ(bb, cc, dd, ee, aa, ctx->X[13],  6);

	/* parallel round 1 */
	JJJ(aaa, bbb, ccc, ddd, eee, ctx->X[ 5],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, ctx->X[14],  9);
	JJJ(ddd, eee, aaa, bbb, ccc, ctx->X[ 7],  9);
	JJJ(ccc, ddd, eee, aaa, bbb, ctx->X[ 0], 11);
	JJJ(bbb, ccc, ddd, eee, aaa, ctx->X[ 9], 13);
	JJJ(aaa, bbb, ccc, ddd, eee, ctx->X[ 2], 15);
	JJJ(eee, aaa, bbb, ccc, ddd, ctx->X[11], 15);
	JJJ(ddd, eee, aaa, bbb, ccc, ctx->X[ 4],  5);
	JJJ(ccc, ddd, eee, aaa, bbb, ctx->X[13],  7);
	JJJ(bbb, ccc, ddd, eee, aaa, ctx->X[ 6],  7);
	JJJ(aaa, bbb, ccc, ddd, eee, ctx->X[15],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, ctx->X[ 8], 11);
	JJJ(ddd, eee, aaa, bbb, ccc, ctx->X[ 1], 14);
	JJJ(ccc, ddd, eee, aaa, bbb, ctx->X[10], 14);
	JJJ(bbb, ccc, ddd, eee, aaa, ctx->X[ 3], 12);
	JJJ(aaa, bbb, ccc, ddd, eee, ctx->X[12],  6);

	/* parallel round 2 */
	III(eee, aaa, bbb, ccc, ddd, ctx->X[ 6],  9); 
	III(ddd, eee, aaa, bbb, ccc, ctx->X[11], 13);
	III(ccc, ddd, eee, aaa, bbb, ctx->X[ 3], 15);
	III(bbb, ccc, ddd, eee, aaa, ctx->X[ 7],  7);
	III(aaa, bbb, ccc, ddd, eee, ctx->X[ 0], 12);
	III(eee, aaa, bbb, ccc, ddd, ctx->X[13],  8);
	III(ddd, eee, aaa, bbb, ccc, ctx->X[ 5],  9);
	III(ccc, ddd, eee, aaa, bbb, ctx->X[10], 11);
	III(bbb, ccc, ddd, eee, aaa, ctx->X[14],  7);
	III(aaa, bbb, ccc, ddd, eee, ctx->X[15],  7);
	III(eee, aaa, bbb, ccc, ddd, ctx->X[ 8], 12);
	III(ddd, eee, aaa, bbb, ccc, ctx->X[12],  7);
	III(ccc, ddd, eee, aaa, bbb, ctx->X[ 4],  6);
	III(bbb, ccc, ddd, eee, aaa, ctx->X[ 9], 15);
	III(aaa, bbb, ccc, ddd, eee, ctx->X[ 1], 13);
	III(eee, aaa, bbb, ccc, ddd, ctx->X[ 2], 11);

	/* parallel round 3 */
	HHH(ddd, eee, aaa, bbb, ccc, ctx->X[15],  9);
	HHH(ccc, ddd, eee, aaa, bbb, ctx->X[ 5],  7);
	HHH(bbb, ccc, ddd, eee, aaa, ctx->X[ 1], 15);
	HHH(aaa, bbb, ccc, ddd, eee, ctx->X[ 3], 11);
	HHH(eee, aaa, bbb, ccc, ddd, ctx->X[ 7],  8);
	HHH(ddd, eee, aaa, bbb, ccc, ctx->X[14],  6);
	HHH(ccc, ddd, eee, aaa, bbb, ctx->X[ 6],  6);
	HHH(bbb, ccc, ddd, eee, aaa, ctx->X[ 9], 14);
	HHH(aaa, bbb, ccc, ddd, eee, ctx->X[11], 12);
	HHH(eee, aaa, bbb, ccc, ddd, ctx->X[ 8], 13);
	HHH(ddd, eee, aaa, bbb, ccc, ctx->X[12],  5);
	HHH(ccc, ddd, eee, aaa, bbb, ctx->X[ 2], 14);
	HHH(bbb, ccc, ddd, eee, aaa, ctx->X[10], 13);
	HHH(aaa, bbb, ccc, ddd, eee, ctx->X[ 0], 13);
	HHH(eee, aaa, bbb, ccc, ddd, ctx->X[ 4],  7);
	HHH(ddd, eee, aaa, bbb, ccc, ctx->X[13],  5);

	/* parallel round 4 */   
	GGG(ccc, ddd, eee, aaa, bbb, ctx->X[ 8], 15);
	GGG(bbb, ccc, ddd, eee, aaa, ctx->X[ 6],  5);
	GGG(aaa, bbb, ccc, ddd, eee, ctx->X[ 4],  8);
	GGG(eee, aaa, bbb, ccc, ddd, ctx->X[ 1], 11);
	GGG(ddd, eee, aaa, bbb, ccc, ctx->X[ 3], 14);
	GGG(ccc, ddd, eee, aaa, bbb, ctx->X[11], 14);
	GGG(bbb, ccc, ddd, eee, aaa, ctx->X[15],  6);
	GGG(aaa, bbb, ccc, ddd, eee, ctx->X[ 0], 14);
	GGG(eee, aaa, bbb, ccc, ddd, ctx->X[ 5],  6);
	GGG(ddd, eee, aaa, bbb, ccc, ctx->X[12],  9);
	GGG(ccc, ddd, eee, aaa, bbb, ctx->X[ 2], 12);
	GGG(bbb, ccc, ddd, eee, aaa, ctx->X[13],  9);
	GGG(aaa, bbb, ccc, ddd, eee, ctx->X[ 9], 12);
	GGG(eee, aaa, bbb, ccc, ddd, ctx->X[ 7],  5);
	GGG(ddd, eee, aaa, bbb, ccc, ctx->X[10], 15);
	GGG(ccc, ddd, eee, aaa, bbb, ctx->X[14],  8);

	/* parallel round 5 */
	FFF(bbb, ccc, ddd, eee, aaa, ctx->X[12] ,  8);
	FFF(aaa, bbb, ccc, ddd, eee, ctx->X[15] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, ctx->X[10] , 12);
	FFF(ddd, eee, aaa, bbb, ccc, ctx->X[ 4] ,  9);
	FFF(ccc, ddd, eee, aaa, bbb, ctx->X[ 1] , 12);
	FFF(bbb, ccc, ddd, eee, aaa, ctx->X[ 5] ,  5);
	FFF(aaa, bbb, ccc, ddd, eee, ctx->X[ 8] , 14);
	FFF(eee, aaa, bbb, ccc, ddd, ctx->X[ 7] ,  6);
	FFF(ddd, eee, aaa, bbb, ccc, ctx->X[ 6] ,  8);
	FFF(ccc, ddd, eee, aaa, bbb, ctx->X[ 2] , 13);
	FFF(bbb, ccc, ddd, eee, aaa, ctx->X[13] ,  6);
	FFF(aaa, bbb, ccc, ddd, eee, ctx->X[14] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, ctx->X[ 0] , 15);
	FFF(ddd, eee, aaa, bbb, ccc, ctx->X[ 3] , 13);
	FFF(ccc, ddd, eee, aaa, bbb, ctx->X[ 9] , 11);
	FFF(bbb, ccc, ddd, eee, aaa, ctx->X[11] , 11);

	/* combine results */
	ddd += cc + ctx->MD[1];
	ctx->MD[1] = ctx->MD[2] + dd + eee;
	ctx->MD[2] = ctx->MD[3] + ee + aaa;
	ctx->MD[3] = ctx->MD[4] + aa + bbb;
	ctx->MD[4] = ctx->MD[0] + bb + ccc;
	ctx->MD[0] = ddd;
	ctx->len+=64;

	memset(ctx->X, 0, 16*sizeof(u32));
	return;
}

void RIPEMD160_Init(RIPEMD160_CTX *ctx)
{
	ctx->MD[0] = 0x67452301UL;
	ctx->MD[1] = 0xefcdab89UL;
	ctx->MD[2] = 0x98badcfeUL;
	ctx->MD[3] = 0x10325476UL;
	ctx->MD[4] = 0xc3d2e1f0UL;
	ctx->len=ctx->l=0;
	return;
}

void RIPEMD160_Update(RIPEMD160_CTX *ctx, const void *data, u32 size)
{
	u8 *message=(u8 *)data;
	u32 i,bytes;

	/* process message in 16-word chunks */
	for (bytes=size; bytes > 63; bytes-=64) {
		for (i=0; i<16; i++) {
			ctx->X[i] = bytes_TO_u32(message); message += 4;
		}
		compress(ctx);
	}

	/* XXX, is this correct:
	 * - I process firstly the chunks
	 * - then I add the rest to the data... ist this really okay?
	 * - Tino Reichardt <crypto@mcmilk.de>
	 */
	if (bytes+ctx->l >= 64) {
		u8 *m=ctx->D;
		memcpy(m+ctx->l, message, 64-ctx->l);
		for (i=0; i<16; i++) {
			ctx->X[i] = bytes_TO_u32(m); m += 4;
		}
		compress(ctx);
		bytes-=64-ctx->l; message+=64-ctx->l; ctx->l=0;
	}

	if (bytes > 0) {
		memcpy(ctx->D+ctx->l, message, bytes);
		ctx->l+=bytes;
	}

	return;
}

void RIPEMD160_Final(u8 *result, RIPEMD160_CTX *ctx)
{
	int i,bytes=ctx->len+ctx->l;
	u8 *message=ctx->D;

	memset(ctx->X, 0, 16 * sizeof(u32));

	for (i=0; i<(bytes&63); i++) {
		ctx->X[i>>2] ^= (u32) *message++ << (8*(i&3));
	}
	ctx->X[(bytes>>2)&15] ^= (u32)1 << (8*(bytes&3) + 7);
	if ((bytes & 63) > 55) { compress(ctx); }
	ctx->X[14] = bytes << 3;
	ctx->X[15] = bytes >> 29;
	compress(ctx);
	for (i=0; i<20; i+=4) {
		result[i]   =  ctx->MD[i>>2];
		result[i+1] = (ctx->MD[i>>2] >>  8);
		result[i+2] = (ctx->MD[i>>2] >> 16);
		result[i+3] = (ctx->MD[i>>2] >> 24);
	}

	return;
}
#undef F
#undef FF
#undef FFF
#undef G
#undef GG
#undef GGG
#undef H
#undef HH
#undef HHH
#undef I
#undef II
#undef III
#undef J
#undef JJ
#undef JJJ
#undef ROL
#undef bytes_TO_u32

void
getversion_ripemd_c()
{
	static char    *x = "$Id: ripemd.c,v 2.2 2011-12-05 15:10:07+05:30 Cprogrammer Stab mbhangui $";
	x=sccsidripemdh;
	x=sccsidtypesxh;
	x++;
}
