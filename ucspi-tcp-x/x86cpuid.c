/*
 * $Log: x86cpuid.c,v $
 * Revision 1.2  2020-08-03 17:28:33+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.1  2003-12-31 19:46:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void
nope()
{
	exit(1);
}

int
main()
{
	unsigned long   x[4];
	unsigned long   y[4];
	int             i;
	int             j;
	char            c;

	signal(SIGILL, nope);

	x[0] = 0;
	x[1] = 0;
	x[2] = 0;
	x[3] = 0;

	asm volatile    (".byte 15;.byte 162":"=a" (x[0]), "=b"(x[1]), "=c"(x[3]), "=d"(x[2]):"0"(0));
	if (!x[0])
		return 0;
	asm volatile    (".byte 15;.byte 162":"=a" (y[0]), "=b"(y[1]), "=c"(y[2]), "=d"(y[3]):"0"(1));

	for (i = 1; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			c = x[i] >> (8 * j);
			if (c < 32)
				c = 32;
			if (c > 126)
				c = 126;
			putchar(c);
		}
	}
	printf("-%08x-%08x\n", y[0], y[3]);

	return 0;
}
