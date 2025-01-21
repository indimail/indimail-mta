/*
 * $Id: readsubdir.h,v 1.8 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
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
	const char     *name;
	void            (*pause) (const char *);
} readsubdir;

void            readsubdir_init(readsubdir *, const char *, int, void (*pause) (const char *));
int             readsubdir_next(readsubdir *, unsigned long *);
char           *readsubdir_name(readsubdir *);

#endif
/*
 * $Log: readsubdir.h,v $
 * Revision 1.8  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.7  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2022-01-30 09:38:08+05:30  Cprogrammer
 * allow configurable big/small todo/intd
 *
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
