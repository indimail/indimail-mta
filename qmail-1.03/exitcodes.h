/*
 * $Log: exitcodes.h,v $
 * Revision 1.2  2004-09-21 23:45:49+05:30  Cprogrammer
 * added cases for virus infection and banned attachments
 *
 * Revision 1.1  2004-09-19 18:52:57+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef _EXITCODES_H_
#define _EXITCODES_H_
/*- Exit codes for qmail-queue */
#define QQ_XPERM  31
#define QQ_VIRUS  33
#define QQ_BADEX  34
#define QQ_XWRTE  53
#define QQ_XREAD  54
#define QQ_XTEMP  71
#define QQ_XBUGS  81

/*- Exit codes for virus scanning */
#define EX_ALLOK  0
#define EX_VIRUS  1
#define EX_TMPERR 2
#define EX_BADEX  3
#endif
