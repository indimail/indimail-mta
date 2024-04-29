#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>

int
main()
{
	inotify_init();
	return(0);
}
