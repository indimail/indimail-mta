 
#include <stdio.h>
#include <string.h>
#include "uint32.h"
#include "uint64.h"
#include "siphash.h"

typedef uint64 u64;
typedef uint32 u32;
typedef unsigned char u8;

#define ROTL(x,b) (u64)( ((x) << (b)) | ( (x) >> (64 - (b))) )

#define U32TO8_LE(p, v)         \
    (p)[0] = (u8)((v)      ); (p)[1] = (u8)((v) >>  8); \
    (p)[2] = (u8)((v) >> 16); (p)[3] = (u8)((v) >> 24);

#define U64TO8_LE(p, v)         \
  U32TO8_LE((p),     (u32)((v)      ));   \
  U32TO8_LE((p) + 4, (u32)((v) >> 32));

#define U8TO64_LE(p) \
  (((u64)((p)[0])      ) | \
   ((u64)((p)[1]) <<  8) | \
   ((u64)((p)[2]) << 16) | \
   ((u64)((p)[3]) << 24) | \
   ((u64)((p)[4]) << 32) | \
   ((u64)((p)[5]) << 40) | \
   ((u64)((p)[6]) << 48) | \
   ((u64)((p)[7]) << 56))

#define SIPROUND            \
  do {              \
    x0 += x1; x1=ROTL(x1,13); x1 ^= x0; x0=ROTL(x0,32); \
    x2 += x3; x3=ROTL(x3,16); x3 ^= x2;     \
    x0 += x3; x3=ROTL(x3,21); x3 ^= x0;     \
    x2 += x1; x1=ROTL(x1,17); x1 ^= x2; x2=ROTL(x2,32); \
  } while(0)

/* SipHash-2-4 */
int siphash24( unsigned char *out, const unsigned char *in, unsigned long long inlen, const unsigned char *k )
{
  /* "somepseudorandomlygeneratedbytes" */
  u64 x0 = 0x736f6d6570736575ULL;
  u64 x1 = 0x646f72616e646f6dULL;
  u64 x2 = 0x6c7967656e657261ULL;
  u64 x3 = 0x7465646279746573ULL;
  u64 b;
  u64 k0 = U8TO64_LE( k );
  u64 k1 = U8TO64_LE( k + 8 );
  u64 m;
  const u8 *end = in + inlen - ( inlen % sizeof( u64 ) );
  const int left = inlen & 7;
  b = ( ( u64 )inlen ) << 56;
  x3 ^= k1;
  x2 ^= k0;
  x1 ^= k1;
  x0 ^= k0;

  for ( ; in != end; in += 8 )
  {
    m = U8TO64_LE( in );
    x3 ^= m;
    SIPROUND;
    SIPROUND;
    x0 ^= m;
  }

  switch( left )
  {
  case 7: b |= ( ( u64 )in[ 6] )  << 48;

  case 6: b |= ( ( u64 )in[ 5] )  << 40;

  case 5: b |= ( ( u64 )in[ 4] )  << 32;

  case 4: b |= ( ( u64 )in[ 3] )  << 24;

  case 3: b |= ( ( u64 )in[ 2] )  << 16;

  case 2: b |= ( ( u64 )in[ 1] )  <<  8;

  case 1: b |= ( ( u64 )in[ 0] ); break;

  case 0: break;
  }

  x3 ^= b;
  SIPROUND;
  SIPROUND;
  x0 ^= b;
  x2 ^= 0xff;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  SIPROUND;
  b = x0 ^ x1 ^ x2  ^ x3;
  U64TO8_LE( out, b );
  return 0;
}
