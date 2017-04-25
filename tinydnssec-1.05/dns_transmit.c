#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "socket.h"
#include "alloc.h"
#include "error.h"
#include "byte.h"
#include "uint16.h"
#ifdef DNSCURVE
#include "uint64.h"
#include "case.h"
#endif
#include "dns.h"
#include "ip6.h"

#ifdef DNSCURVE
#include "nacl/crypto_box_curve25519xsalsa20poly1305.h"

#define crypto_box_afternm crypto_box_curve25519xsalsa20poly1305_afternm
#define crypto_box_open_afternm crypto_box_curve25519xsalsa20poly1305_open_afternm
#endif

#ifdef DNSCURVE
/* XXX: put in uint64_pack.c */
static void uint64_pack(char s[8],uint64 u)
{
  s[0] = u & 255; u >>= 8;
  s[1] = u & 255; u >>= 8;
  s[2] = u & 255; u >>= 8;
  s[3] = u & 255; u >>= 8;
  s[4] = u & 255; u >>= 8;
  s[5] = u & 255; u >>= 8;
  s[6] = u & 255; u >>= 8;
  s[7] = u & 255;
}

/* XXX: put in dns_nonce.c */
static void dns_nonce(char nonce[12])
{
  /* XXX: use nanoseconds */
  static uint64 x;

  uint64_pack(nonce,++x);
  nonce[8] = dns_random(256);
  nonce[9] = dns_random(256);
  nonce[10] = dns_random(256);
  nonce[11] = dns_random(256);
}


/* XXX: put in base32.c */
static const char base32_digits[32] = "0123456789bcdfghjklmnpqrstuvwxyz";

static unsigned int base32_bytessize(unsigned int len)
{
  len = (8 * len + 4) / 5;
  return len + (len + 49) / 50;
}

static void base32_encodebytes(char *out,const char *in,unsigned int len)
{
  unsigned int i, x, v, vbits;

  x = v = vbits = 0;
  for (i = 0;i < len;++i) {
    v |= ((unsigned int) (unsigned char) in[i]) << vbits;
    vbits += 8;
    do {
      out[++x] = base32_digits[v & 31];
      v >>= 5;
      vbits -= 5;
      if (x == 50) {
        *out = x;
        out += 1 + x;
        x = 0;
      }
    } while (vbits >= 5);
  }

  if (vbits) out[++x] = base32_digits[v & 31];
  if (x) *out = x;
}

static void base32_encodekey(char *out,const char *key)
{
  unsigned int i, v, vbits;

  byte_copy(out,4,"\66x1a");
  out += 4;

  v = vbits = 0;
  for (i = 0;i < 32;++i) {
    v |= ((unsigned int) (unsigned char) key[i]) << vbits;
    vbits += 8;
    do {
      *out++ = base32_digits[v & 31];
      v >>= 5;
      vbits -= 5;
    } while (vbits >= 5);
  }
}


static void makebasequery(struct dns_transmit *d,char *query)
{
  unsigned int len;

  len = dns_domain_length(d->name);

  byte_copy(query,2,d->nonce + 8);
  byte_copy(query + 2,10,d->flagrecursive ? "\1\0\0\1\0\0\0\0\0\0" : "\0\0\0\1\0\0\0\0\0\0gcc-bug-workaround");
  byte_copy(query + 12,len,d->name);
  byte_copy(query + 12 + len,2,d->qtype);
  byte_copy(query + 14 + len,2,DNS_C_IN);
}

