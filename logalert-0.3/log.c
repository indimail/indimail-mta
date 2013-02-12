#include "log.h"

void
debug(char *msg, ...)
{

	va_list         ap;
	char           *buff;
	size_t          bsize = MAXLOGBSIZE;
	int             n;

	buff = (char *) xmalloc(bsize);

START:
	va_start(ap, msg);
	n = vsnprintf(buff, bsize, msg, ap);
	va_end(ap);

	if (n < 0 || n > bsize) {
		bsize *= 2;
		xrealloc((char *) buff, bsize);
		goto START;
	}
	fprintf(stdout, "%s\n", buff);
	free(buff);
}


/*
 * this is when we are really fucked up - memory problems, etc
 */
void
fatal(char *msg)
{

	fprintf(stderr, "[fatal] - %s\n", msg);
	exit(1);
}
