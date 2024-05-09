/*
 * $Log: rules.h,v $
 * Revision 1.3  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2005-05-13 23:46:28+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef RULES_H
#define RULES_H

#include "stralloc.h"

extern stralloc rules_name;
int             rules(void (*)(char *, unsigned int), int, char *, const char *, const char *);

#endif
