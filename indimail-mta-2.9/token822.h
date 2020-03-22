/*
 * $Log: token822.h,v $
 * Revision 1.4  2009-06-04 12:09:14+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.3  2004-10-11 14:16:04+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.2  2004-06-18 23:02:17+05:30  Cprogrammer
 * added RCS log
 *
 */
#ifndef TOKEN822_H
#define TOKEN822_H
#include "gen_alloc.h"

#define TOKEN822_ATOM 1
#define TOKEN822_QUOTE 2
#define TOKEN822_LITERAL 3
#define TOKEN822_COMMENT 4
#define TOKEN822_LEFT 5
#define TOKEN822_RIGHT 6
#define TOKEN822_AT 7
#define TOKEN822_COMMA 8
#define TOKEN822_SEMI 9
#define TOKEN822_COLON 10
#define TOKEN822_DOT 11
GEN_ALLOC_typedef(token822_alloc, struct token822, t, len, a)
struct token822
{
	int             type;
	char           *s;
	int             slen;
};

int             token822_parse(token822_alloc *, stralloc *, stralloc *);
int             token822_addrlist(token822_alloc *, token822_alloc *, token822_alloc *, int (*callback)());
int             token822_unquote(stralloc *, token822_alloc *);
int             token822_unparse(stralloc *, token822_alloc *, unsigned int);
void            token822_reverse(token822_alloc *);
int             token822_ready(token822_alloc *, register unsigned int);
int             token822_readyplus(token822_alloc *, register unsigned int);
int             token822_append(token822_alloc *, struct token822 *);

#endif
