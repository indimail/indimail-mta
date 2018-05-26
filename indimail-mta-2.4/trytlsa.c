#include "stralloc.h"
#include "tlsarralloc.h"
int main()
{
#if defined(HASTLSA)
	dns_tlsarr();
#else
	:
#endif
  return(0);
}
