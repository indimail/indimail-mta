#include "stralloc.h"
#include "tlsarralloc.h"
#include <arpa/nameser.h>
int main()
{
#if defined(HASTLSA) && defined T_TLSA
	dns_tlsarr();
#else
	:
#endif
  return(0);
}
