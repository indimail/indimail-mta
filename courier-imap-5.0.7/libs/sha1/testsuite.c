/*
** Copyright 2001-2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	"sha1.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>


static char foo[1000001];

static void sha1()
{
SHA1_DIGEST	digest;
unsigned i, n;

static char *testcases[]={"abc",
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", foo};

	for (n=0; n<sizeof(testcases)/sizeof(testcases[0]); n++)
	{
		i=strlen(testcases[n]);
		sha1_digest(testcases[n], i, digest);
		printf( (i < 200 ? "SHA1(%s)=":
			"SHA1(%-1.20s...)="), testcases[n]);

		for (i=0; i<20; i++)
		{
			if (i && (i & 3) == 0)	putchar(' ');
			printf("%02X", digest[i]);
		}
		printf("\n");
	}
}

static void sha256()
{
	SHA256_DIGEST	digest;
	unsigned i, n;

	static char *testcases[]={"abc",
				  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
				  foo};

	for (n=0; n<sizeof(testcases)/sizeof(testcases[0]); n++)
	{
		i=strlen(testcases[n]);
		sha256_digest(testcases[n], i, digest);
		printf( (i < 200 ? "SHA256(%s)=":
			 "SHA256(%-1.20s...)="), testcases[n]);

		for (i=0; i<sizeof(digest); i++)
		{
			if (i && (i & 3) == 0)	putchar(' ');
			printf("%02X", digest[i]);
		}
		printf("\n");
	}
}

static void sha512()
{
	SHA512_DIGEST	digest;
	unsigned i, n;

	static char *testcases[]={"abc",
				  "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
				  foo};

	for (n=0; n<sizeof(testcases)/sizeof(testcases[0]); n++)
	{
		i=strlen(testcases[n]);
		sha512_digest(testcases[n], i, digest);
		printf( (i < 200 ? "SHA512(%s)=":
			 "SHA512(%-1.20s...)="), testcases[n]);

		for (i=0; i<sizeof(digest); i++)
		{
			if (i && (i & 7) == 0)	putchar(' ');
			printf("%02X", digest[i]);
		}
		printf("\n");
	}
}
int main()
{
	memset(foo, 'a', 1000000);
	sha1();
	sha256();
	sha512();
	exit (0);
	return (0);
}
