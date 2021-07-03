/*
 * $Log: dlnamespace.h,v $
 * Revision 1.3  2021-07-03 14:04:26+05:30  Cprogrammer
 * use Lmid_t data type for id instead of unsigned long
 *
 * Revision 1.2  2017-04-22 11:55:11+05:30  Cprogrammer
 * added new argument, environ list to dlnamespace()
 *
 * Revision 1.1  2017-04-05 03:09:29+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef DLNAMESPACE_H
#define DLNAMESPACE_H
#define _GNU_SOURCE
#include <dlfcn.h>

int             dlnamespace(char *, char **, Lmid_t *);

#endif
