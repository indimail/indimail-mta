/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<ctype.h>
#include	<string.h>
#include	"random128.h"


static int nyb(char c)
{
	static const char xdigit[]="0123456789ABCDEF";

	const char *p=strchr(xdigit, c);

	if (p)
		return (p-xdigit);
	return 0;
}

void random128_binary(random128binbuf *bytes)
{
	char randombuf[ 128 / 8 * 2 + 1];
	int i;

	strcpy(randombuf, random128());

	for (i=0; i<128/8; i++)
		(*bytes)[i]=(nyb(randombuf[i*2]) << 4) | nyb(randombuf[i*2+1]);
}
