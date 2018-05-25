#include "tlsaquery.h"
int main()
{
#if defined(HASTLSA) && defined(TLS)
	do_tlsa_query("mail.ietf.org", 25);
#else
	:
#endif
  return(0);
}
