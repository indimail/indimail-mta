/*
 * $Log: qcount_dir.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2020-03-24 12:56:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef QCOUNT_DIR_H
#define QCOUNT_DIR_H
#include <sys/types.h>

ssize_t         qcount_dir(const char *, size_t *);

#endif
