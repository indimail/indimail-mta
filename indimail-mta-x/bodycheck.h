/*
 * $Log: bodycheck.h,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2008-12-23 11:29:12+05:30  Cprogrammer
 * added bodycheck_free()
 *
 * Revision 1.2  2004-06-18 22:55:49+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef _BODY_CHECK_H
#define _BODY_CHECK_H
int             bodycheck(stralloc *, stralloc *, const char **, int);
void            bodycheck_free(void);
#endif
