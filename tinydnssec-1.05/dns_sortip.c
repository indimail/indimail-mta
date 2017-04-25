#include "byte.h"
#include "dns.h"

/* XXX: sort servers by configurable notion of closeness? */
/* XXX: pay attention to competence of each server? */

void dns_sortip(char *s,unsigned int n)
{
  unsigned int i;
  char tmp[4];

  n >>= 2;
  while (n > 1) {
    i = dns_random(n);
    --n;
    byte_copy(tmp,4,s + (i << 2));
    byte_copy(s + (i << 2),4,s + (n << 2));
    byte_copy(s + (n << 2),4,tmp);
  }
}

#ifdef DNSCURVE
void dns_sortip2(char *s,char *t,unsigned int n)
{
  unsigned int i;
  char tmp[32];

  while (n > 1) {
    i = dns_random(n);
    --n;
    byte_copy(tmp,4,s + (i << 2));
    byte_copy(s + (i << 2),4,s + (n << 2));
    byte_copy(s + (n << 2),4,tmp);
    byte_copy(tmp,32,t + (i << 5));
    byte_copy(t + (i << 5),32,t + (n << 5));
    byte_copy(t + (n << 5),32,tmp);
  }
}
#endif
