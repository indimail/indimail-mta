#include <unistd.h>
#include <strerr.h>
#include "filterit.h"

int
main(int argc, char **argv)
{
	if (lseek(0, 0, SEEK_SET) == -1)
		strerr_die2sys(111, FATAL, "unable to seek: ");
	return (filterit_sub1(argc, argv));
}
