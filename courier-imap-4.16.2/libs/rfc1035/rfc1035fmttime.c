/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>


const char *rfc1035_fmttime(unsigned long n, char *buf)
{
unsigned long s,m,h;

	s = n % 60; n=n/60;
	m = n % 60; n=n/60;
	h = n % 24; n=n/24;

	buf[0]='\0';
	if (n)	sprintf(buf,"%lud",n);
	if (n || h)	sprintf(buf+strlen(buf), "%luh", h);
	if (n || h || m)	sprintf(buf+strlen(buf), "%lum", m);
	sprintf(buf+strlen(buf), "%lus", s);
	return (buf);
}
