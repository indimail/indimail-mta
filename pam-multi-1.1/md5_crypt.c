/*
 * $Id: md5_crypt.c,v 1.3 2018-09-12 12:59:03+05:30 Cprogrammer Exp mbhangui $
 *
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dknet.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * Origin: Id: crypt.c,v 1.3 1995/05/30 05:42:22 rgrimes Exp
 *
 */

#include <string.h>
#include <stdlib.h>

typedef unsigned int uint32;

struct MD5Context {
	uint32          buf[4];
	uint32          bits[2];
	unsigned char   in[64];
};

void            MD5Init(struct MD5Context *);
void            MD5Update(struct MD5Context *, unsigned const char *, unsigned);
void            MD5Final(unsigned char digest[16], struct MD5Context *);
void            MD5Transform(uint32 buf[4], uint32 const in[16]);

#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
static void     byteReverse(unsigned char *buf, unsigned longs);

#ifndef ASM_MD5
/*
 * Note: this code is harmless on little-endian machines.
 */
static void
byteReverse(unsigned char *buf, unsigned longs)
{
	uint32          t;
	do {
		t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 | ((unsigned) buf[1] << 8 | buf[0]);
		*(uint32 *) buf = t;
		buf += 4;
	} while (--longs);
}
#endif /*- #ifndef ASM_MD5 */
#endif /*- #ifndef HIGHFIRST */


/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void
MD5Init(struct MD5Context *ctx)
{
	ctx->buf[0] = 0x67452301U;
	ctx->buf[1] = 0xefcdab89U;
	ctx->buf[2] = 0x98badcfeU;
	ctx->buf[3] = 0x10325476U;

	ctx->bits[0] = 0;
	ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void
MD5Update(struct MD5Context *ctx, unsigned const char *buf, unsigned len)
{
	uint32          t;

	/*- Update bitcount */

	t = ctx->bits[0];
	if ((ctx->bits[0] = t + ((uint32) len << 3)) < t)
		ctx->bits[1]++;	/* Carry from low to high */
	ctx->bits[1] += len >> 29;
	t = (t >> 3) & 0x3f; /* Bytes already in shsInfo->data */

	/*- Handle any leading odd-sized chunks */
	if (t) {
		unsigned char  *p = (unsigned char *) ctx->in + t;

		t = 64 - t;
		if (len < t) {
			memcpy(p, buf, len);
			return;
		}
		memcpy(p, buf, t);
		byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (uint32 *) ctx->in);
		buf += t;
		len -= t;
	}
	/*- Process data in 64-byte chunks */
	while (len >= 64) {
		memcpy(ctx->in, buf, 64);
		byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (uint32 *) ctx->in);
		buf += 64;
		len -= 64;
	}

	/*- Handle any remaining bytes of data.  */
	memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern 
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void
MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
	unsigned        count;
	unsigned char  *p;

	/*- Compute number of bytes mod 64 */
	count = (ctx->bits[0] >> 3) & 0x3F;

	/*-
	 * Set the first char of padding to 0x80.
	 * This is safe since there is always at least one byte free 
	 */
	p = ctx->in + count;
	*p++ = 0x80;

	/*- Bytes of padding needed to make 64 bytes */
	count = 64 - 1 - count;

	/*- Pad out to 56 mod 64 */
	if (count < 8) {
		/*- Two lots of padding:  Pad the first block to 64 bytes */
		memset(p, 0, count);
		byteReverse(ctx->in, 16);
		MD5Transform(ctx->buf, (uint32 *) ctx->in);

		/*- Now fill the next block with 56 bytes */
		memset(ctx->in, 0, 56);
	} else /*- Pad block to 56 bytes */
		memset(p, 0, count - 8);
	byteReverse(ctx->in, 14);

	/*- Append length in bits and transform */
	((uint32 *) ctx->in)[14] = ctx->bits[0];
	((uint32 *) ctx->in)[15] = ctx->bits[1];

	MD5Transform(ctx->buf, (uint32 *) ctx->in);
	byteReverse((unsigned char *) ctx->buf, 4);
	memcpy(digest, ctx->buf, 16);
	memset((char *) ctx, 0, sizeof (ctx));	/* In case it's sensitive */
}

#ifndef ASM_MD5

/*
 * The four core functions - F1 is optimized somewhat 
 */

/*
 * #define F1(x, y, z) (x & y | ~x & z) 
 */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/*
 * This is the central step in the MD5 algorithm. 
 */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void
MD5Transform(uint32 buf[4], uint32 const in[16])
{
	register uint32 a, b, c, d;

	a = buf[0];
	b = buf[1];
	c = buf[2];
	d = buf[3];

	MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478U, 7);
	MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756U, 12);
	MD5STEP(F1, c, d, a, b, in[2] + 0x242070dbU, 17);
	MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceeeU, 22);
	MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0fafU, 7);
	MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62aU, 12);
	MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613U, 17);
	MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501U, 22);
	MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8U, 7);
	MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7afU, 12);
	MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1U, 17);
	MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7beU, 22);
	MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122U, 7);
	MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193U, 12);
	MD5STEP(F1, c, d, a, b, in[14] + 0xa679438eU, 17);
	MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821U, 22);

	MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562U, 5);
	MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340U, 9);
	MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51U, 14);
	MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aaU, 20);
	MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105dU, 5);
	MD5STEP(F2, d, a, b, c, in[10] + 0x02441453U, 9);
	MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681U, 14);
	MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8U, 20);
	MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6U, 5);
	MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6U, 9);
	MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87U, 14);
	MD5STEP(F2, b, c, d, a, in[8] + 0x455a14edU, 20);
	MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905U, 5);
	MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8U, 9);
	MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9U, 14);
	MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8aU, 20);

	MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942U, 4);
	MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681U, 11);
	MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122U, 16);
	MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380cU, 23);
	MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44U, 4);
	MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9U, 11);
	MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60U, 16);
	MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70U, 23);
	MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6U, 4);
	MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127faU, 11);
	MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085U, 16);
	MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05U, 23);
	MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039U, 4);
	MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5U, 11);
	MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8U, 16);
	MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665U, 23);

	MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244U, 6);
	MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97U, 10);
	MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7U, 15);
	MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039U, 21);
	MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3U, 6);
	MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92U, 10);
	MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47dU, 15);
	MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1U, 21);
	MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4fU, 6);
	MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0U, 10);
	MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314U, 15);
	MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1U, 21);
	MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82U, 6);
	MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235U, 10);
	MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bbU, 15);
	MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391U, 21);

	buf[0] += a;
	buf[1] += b;
	buf[2] += c;
	buf[3] += d;
}