static void prepquery(struct dns_transmit *d)
{
  unsigned int len;
  char nonce[24];
  const char *key;
  unsigned int m;
  unsigned int suffixlen;

  dns_nonce(d->nonce);

  if (!d->keys) {
    byte_copy(d->query + 2,2,d->nonce + 8);
    return;
  }

  len = dns_domain_length(d->name);

  byte_copy(nonce,12,d->nonce);
  byte_zero(nonce + 12,12);
  key = d->keys + 32 * d->curserver;

  byte_zero(d->query,32);
  makebasequery(d,d->query + 32);
  crypto_box_afternm((unsigned char *) d->query,(const unsigned char *) d->query,len + 48,(const unsigned char *) nonce,(const unsigned char *) key);

  if (!d->suffix) {
    byte_copyr(d->query + 54,len + 32,d->query + 16);
    uint16_pack_big(d->query,len + 84);
    byte_copy(d->query + 2,8,"Q6fnvWj8");
    byte_copy(d->query + 10,32,d->pubkey);
    byte_copy(d->query + 42,12,nonce);
    return;
  }

  byte_copyr(d->query + d->querylen - len - 32,len + 32,d->query + 16);
  byte_copy(d->query + d->querylen - len - 44,12,nonce);

  suffixlen = dns_domain_length(d->suffix);
  m = base32_bytessize(len + 44);

  uint16_pack_big(d->query,d->querylen - 2);
  d->query[2] = dns_random(256);
  d->query[3] = dns_random(256);
  byte_copy(d->query + 4,10,"\0\0\0\1\0\0\0\0\0\0");
  base32_encodebytes(d->query + 14,d->query + d->querylen - len - 44,len + 44);
  base32_encodekey(d->query + 14 + m,d->pubkey);
  byte_copy(d->query + 69 + m,suffixlen,d->suffix);
  byte_copy(d->query + 69 + m + suffixlen,4,DNS_T_TXT DNS_C_IN);
}

static int uncurve(const struct dns_transmit *d,char *buf,unsigned int *lenp)
{
  const char *key;
  char nonce[24];
  unsigned int len;
  char out[16];
  unsigned int pos;
  uint16 datalen;
  unsigned int i;
  unsigned int j;
  char ch;
  unsigned int txtlen;
  unsigned int namelen;

  if (!d->keys) return 0;

  key = d->keys + 32 * d->curserver;
  len = *lenp;

  if (!d->suffix) {
    if (len < 48) return 1;
    if (byte_diff(buf,8,"R6fnvWJ8")) return 1;
    if (byte_diff(buf + 8,12,d->nonce)) return 1;
    byte_copy(nonce,24,buf + 8);
    byte_zero(buf + 16,16);
    if (crypto_box_open_afternm((unsigned char *) buf + 16,(const unsigned char *) buf + 16,len - 16,(const unsigned char *) nonce,(const unsigned char *) key)) return 1;
    byte_copy(buf,len - 48,buf + 48);
    *lenp = len - 48;
    return 0;
  }

  /* XXX: be more leniant? */

  pos = dns_packet_copy(buf,len,0,out,12); if (!pos) return 1;
  if (byte_diff(out,2,d->query + 2)) return 1;
  if (byte_diff(out + 2,10,"\204\0\0\1\0\1\0\0\0\0")) return 1;

  /* query name might be >255 bytes, so can't use dns_packet_getname */
  namelen = dns_domain_length(d->query + 14);
  if (namelen > len - pos) return 1;
  if (case_diffb(buf + pos,namelen,d->query + 14)) return 1;
  pos += namelen;

  pos = dns_packet_copy(buf,len,pos,out,16); if (!pos) return 1;
  if (byte_diff(out,14,"\0\20\0\1\300\14\0\20\0\1\0\0\0\0")) return 1;
  uint16_unpack_big(out + 14,&datalen);
  if (datalen > len - pos) return 1;

  j = 4;
  txtlen = 0;
  for (i = 0;i < datalen;++i) {
    ch = buf[pos + i];
    if (!txtlen)
      txtlen = (unsigned char) ch;
    else {
      --txtlen;
      buf[j++] = ch;
    }
  }
  if (txtlen) return 1;

  if (j < 32) return 1;
  byte_copy(nonce,12,d->nonce);
  byte_copy(nonce + 12,12,buf + 4);
  byte_zero(buf,16);
  if (crypto_box_open_afternm((unsigned char *) buf,(const unsigned char *) buf,j,(const unsigned char *) nonce,(const unsigned char *) key)) return 1;
  byte_copy(buf,j - 32,buf + 32);
  *lenp = j - 32;
  return 0;
}
#endif

static int serverwantstcp(const char *buf,unsigned int len)
{
  char out[12];

  if (!dns_packet_copy(buf,len,0,out,12)) return 1;
  if (out[2] & 2) return 1;
  return 0;
}

