/*
** Copyright 2018 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc2045.h"

#include	<stdlib.h>
#include	<stdio.h>

static const struct {
	const char *address;
	int use_rfc822;
	const char *result;
} encode_tests[]={
		  {"nobody@example.com", 0, "rfc822;nobody@example.com"},
		  {"nobody+=me@example.com", 0,
		   "rfc822;nobody+2B+3Dme@example.com"},
		  {"\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5@example.com", 0, "utf-8;\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5@example.com"},
		  {"\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5@example.com", 1, "rfc822;+D0+B8+D1+81+D0+BF+D1+8B+D1+82+D0+B0+D0+BD+D0+B8+D0+B5@example.com"},
		  {"\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5+=\\me@example.com", 0, "utf-8;\xd0\xb8\xd1\x81\xd0\xbf\xd1\x8b\xd1\x82\xd0\xb0\xd0\xbd\xd0\xb8\xd0\xb5\\x{2B}\\x{3D}\\x{5C}me@example.com"},

};

int main(int argc, char **argv)
{
	size_t i;

	for (i=0; i<sizeof(encode_tests)/sizeof(encode_tests[0]); ++i)
	{
		char *p=rfc6533_encode(encode_tests[i].address,
				       encode_tests[i].use_rfc822);
		char *q;

		if (strcmp(p, encode_tests[i].result))
		{
			fprintf(stderr, "Expected to encode %s as %s, got %s\n",
				encode_tests[i].address,
				encode_tests[i].result,
				p);
			exit(1);
		}
		q=rfc6533_decode(p);

		if (!q)
		{
			fprintf(stderr, "Could not decode %s\n", p);
			exit(1);
		}

		if (strcmp(q, encode_tests[i].address))
		{
			fprintf(stderr, "Expected to decode %s as %s, got %s\n",
				p,
				encode_tests[i].address,
				q);
			exit(1);
		}
		free(p);
		free(q);
	}

	exit(0);
}
