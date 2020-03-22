#include "stralloc.h"
#include "tlsarralloc.h"
#include "dns.h"
int main()
{
#ifdef HASTLSA
tlsarralloc ta;
stralloc  sa = {0};
	dns_tlsarr(&ta, &sa);
#else
	:
#endif
  return(0);
}
