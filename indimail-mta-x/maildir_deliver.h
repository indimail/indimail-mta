/*
 * $Log: maildir_deliver.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2021-05-16 22:43:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef MAILDIR_CHILD_H
#define MAILDIR_CHILD_H
#include <stralloc.h>

int             maildir_deliver(const char *, stralloc *, stralloc *, const char *);

#endif
