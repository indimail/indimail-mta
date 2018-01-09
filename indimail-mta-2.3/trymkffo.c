#include <sys/types.h>
#include <sys/stat.h>

int
main()
{
	mkfifo("temp-trymkffo", 0);
	return(0);
}
