/*
** Copyright 2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#define	SHA1_INTERNAL
#include	"sha1.h"

#include	<string.h>
#include	<stdlib.h>


#define ROTR(x,n) ((SHA256_WORD)(((SHA256_WORD)(x) >> (n))|((x) << (32-(n)))))

#define ROTL(x,n) ((SHA256_WORD)(((SHA256_WORD)(x) << (n))|((x) >> (32-(n)))))


#define CH(x,y,z) ((SHA256_WORD)(((x) & (y)) ^ ((~(x))&(z))))
#define MAJ(x,y,z) ((SHA256_WORD)(((x)&(y))^((x)&(z))^((y)&(z))))

#define SUM0(x) ((SHA256_WORD)(ROTR((x),2)^ROTR((x),13)^ROTR((x),22)))
#define SUM1(x) ((SHA256_WORD)(ROTR((x),6)^ROTR((x),11)^ROTR((x),25)))

#define TH0(x) ((SHA256_WORD)(ROTR((x),7)^ROTR((x),18)^((SHA256_WORD)(x)>>3)))
#define TH1(x) ((SHA256_WORD)(ROTR((x),17)^ROTR((x),19)^((SHA256_WORD)(x)>>10)))

static const SHA256_WORD K[64]=
	{0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	 0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	 0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	 0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	 0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	 0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	 0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	 0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

void sha256_context_init(struct SHA256_CONTEXT *c)
{
	if (sizeof(SHA256_WORD) != 4)
		abort();

	c->H[0] = 0x6A09E667;
	c->H[1] = 0xBB67AE85;
	c->H[2] = 0x3C6EF372;
	c->H[3] = 0xA54FF53A;
	c->H[4] = 0x510E527F;
	c->H[5] = 0x9B05688C;
	c->H[6] = 0x1F83D9AB;
	c->H[7] = 0x5BE0CD19;
	c->blk_ptr=0;
}

void sha256_context_hash(struct SHA256_CONTEXT *cc,
			 const unsigned char blk[SHA256_BLOCK_SIZE])
{
	SHA256_WORD W[64];
	unsigned i, t;
	SHA256_WORD a,b,c,d,e,f,g,h;

	for (i=t=0; t<16; t++)
	{
		SHA256_WORD x=blk[i]; i++;

		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		W[t]=(x << 8) | blk[i]; i++;
	}

	for (t=16; t<64; t++)
		W[t]= TH1(W[t-2]) + W[t-7] + TH0(W[t-15]) + W[t-16];

	a=cc->H[0];
	b=cc->H[1];
	c=cc->H[2];
	d=cc->H[3];
	e=cc->H[4];
	f=cc->H[5];
	g=cc->H[6];
	h=cc->H[7];

	for (t=0; t<64; t++)
	{
		SHA256_WORD T1=h + SUM1(e) + CH(e,f,g) + K[t] + W[t];
		SHA256_WORD T2=SUM0(a)+MAJ(a,b,c);
		h=g;
		g=f;
		f=e;
		e=d+T1;
		d=c;
		c=b;
		b=a;
		a=T1+T2;
	}

	cc->H[0] += a;
	cc->H[1] += b;
	cc->H[2] += c;
	cc->H[3] += d;
	cc->H[4] += e;
	cc->H[5] += f;
	cc->H[6] += g;
	cc->H[7] += h;
}

void sha256_context_hashstream(struct SHA256_CONTEXT *c, const void *p, unsigned l)
{
const unsigned char *cp=(const unsigned char *)p;
unsigned ll;

	while (l)
	{
		if (c->blk_ptr == 0 && l >= SHA256_BLOCK_SIZE)
		{
			sha256_context_hash(c, cp);
			cp += SHA256_BLOCK_SIZE;
			l -= SHA256_BLOCK_SIZE;
			continue;
		}

		ll=l;
		if (ll > SHA256_BLOCK_SIZE - c->blk_ptr)
			ll=SHA256_BLOCK_SIZE - c->blk_ptr;
		memcpy(c->blk + c->blk_ptr, cp, ll);
		c->blk_ptr += ll;
		cp += ll;
		l -= ll;
		if (c->blk_ptr >= SHA256_BLOCK_SIZE)
		{
			sha256_context_hash(c, c->blk);
			c->blk_ptr=0;
		}
	}
}

void sha256_context_endstream(struct SHA256_CONTEXT *c, unsigned long l)
{
	unsigned char buf[8];
	static const unsigned char zero[SHA256_BLOCK_SIZE-8];

	buf[0]=0x80;
	sha256_context_hashstream(c, &buf, 1);
	while (c->blk_ptr != SHA256_BLOCK_SIZE-8)
	{
		if (c->blk_ptr > SHA256_BLOCK_SIZE-8)
		{
			sha256_context_hashstream(c, zero,
				SHA256_BLOCK_SIZE - c->blk_ptr);
			continue;
		}
		sha256_context_hashstream(c, zero,
			SHA256_BLOCK_SIZE-8-c->blk_ptr);
	}

	l *= 8;
	buf[7] = l;
	buf[6] = (l >>= 8);
	buf[5] = (l >>= 8);
	buf[4] = (l >> 8);
	buf[3]=buf[2]=buf[1]=buf[0]=0;

	sha256_context_hashstream(c, buf, 8);
}

void sha256_context_digest(struct SHA256_CONTEXT *c, SHA256_DIGEST d)
{
	unsigned char *dp=d + SHA256_DIGEST_SIZE;
	unsigned i;

	for ( i=8; i; )
	{
		SHA256_WORD	w=c->H[--i];

		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w;
	}
}

void sha256_context_restore(struct SHA256_CONTEXT *c, const SHA256_DIGEST d)
{
	const unsigned char *dp=d;
	unsigned i;

	for (i=0; i<8; i++)
	{
		SHA256_WORD	w= *dp++;

		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		c->H[i]=w;
	}
	c->blk_ptr=0;
}

void sha256_digest(const void *msg, unsigned len, SHA256_DIGEST d)
{
	struct SHA256_CONTEXT c;

	sha256_context_init( &c );
	sha256_context_hashstream(&c, msg, len);
	sha256_context_endstream(&c, len);
	sha256_context_digest( &c, d );
}
