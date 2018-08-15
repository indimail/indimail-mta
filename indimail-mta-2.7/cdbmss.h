/*
 * $Log: cdbmss.h,v $
 * Revision 1.3  2004-06-18 22:58:05+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef CDBMSS_H
#define CDBMSS_H

#include "cdbmake.h"
#include "substdio.h"

struct cdbmss
{
	char            ssbuf[1024];
	struct cdbmake  cdbm;
	substdio        ss;
	char            packbuf[8];
	uint32          pos;
	int             fd;
};

int             cdbmss_start(struct cdbmss *, int);
int             cdbmss_add(struct cdbmss *, unsigned char *, unsigned int, unsigned char *, unsigned int);
int             cdbmss_finish(struct cdbmss *);

#endif
