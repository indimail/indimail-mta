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
	int             ret = 0;
	struct base64_t b;
	FILE           *stream = NULL;
	unsigned long   len = 0;
	unsigned char   buf[500] = { 0 };
	struct line_t  *ld = NULL;

	if (argc < 2)
		return 1;
	stream = fopen(argv[1], "r");
	if (stream == NULL)
		return 1;
	base64_init(&b);
	ld = line_alloc();
	if (ld == NULL)
	{
		fclose(stream);
		return 1;
	}
	while (!(feof(stream)))
	{
		memset(buf, 0, 500);
		fgets(buf, 500, stream);
		ret = base64_decode(&b, ld, buf);
		if (!ret)
		{
			printf("Error decoding\n");
			return 1;
		}
	}
	fclose(stream);
	for (len = 0; len < ld->bytes; len++)
		putchar(*(ld->data + len));
	line_kill(ld);
	return 0;
}
