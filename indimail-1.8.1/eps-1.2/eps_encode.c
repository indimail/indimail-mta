#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "eps.h"

int
main(int argc, char *argv[])
{
	int             ret = 0, fd = 0;
	char            buf[500] = { 0 };
	struct line_t  *l = NULL;

	if (argc < 2)
		return 1;
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		return 1;
	l = line_alloc();
	if (l == NULL)
	{
		close(fd);
		return 1;
	}
	line_init(l, NULL, 5000);
	while (1)
	{
		memset((char *) buf, 0, 500);
		ret = read(fd, buf, 500);
		if (ret < 1)
			break;
		line_inject(l, buf, ret);
	}
	ret = base64_encode(1, l);
	if (!ret)
		printf("Error encoding\n");
	line_kill(l);
	close(fd);
	return 0;
}
