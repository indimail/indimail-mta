/** \file bogogrep.c
 *
 * This file emulates GNU grep -ab with a plain text pattern anchored to
 * the left. The Horspool search was taken from a publicly available
 * version on the Internet.
 *
 * The rest of the program was written by Matthias Andree and is public
 * domain.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef MAP_FAILED
#define MAP_FAILED	((void *) -1)
#endif

#if !defined(__GNUC__)
#define __attribute__(a)
#endif

#define ALPHABET 256

/* vanilla Boyer-Moore-Horspool */
static const unsigned char *search(const unsigned char *text,
	const unsigned char *pat, long n)
{
	long i, j, k, m, skip[ALPHABET];

	m = strlen((const char *)pat);
	if (m == 0)
		return (text);
	for (k = 0; k < ALPHABET; k++)
		skip[k] = m;
	for (k = 0; k < m - 1; k++)
		skip[pat[k]] = m - k - 1;

	for (k = m - 1; k < n; k += skip[text[k] & (ALPHABET - 1)]) {
		for (j = m - 1, i = k; j >= 0 && text[i] == pat[j]; j--)
			i--;
		if (j == (-1))
			return (text + i + 1);
	}
	return (NULL);
}

static void usage(int rc, const char *tag) __attribute__((noreturn)) ;
static void usage(int rc, const char *tag) {
    fprintf(stderr, "Usage: %s searchstring regular_file [...]\n"
	    "This program searches all lines in regular_file that start with searchstring.\n"
	    "For each match, it prints the byte offset (starting with 0), a colon and the\n"
	    "the matching line.\n", tag);
    exit(rc);
}

int main(int argc, char **argv) {
    int fd;
    unsigned char *base;
    const unsigned char *i;
    struct stat st;
    int argindex; /* file name in argv[] vector */
    int rc = 0;

    if (argc < 3) usage(1, argv[0]);

    for(argindex = 2; argindex < argc; argindex++) {
	if ((fd = open(argv[argindex], O_RDONLY)) < 0) {
	    perror(argv[argindex]);
	    rc = 1;
	    continue;
	}

	if (fstat(fd, &st)) {
	    perror(argv[argindex]);
	    close(fd);
	    rc = 1;
	    continue;
	}

	if (MAP_FAILED == (base = (unsigned char *)mmap(NULL, st.st_size,
			PROT_READ, MAP_SHARED, fd, 0))) {
	    perror("mmap");
	    close(fd);
	    rc = 1;
	    continue;
	}

	i = base;
	while(NULL != (i = search((const unsigned char *)i,
			(unsigned char *)argv[1],
			base - i + st.st_size))) {
	    if (i == base || *(i-1) == '\n') {
		/** \bug FIXME: dead assignments here */
		int l = strcspn((const char *)i, "\n");

		if (l > base - i + st.st_size) l = base - i + st.st_size;
		printf("%ld:", (long)(i - base));
		(void) fwrite(i, 1, strcspn((const char *)i, "\n"), stdout);
		if (EOF == puts("")) {
		    perror("stdout");
		    exit(2);
		}
	    }
	    i += strlen(argv[1]);
	}

	if (munmap((char *)base, st.st_size)) {
	    perror("munmap");
	    rc = 1;
	}
	if (close(fd)) {
	    perror("close");
	    rc = 1;
	}
    }
    if (fflush(stdout)) {
	perror("stdout");
	rc = 2;
    }
    exit(rc);
}
