#include <stdio.h>
#include <stdlib.h>
#include "rfc2045.h"

void rfc2045_error(const char *s)
{
	printf("%s\n", s);
	exit(0);
}

int main(int argc, char **argv)
{
	struct rfc2045 *rfcp;
	struct rfc2045headerinfo *hi;
	char *h, *v;

	if (argc < 3)
		return (0);

	rfcp=rfc2045_fromfd(0);

	if (!rfcp)
		return (0);

	if (argv[2][0])
		rfcp=rfc2045_find(rfcp, argv[2]);

	hi=rfc2045header_start(0, rfcp);

	if (!hi)
		return (0);

	while (rfc2045header_get(hi, &h, &v, atoi(argv[1])) == 0)
	{
		if (h == NULL)
			break;
		printf("Header: %s\n", h);
		printf("Value: %s\n", v);
	}
	return (0);
}
