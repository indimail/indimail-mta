/*
 * $Log: control.h,v $
 * Revision 1.4  2024-05-09 22:55:54+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2017-03-30 16:19:06+05:30  Cprogrammer
 * removed control_readfile()
 *
 * Revision 1.2  2005-05-13 23:44:37+05:30  Cprogrammer
 * code indentation
 *
 * Revision 1.1  2003-12-31 19:57:33+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef CONTROL_H
#define CONTROL_H
#include "stralloc.h"

int             control_readint(int *, const char *);
int             control_readline(stralloc *, const char *);
void            striptrailingwhitespace(stralloc *);
#endif
