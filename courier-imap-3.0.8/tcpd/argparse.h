#ifndef	argparse_h
#define	argparse_h

/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

/* $Id: argparse.h,v 1.1 2000/07/09 16:31:24 mrsam Exp $ */

#include	"config.h"

struct args {
	const char *name;
	const char **valuep;
	void (*funcp)(const char *);
	} ;

int argparse(int argc, char **, struct args *);

#endif
