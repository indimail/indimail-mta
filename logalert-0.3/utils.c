/*
 * $Log: utils.c,v $
 * Revision 1.1  2013-05-15 00:35:21+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <ctype.h>
#include "utils.h"

void           *
xmalloc(size_t size)
{
	register void  *value = malloc(size);
	if (value == 0) {
		fprintf(stderr, "[*]xmalloc: virtual memory exhausted");
		exit(1);
	}
	return value;
}

void           *
xrealloc(void *ptr, size_t size)
{
	register void  *value = realloc(ptr, size);
	if (value == 0) {
		fprintf(stderr, "[*]xrealloc: virtual memory exhausted");
		exit(1);
	}
	return value;
}

int
open_file(char *file, int flags, mode_t * mode)
{
	return open(file, flags, mode);
}

int
open_file_ro(char *file)
{
	return open_file(file, O_RDONLY, 0);
}


char           *
getword(char *line)
{
	return strtok(line, DELIMITERS);
}

void
mywrite(int fd, char const *buffer, size_t n_bytes)
{
	if (n_bytes > 0 && fwrite(buffer, 1, n_bytes, stdout) == 0)
		fatal("could not write to stdout");
}

/*
 * dupArray(char **src)
 * 
 * Returns a copy of **src. Finds out the size of src based on last value
 * being NULL.
 */
char          **
dupArray(char **src)
{
	char          **dst;
	unsigned int    i, asize = 0;

	while (*(src + asize))
		asize++;
	dst = (char **) xmalloc((asize + 2) * sizeof (char *));
	for (i = 0; i < asize; i++) {
		dst[i + 1] = (char *) xmalloc(strlen(src[i]) + 1);
		strcpy(dst[i + 1], src[i]);
	}
	dst[i + 1] = NULL;
	return dst;
}
