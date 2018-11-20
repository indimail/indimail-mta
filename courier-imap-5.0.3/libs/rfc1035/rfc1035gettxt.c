/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"rfc1035.h"
#include	<string.h>


int rfc1035_rr_gettxt(struct rfc1035_rr *p, int startpos, char buf[256])
{
unsigned l;

	if (startpos < 0 || (unsigned)startpos >= p->rdlength ||
		p->rdlength - (unsigned)startpos <=
			(l=(unsigned)(unsigned char)p->rdata[startpos]))
	{
		buf[0]=0;
		return (-1);
	}

	++startpos;

	memcpy(buf, p->rdata + startpos, l);
	buf[l]=0;
	startpos += l;
	if (startpos >= p->rdlength)
		startpos= -1;
	return (startpos);
}
