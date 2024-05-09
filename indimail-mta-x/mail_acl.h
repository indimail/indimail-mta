/*
 * $Log: mail_acl.h,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2010-11-05 01:06:24+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifndef _MAIL_ACL_H_
#define _MAIL_ACL_H_

int             mail_acl(stralloc *, int, const char *, const char *, char);

#endif
