#include "byte.h"
#include "fmt.h"
#include "dns.h"

/* RFC1886:
 *   4321:0:1:2:3:4:567:89ab
 * ->
 *   b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.IP6.INT.
 *   b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.IP6.ARPA.
 */

extern char tohex(char num);

unsigned int mkint(unsigned char a,unsigned char b) {
  return ((unsigned int)a << 8) + (unsigned int)b;
}

int dns_name6_domain(char name[DNS_NAME6_DOMAIN],const char ip[16],int t)
{
  unsigned int j;

  for (j=0; j<16; j++) {
    name[j*4]=1;
    name[j*4+1]=tohex(ip[15-j] & 15);
    name[j*4+2]=1;
    name[j*4+3]=tohex((unsigned char)ip[15-j] >> 4);
  }
  if (t==DNS_IP6_INT)
    byte_copy(name + 4*16,9,"\3ip6\3int\0");
  else if (t==DNS_IP6_ARPA)
    byte_copy(name + 4*16,10,"\3ip6\4arpa\0");
  else return 0;
  return 4*16+9+t;
}

