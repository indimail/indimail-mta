#include "common.h"
#include <stdarg.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>

/*
 * With O_NONBLOCK in effect, we need to loop our read/write with a select
 * combination until *all* our data is read/written (or until error)
 */
ssize_t
write_wt(int fd, const void *buf, size_t count, int timeout)
{
	fd_set          wfds;
	struct timeval  tv;
	ssize_t         bytes;
	ssize_t         to_write = count;
	const char     *cbuf = (char *) buf;

	if (to_write == 0)
		return count;
	while (1) {
		FD_ZERO(&wfds);
		FD_SET(fd, &wfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		if (select(fd + 1, NULL, &wfds, NULL, &tv) < 1)
			return -1;
		bytes = write(fd, cbuf, to_write);
		if (bytes < 1)
			return -1;
		to_write -= bytes;
		if (to_write == 0)
			return count;
		if (to_write < 0)
			return -1;
		cbuf += bytes;
	}
}

ssize_t
read_wt(int fd, void *buf, size_t count, int timeout)
{
	fd_set          rfds;
	struct timeval  tv;
	ssize_t         bytes;
	ssize_t         to_read = count;
	char           *cbuf = (char *) buf;

	if (to_read == 0)
		return count;
	while (1) {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		if (select(fd + 1, &rfds, NULL, NULL, &tv) < 1)
			return -1;
		bytes = read(fd, cbuf, to_read);
		if (bytes < 1)
			return -1;
		to_read -= bytes;
		if (to_read == 0)
			return count;
		if (to_read < 0)
			return -1;
		cbuf += bytes;
	}
}

void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = (char *) getenv(envname)))
		*source = defaultValue;
	return;
}
