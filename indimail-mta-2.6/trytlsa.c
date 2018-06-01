#include "stralloc.h"
#include "tlsarralloc.h"
#include "dns.h"
int main()
{
#ifdef HASTLSA
tlsarralloc ta;
ipalloc  ia;
	dns_tlsarr(&ta, &ia);
#else
	:
#endif
  return(0);
}
