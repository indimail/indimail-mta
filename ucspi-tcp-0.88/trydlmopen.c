#define _GNU_SOURCE
#include <dlfcn.h>

void
main()
{
	dlmopen(0, 0, 0);
}

