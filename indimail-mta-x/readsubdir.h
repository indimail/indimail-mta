/*
 * $Log: readsubdir.h,v $
 * Revision 1.4  2021-05-12 17:50:29+05:30  Cprogrammer
 * added readsubdir_name()
 *
 * Revision 1.3  2004-10-11 14:00:38+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:36+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef READSUBDIR_H
#define READSUBDIR_H

#include "direntry.h"

#define READSUBDIR_NAMELEN 10

typedef struct readsubdir
{
	DIR            *dir;
	int             pos;
	int             split;
	char           *name;
	void            (*pause) ();
} readsubdir;

void            readsubdir_init(readsubdir *, char *, int, void (*pause) ());
int             readsubdir_next(readsubdir *, unsigned long *);
char           *readsubdir_name(readsubdir *);

#endif
