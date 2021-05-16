/*
 * $Log: maildir_deliver.h,v $
 * Revision 1.1  2021-05-16 22:43:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef MAILDIR_CHILD_H
#define MAILDIR_CHILD_H
#include <stralloc.h>

int             maildir_deliver(char *, stralloc *, stralloc *, char *);

#endif
