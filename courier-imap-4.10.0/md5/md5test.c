/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"md5.h"
#include	<stdio.h>
#include	<string.h>

int	main()
{
static const char * const teststr[]={
"",
"a",
"abc",
"message digest",
"abcdefghijklmnopqrstuvwxyz",
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
"12345678901234567890123456789012345678901234567890123456789012345678901234567890"};

char	*salts[4]={"abcdef","01234567","76543210","QWERTY"};
char	*passwds[4]={	"rosebud",
			"trust noone",
			"trust, but verify",
			"for the world is hollow, and I have touched the sky"};

int	i,j;

	printf("MD5 test suite:\n");
	for (i=0; i<(int)sizeof(teststr)/sizeof(teststr[0]); i++)
	{
	MD5_DIGEST digest;

		md5_digest(teststr[i], strlen(teststr[i]), digest);

		printf("MD5 (\"%s\") = ", teststr[i]);
		for (j=0; j<sizeof(digest); j++)
			printf("%02x", digest[j]);
		printf("\n");
	}
	for (i=0; i<sizeof(salts)/sizeof(salts[0]); i++)
		printf("Salt: %s\nPassword: %s\nHash:%s\n\n",
				salts[i], passwds[i],
				md5_crypt_redhat(passwds[i], salts[i]));
	return (0);
}
