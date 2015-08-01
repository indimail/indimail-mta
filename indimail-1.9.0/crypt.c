/*
 * $Log: crypt.c,v $
 * Revision 2.2  2011-04-08 17:25:42+05:30  Cprogrammer
 * added HAVE_CONFIG_H
 *
 * Revision 2.1  2009-11-17 13:47:33+05:30  Cprogrammer
 * crypt program
 *
 * Revision 1.1  2002-12-16 01:54:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#define _XOPEN_SOURCE
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#else
char           *crypt(const char *, const char *);
#endif

int
main(int argc, char **argv)
{
	if (argc != 3)
	{
		printf("Usage: %s <key> <salt>\n", *argv);
		exit(EXIT_FAILURE);
	}
	printf("\"%s\"\n", crypt(*(argv + 1), *(argv + 2)));
	return(0);
}
