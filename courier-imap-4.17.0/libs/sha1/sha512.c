/*
** Copyright 2008 Double Precision, Inc.
** See COPYING for distribution information.
*/

#define	SHA1_INTERNAL
#include	"sha1.h"

#include	<string.h>
#include	<stdlib.h>


#define ROTR(x,n) ((SHA512_WORD)(((SHA512_WORD)(x) >> (n))|((x) << (64-(n)))))

#define ROTL(x,n) ((SHA512_WORD)(((SHA512_WORD)(x) << (n))|((x) >> (64-(n)))))


#define CH(x,y,z) ((SHA512_WORD)(((x) & (y)) ^ ((~(x))&(z))))
#define MAJ(x,y,z) ((SHA512_WORD)(((x)&(y))^((x)&(z))^((y)&(z))))

#define SUM0(x) ((SHA512_WORD)(ROTR((x),28)^ROTR((x),34)^ROTR((x),39)))
#define SUM1(x) ((SHA512_WORD)(ROTR((x),14)^ROTR((x),18)^ROTR((x),41)))

#define TH0(x) ((SHA512_WORD)(ROTR((x),1)^ROTR((x),8)^((SHA512_WORD)(x)>>7)))
#define TH1(x) ((SHA512_WORD)(ROTR((x),19)^ROTR((x),61)^((SHA512_WORD)(x)>>6)))

static const SHA512_WORD K[80]={
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL};

void sha512_context_init(struct SHA512_CONTEXT *c)
{
	if (sizeof(SHA512_WORD) != 8)
		abort();

	c->H[0] = 0x6a09e667f3bcc908ULL;
	c->H[1] = 0xbb67ae8584caa73bULL;
	c->H[2] = 0x3c6ef372fe94f82bULL;
	c->H[3] = 0xa54ff53a5f1d36f1ULL;
	c->H[4] = 0x510e527fade682d1ULL;
	c->H[5] = 0x9b05688c2b3e6c1fULL;
	c->H[6] = 0x1f83d9abfb41bd6bULL;
	c->H[7] = 0x5be0cd19137e2179ULL;

	c->blk_ptr=0;
}

void sha512_context_hash(struct SHA512_CONTEXT *cc,
			 const unsigned char blk[SHA512_BLOCK_SIZE])
{
	SHA512_WORD W[80];
	unsigned i, t;
	SHA512_WORD a,b,c,d,e,f,g,h;

	for (i=t=0; t<16; t++)
	{
		SHA512_WORD x=blk[i]; i++;

		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		x=(x << 8) | blk[i]; i++;
		W[t]=(x << 8) | blk[i]; i++;
	}

	for (t=16; t<80; t++)
		W[t]= TH1(W[t-2]) + W[t-7] + TH0(W[t-15]) + W[t-16];

	a=cc->H[0];
	b=cc->H[1];
	c=cc->H[2];
	d=cc->H[3];
	e=cc->H[4];
	f=cc->H[5];
	g=cc->H[6];
	h=cc->H[7];

	for (t=0; t<80; t++)
	{
		SHA512_WORD T1=h + SUM1(e) + CH(e,f,g) + K[t] + W[t];
		SHA512_WORD T2=SUM0(a)+MAJ(a,b,c);
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

void sha512_context_hashstream(struct SHA512_CONTEXT *c, const void *p, unsigned l)
{
	const unsigned char *cp=(const unsigned char *)p;
	unsigned ll;

	while (l)
	{
		if (c->blk_ptr == 0 && l >= SHA512_BLOCK_SIZE)
		{
			sha512_context_hash(c, cp);
			cp += SHA512_BLOCK_SIZE;
			l -= SHA512_BLOCK_SIZE;
			continue;
		}

		ll=l;
		if (ll > SHA512_BLOCK_SIZE - c->blk_ptr)
			ll=SHA512_BLOCK_SIZE - c->blk_ptr;
		memcpy(c->blk + c->blk_ptr, cp, ll);
		c->blk_ptr += ll;
		cp += ll;
		l -= ll;
		if (c->blk_ptr >= SHA512_BLOCK_SIZE)
		{
			sha512_context_hash(c, c->blk);
			c->blk_ptr=0;
		}
	}
}

void sha512_context_endstream(struct SHA512_CONTEXT *c, SHA512_WORD l)
{
	unsigned char buf[16];
	size_t i;
	static const unsigned char zero[SHA512_BLOCK_SIZE-8];

	buf[0]=0x80;
	sha512_context_hashstream(c, &buf, 1);
	while (c->blk_ptr != SHA512_BLOCK_SIZE-16)
	{
		if (c->blk_ptr > SHA512_BLOCK_SIZE-16)
		{
			sha512_context_hashstream(c, zero,
				SHA512_BLOCK_SIZE - c->blk_ptr);
			continue;
		}
		sha512_context_hashstream(c, zero,
			SHA512_BLOCK_SIZE-16-c->blk_ptr);
	}

	l *= 8;

	for (i=0; i<16; i++)
	{
		buf[15-i]=l;
		l >>= 8;
	}

	sha512_context_hashstream(c, buf, sizeof(buf));
}

void sha512_context_digest(struct SHA512_CONTEXT *c, SHA512_DIGEST d)
{
	unsigned char *dp=d + SHA512_DIGEST_SIZE;
	unsigned i;

	for ( i=8; i; )
	{
		SHA512_WORD	w=c->H[--i];

		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w; w >>= 8;
		*--dp=w;
	}
}

void sha512_context_restore(struct SHA512_CONTEXT *c, const SHA512_DIGEST d)
{
	const unsigned char *dp=d;
	unsigned i;

	for (i=0; i<8; i++)
	{
		SHA512_WORD	w= *dp++;

		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		w=(w << 8) | *dp++;
		c->H[i]=w;
	}
	c->blk_ptr=0;
}

void sha512_digest(const void *msg, unsigned len, SHA512_DIGEST d)
{
	struct SHA512_CONTEXT c;

	sha512_context_init( &c );
	sha512_context_hashstream(&c, msg, len);
	sha512_context_endstream(&c, len);
	sha512_context_digest( &c, d );
}
