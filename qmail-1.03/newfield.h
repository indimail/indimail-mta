/*
 * $Log: newfield.h,v $
 * Revision 1.3  2004-10-11 13:56:15+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:01:07+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef NEWFIELD_H
#define NEWFIELD_H

#include "stralloc.h"
#include "datetime.h"

extern stralloc newfield_date;
extern stralloc newfield_msgid;

int             newfield_datemake(datetime_sec);
int             newfield_msgidmake(char *, int, datetime_sec);

#endif
