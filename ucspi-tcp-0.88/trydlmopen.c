#define _GNU_SOURCE
#include <dlfcn.h>

int
main()
{
#ifdef USE_DLMOPEN
	dlmopen(0, 0, 0);
#else
	:
#endif
}