static int serverfailed(const char *buf,unsigned int len)
{
  char out[12];
  unsigned int rcode;

  if (!dns_packet_copy(buf,len,0,out,12)) return 1;
  rcode = out[3];
  rcode &= 15;
  if (rcode && (rcode != 3)) { errno = error_again; return 1; }
  return 0;
}

static int irrelevant(const struct dns_transmit *d,const char *buf,unsigned int len)
{
  char out[12];
  char *dn;
  unsigned int pos;

  pos = dns_packet_copy(buf,len,0,out,12); if (!pos) return 1;
#ifdef DNSCURVE
  if (byte_diff(out,2,d->nonce + 8)) return 1;
#else
  if (byte_diff(out,2,d->query + 2)) return 1;
#endif
  if (out[4] != 0) return 1;
  if (out[5] != 1) return 1;

  dn = 0;
  pos = dns_packet_getname(buf,len,pos,&dn); if (!pos) return 1;
#ifdef DNSCURVE
  if (!dns_domain_equal(dn,d->name)) { alloc_free(dn); return 1; }
#else
  if (!dns_domain_equal(dn,d->query + 14)) { alloc_free(dn); return 1; }
#endif
  alloc_free(dn);

  pos = dns_packet_copy(buf,len,pos,out,4); if (!pos) return 1;
  if (byte_diff(out,2,d->qtype)) return 1;
  if (byte_diff(out + 2,2,DNS_C_IN)) return 1;

  return 0;
}

static void packetfree(struct dns_transmit *d)
{
  if (!d->packet) return;
  alloc_free(d->packet);
  d->packet = 0;
}

static void queryfree(struct dns_transmit *d)
{
  if (!d->query) return;
  alloc_free(d->query);
  d->query = 0;
}

static void socketfree(struct dns_transmit *d)
{
  if (!d->s1) return;
  close(d->s1 - 1);
  d->s1 = 0;
}

void dns_transmit_free(struct dns_transmit *d)
{
  queryfree(d);
  socketfree(d);
  packetfree(d);
}

static int randombind(struct dns_transmit *d)
{
  int j;

  for (j = 0;j < 10;++j)
    if (socket_bind6(d->s1 - 1,d->localip,1025 + dns_random(64510),d->scope_id) == 0)
      return 0;
  if (socket_bind6(d->s1 - 1,d->localip,0,d->scope_id) == 0)
    return 0;
  return -1;
}

static const int timeouts[4] = { 1, 3, 11, 45 };

static int thisudp(struct dns_transmit *d)
{
  const char *ip;

  socketfree(d);

  while (d->udploop < 4) {
    for (;d->curserver < 16;++d->curserver) {
      ip = d->servers + 16 * d->curserver;
      if (byte_diff(ip,16,V6any)) {
#ifdef DNSCURVE
        prepquery(d);
#else
        d->query[2] = dns_random(256);
        d->query[3] = dns_random(256);
#endif
  
        d->s1 = 1 + socket_udp6();
        if (!d->s1) { dns_transmit_free(d); return -1; }
        if (randombind(d) == -1) { dns_transmit_free(d); return -1; }

        if (socket_connect6(d->s1 - 1,ip,53,d->scope_id) == 0)
          if (send(d->s1 - 1,d->query + 2,d->querylen - 2,0) == d->querylen - 2) {
            struct taia now;
            taia_now(&now);
            taia_uint(&d->deadline,timeouts[d->udploop]);
            taia_add(&d->deadline,&d->deadline,&now);
            d->tcpstate = 0;
            return 0;
          }
  
        socketfree(d);
      }
    }

    ++d->udploop;
    d->curserver = 0;
  }

  dns_transmit_free(d); return -1;
}

static int firstudp(struct dns_transmit *d)
{
  d->curserver = 0;
  return thisudp(d);
}

static int nextudp(struct dns_transmit *d)
{
  ++d->curserver;
  return thisudp(d);
}

