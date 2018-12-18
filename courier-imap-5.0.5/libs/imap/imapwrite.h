#ifndef	imapwrite_h
#define	imapwrite_h

/*
** Copyright 1998 - 2018 Double Precision, Inc.
** See COPYING for distribution information.
*/


void writeflush();
void writemem(const char *, size_t);
void writes(const char *);
void writeqs(const char *);
void writemailbox(const char *);
void writen(unsigned long n);
void write_error_exit(const char *);
#endif
