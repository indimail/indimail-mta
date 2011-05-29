#ifndef	imapwrite_h
#define	imapwrite_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

static const char imapwrite_h_rcsid[]="$Id: imapwrite.h,v 1.2 1999/12/06 13:33:00 mrsam Exp $";

void writeflush();
void writemem(const char *, size_t);
void writes(const char *);
void writeqs(const char *);
void writen(unsigned long n);
void write_error_exit(const char *);
#endif