static int thistcp(struct dns_transmit *d)
{
  struct taia now;
  const char *ip;

  socketfree(d);
  packetfree(d);

  for (;d->curserver < 16;++d->curserver) {
    ip = d->servers + 16 * d->curserver;
    if (byte_diff(ip,16,V6any)) {
#ifdef DNSCURVE
      prepquery(d);
#else
      d->query[2] = dns_random(256);
      d->query[3] = dns_random(256);
#endif

      d->s1 = 1 + socket_tcp6();
      if (!d->s1) { dns_transmit_free(d); return -1; }
      if (randombind(d) == -1) { dns_transmit_free(d); return -1; }
  
      taia_now(&now);
      taia_uint(&d->deadline,10);
      taia_add(&d->deadline,&d->deadline,&now);
      if (socket_connect6(d->s1 - 1,ip,53,d->scope_id) == 0) {
        d->pos = 0;
        d->tcpstate = 2;
        return 0;
      }
      if ((errno == error_inprogress) || (errno == error_wouldblock)) {
        d->tcpstate = 1;
        return 0;
      }
  
      socketfree(d);
    }
  }

  dns_transmit_free(d); return -1;
}

static int firsttcp(struct dns_transmit *d)
{
  d->curserver = 0;
  return thistcp(d);
}

static int nexttcp(struct dns_transmit *d)
{
  ++d->curserver;
  return thistcp(d);
}

#ifdef DNSCURVE
int dns_transmit_start(struct dns_transmit *d,const char servers[256],int flagrecursive,const char *q,const char qtype[2],const char localip[16])
{
  return dns_transmit_start2(d,servers,flagrecursive,q,qtype,localip,0,0,0);
}
#endif

#ifdef DNSCURVE
int dns_transmit_start2(struct dns_transmit *d,const char servers[64],int flagrecursive,const char *q,const char qtype[2],const char localip[4],const char keys[512],const char pubkey[32],const char *suffix)
#else
int dns_transmit_start(struct dns_transmit *d,const char servers[256],int flagrecursive,const char *q,const char qtype[2],const char localip[16])
#endif
{
  unsigned int len;
#ifdef DNSCURVE
  unsigned int suffixlen;
  unsigned int m;
#endif

  dns_transmit_free(d);
  errno = error_io;

  len = dns_domain_length(q);
#ifdef DNSCURVE
  if (!keys)
    d->querylen = len + 18;
  else if (!suffix)
    d->querylen = len + 86;
  else {
    suffixlen = dns_domain_length(suffix);
    m = base32_bytessize(len + 44);
    d->querylen = m + suffixlen + 73;
  }
#else
  d->querylen = len + 18;
#endif
  d->query = alloc(d->querylen);
  if (!d->query) return -1;

#ifndef DNSCURVE
  uint16_pack_big(d->query,len + 16);
  byte_copy(d->query + 2,12,flagrecursive ? "\0\0\1\0\0\1\0\0\0\0\0\0" : "\0\0\0\0\0\1\0\0\0\0\0\0gcc-bug-workaround");
  byte_copy(d->query + 14,len,q);
  byte_copy(d->query + 14 + len,2,qtype);
  byte_copy(d->query + 16 + len,2,DNS_C_IN);
#else
  d->name = q;
#endif
  byte_copy(d->qtype,2,qtype);
  d->servers = servers;
  byte_copy(d->localip,16,localip);
#ifdef DNSCURVE
  d->flagrecursive = flagrecursive;
  d->keys = keys;
  d->pubkey = pubkey;
  d->suffix = suffix;

  if (!d->keys) {
    uint16_pack_big(d->query,len + 16);
    makebasequery(d,d->query + 2);
    d->name = d->query + 14; /* keeps dns_transmit_start backwards compatible */
  }
#endif
  d->udploop = flagrecursive ? 1 : 0;

  if ((len + 16 > 512) || byte_equal(qtype,2,DNS_T_ANY)) return firsttcp(d);
  return firstudp(d);
}

void dns_transmit_io(struct dns_transmit *d,iopause_fd *x,struct taia *deadline)
{
  x->fd = d->s1 - 1;

  switch(d->tcpstate) {
    case 0: case 3: case 4: case 5:
      x->events = IOPAUSE_READ;
      break;
    case 1: case 2:
      x->events = IOPAUSE_WRITE;
      break;
  }

  if (taia_less(&d->deadline,deadline))
    *deadline = d->deadline;
}

