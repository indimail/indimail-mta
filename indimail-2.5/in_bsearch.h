/*
 * $Log: in_bsearch.h,v $
 * Revision 2.2  2017-12-11 13:36:00+05:30  Cprogrammer
 * added new member 'domain'
 *
 * Revision 2.1  2017-11-20 23:22:32+05:30  Cprogrammer
 * header for binary search
 *
 */
/*
 * __USE_GNU exposes the GNU tdestroy() function, a
 * function that is not mentioned by the Single Unix
 * Specification. 
 */
#define __USE_GNU 1
#define _GNU_SOURCE
#include <search.h>
#include <sys/types.h>
#include <pwd.h>

typedef struct
{
	char           *in_key;
	struct passwd   in_pw;
	char            pwStat;
	char           *aliases;
	char           *mdahost;
	char           *domain;
	/*-
	 *  0: User is fine
	 *  1: User is not present
	 *  2: User is Inactive
	 *  3: User is overquota
	 * -1: System Error
	 */
	int             in_userStatus;
} INENTRY;