#endif

/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */

typedef struct MD5Context MD5_CTX;

static unsigned char itoa64[] =	/* 0 ... 63 => ascii - 64 */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static void
to64(char *s, unsigned long v, int n)
{
	while (--n >= 0) {
		*s++ = itoa64[v & 0x3f];
		v >>= 6;
	}
}

/*
 * UNIX password
 *
 * Use MD5 for what it is best at...
 */

char           *
md5_crypt(const char *pw, const char *salt)
{
	const char     *magic = "$1$";
	/*
	 * This string is magic for this algorithm.  Having
	 * it this way, we can get get better later on 
	 */
	char           *passwd, *p;
	const char     *sp, *ep;
	unsigned char   final[16];
	int             sl, pl, i, j;
	MD5_CTX         ctx, ctx1;
	unsigned long   l;

	/*- Refine the Salt first */
	sp = salt;

	/*
	 * TODO: now that we're using malloc'ed memory, get rid of the
	 * strange constant buffer size. 
	 */
	passwd = malloc(120);

	/*- If it starts with the magic string, then skip that */
	if (!strncmp(sp, magic, strlen(magic)))
		sp += strlen(magic);

	/*- It stops at the first '$', max 8 chars */
	for (ep = sp; *ep && *ep != '$' && ep < (sp + 8); ep++)
		continue;

	/*- get the length of the true salt */
	sl = ep - sp;

	MD5Init(&ctx);

	/*- The password first, since that is what is most unknown */
	MD5Update(&ctx, (unsigned const char *) pw, strlen(pw));

	/*- Then our magic string */
	MD5Update(&ctx, (unsigned const char *) magic, strlen(magic));

	/*- Then the raw salt */
	MD5Update(&ctx, (unsigned const char *) sp, sl);

	/*- Then just as many characters of the MD5(pw,salt,pw) */
	MD5Init(&ctx1);
	MD5Update(&ctx1, (unsigned const char *) pw, strlen(pw));
	MD5Update(&ctx1, (unsigned const char *) sp, sl);
	MD5Update(&ctx1, (unsigned const char *) pw, strlen(pw));
	MD5Final(final, &ctx1);
	for (pl = strlen(pw); pl > 0; pl -= 16)
		MD5Update(&ctx, (unsigned const char *) final, pl > 16 ? 16 : pl);

	/*- Don't leave anything around in vm they could use.  */
	memset(final, 0, sizeof final);

	/*- Then something really weird...  */
	for (j = 0, i = strlen(pw); i; i >>= 1)
		if (i & 1)
			MD5Update(&ctx, (unsigned const char *) final + j, 1);
		else
			MD5Update(&ctx, (unsigned const char *) pw + j, 1);

	/*- Now make the output string */
	strcpy(passwd, magic);
	strncat(passwd, sp, sl);
	strcat(passwd, "$");
	MD5Final(final, &ctx);

	/*-
	 * and now, just to make sure things don't run too fast
	 * On a 60 Mhz Pentium this takes 34 msec, so you would
	 * need 30 seconds to build a 1000 entry dictionary...
	 */
	for (i = 0; i < 1000; i++) {
		MD5Init(&ctx1);
		if (i & 1)
			MD5Update(&ctx1, (unsigned const char *) pw, strlen(pw));
		else
			MD5Update(&ctx1, (unsigned const char *) final, 16);

		if (i % 3)
			MD5Update(&ctx1, (unsigned const char *) sp, sl);

		if (i % 7)
			MD5Update(&ctx1, (unsigned const char *) pw, strlen(pw));

		if (i & 1)
			MD5Update(&ctx1, (unsigned const char *) final, 16);
		else
			MD5Update(&ctx1, (unsigned const char *) pw, strlen(pw));
		MD5Final(final, &ctx1);
	}

	p = passwd + strlen(passwd);

	l = (final[0] << 16) | (final[6] << 8) | final[12];
	to64(p, l, 4);
	p += 4;
	l = (final[1] << 16) | (final[7] << 8) | final[13];
	to64(p, l, 4);
	p += 4;
	l = (final[2] << 16) | (final[8] << 8) | final[14];
	to64(p, l, 4);
	p += 4;
	l = (final[3] << 16) | (final[9] << 8) | final[15];
	to64(p, l, 4);
	p += 4;
	l = (final[4] << 16) | (final[10] << 8) | final[5];
	to64(p, l, 4);
	p += 4;
	l = final[11];
	to64(p, l, 2);
	p += 2;
	*p = '\0';

	/*-
	 * Don't leave anything around in vm they could use. 
	 */
	memset(final, 0, sizeof final);

	return passwd;
}
