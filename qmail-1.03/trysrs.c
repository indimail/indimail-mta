#include <sys/types.h>
#include <time.h>
#include <srs2.h>

int main()
{
#ifdef HAVESRS
	srs_t          *srs;
#else
  :
#endif
  return(0);
}