int dns_transmit_get(struct dns_transmit *d,const iopause_fd *x,const struct taia *when)
{
  char udpbuf[4097];
  unsigned char ch;
  int r;
  int fd;
#ifdef DNSCURVE
  unsigned int len;
#endif

  errno = error_io;
  fd = d->s1 - 1;

  if (!x->revents) {
    if (taia_less(when,&d->deadline)) return 0;
    errno = error_timeout;
    if (d->tcpstate == 0) return nextudp(d);
    return nexttcp(d);
  }

  if (d->tcpstate == 0) {
/*
have attempted to send UDP query to each server udploop times
have sent query to curserver on UDP socket s
*/
    r = recv(fd,udpbuf,sizeof udpbuf,0);
    if (r <= 0) {
      if (errno == error_connrefused) if (d->udploop == 2) return 0;
      return nextudp(d);
    }
    if (r + 1 > sizeof udpbuf) return 0;

#ifdef DNSCURVE
    len = r;
    if (uncurve(d,udpbuf,&len)) return 0;
    if (irrelevant(d,udpbuf,len)) return 0;
    if (serverwantstcp(udpbuf,len)) return firsttcp(d);
    if (serverfailed(udpbuf,len)) {
       if (d->udploop == 2) return 0;
       return nextudp(d);
     }
#else
    if (irrelevant(d,udpbuf,r)) return 0;
    if (serverwantstcp(udpbuf,r)) return firsttcp(d);
    if (serverfailed(udpbuf,r)) {
      if (d->udploop == 2) return 0;
      return nextudp(d);
    }
#endif
    socketfree(d);

#ifdef DNSCURVE
    d->packetlen = len;
#else
    d->packetlen = r;
#endif
    d->packet = alloc(d->packetlen);
    if (!d->packet) { dns_transmit_free(d); return -1; }
    byte_copy(d->packet,d->packetlen,udpbuf);
    queryfree(d);
    return 1;
  }

  if (d->tcpstate == 1) {
	/*- have sent connection attempt to curserver on TCP socket s pos not defined */
    if (!socket_connected(fd)) return nexttcp(d);
    d->pos = 0;
    d->tcpstate = 2;
    return 0;
  }

  if (d->tcpstate == 2) {
	/*- have connection to curserver on TCP socket s have sent pos bytes of query */
    r = write(fd,d->query + d->pos,d->querylen - d->pos);
    if (r <= 0) return nexttcp(d);
    d->pos += r;
    if (d->pos == d->querylen) {
      struct taia now;
      taia_now(&now);
      taia_uint(&d->deadline,10);
      taia_add(&d->deadline,&d->deadline,&now);
      d->tcpstate = 3;
    }
    return 0;
  }

  if (d->tcpstate == 3) {
	/*- have sent entire query to curserver on TCP socket s pos not defined */
    r = read(fd,&ch,1);
    if (r <= 0) return nexttcp(d);
    d->packetlen = ch;
    d->tcpstate = 4;
    return 0;
  }

  if (d->tcpstate == 4) {
    /*
     * have sent entire query to curserver on TCP socket s
     * pos not defined
     * have received one byte of packet length into packetlen
     */
    r = read(fd,&ch,1);
    if (r <= 0) return nexttcp(d);
    d->packetlen <<= 8;
    d->packetlen += ch;
    d->tcpstate = 5;
    d->pos = 0;
    d->packet = alloc(d->packetlen);
    if (!d->packet) { dns_transmit_free(d); return -1; }
    return 0;
  }

  if (d->tcpstate == 5) {
   /*
    * have sent entire query to curserver on TCP socket s
    * have received entire packet length into packetlen
    * packet is allocated
    * have received pos bytes of packet
    */
    r = read(fd,d->packet + d->pos,d->packetlen - d->pos);
    if (r <= 0) return nexttcp(d);
    d->pos += r;
    if (d->pos < d->packetlen) return 0;

    socketfree(d);
#ifdef DNSCURVE
    if (uncurve(d,d->packet,&d->packetlen)) return nexttcp(d);
#endif
    if (irrelevant(d,d->packet,d->packetlen)) return nexttcp(d);
    if (serverwantstcp(d->packet,d->packetlen)) return nexttcp(d);
    if (serverfailed(d->packet,d->packetlen)) return nexttcp(d);

    queryfree(d);
    return 1;
  }

  return 0;
}
