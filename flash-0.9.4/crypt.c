/*
 * $Log: crypt.c,v $
 * Revision 1.2  2008-07-17 21:36:50+05:30  Cprogrammer
 * fixed compilation warnings on mac
 *
 * Revision 1.1  2002-12-16 01:54:54+05:30  Manny
 * Initial revision
 *
 */
#include<stdio.h>
#include<stdlib.h>
#include "config.h"
#define _XOPEN_SOURCE
#include<unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#else
char *crypt(const char *, const char *);
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
