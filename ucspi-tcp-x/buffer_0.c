/*
 * $Log: buffer_0.c,v $
 * Revision 1.3  2008-07-25 16:48:26+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.2  2008-07-17 23:02:26+05:30  Cprogrammer
 * use unistd.h instead of readwrite.h
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "buffer.h"

ssize_t
buffer_0_read(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	if (buffer_flush(buffer_1) == -1)
		return -1;
	return read(fd, buf, len);
}

char            buffer_0_space[BUFFER_INSIZE];
static buffer   it = BUFFER_INIT(buffer_0_read, 0, buffer_0_space, sizeof buffer_0_space);
buffer         *buffer_0 = &it;
