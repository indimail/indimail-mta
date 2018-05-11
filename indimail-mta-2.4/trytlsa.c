#include "query-getdns.h"
int main()
{
#if defined(HASTLSA) && defined(TLS)
	do_dns_queries("ietf.org", 25, 1);
#else
	:
#endif
  return(0);
}
