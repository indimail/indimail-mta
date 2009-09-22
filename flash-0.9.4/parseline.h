/*
 * $Log: parseline.h,v $
 * Revision 1.1  2002-12-16 01:55:40+05:30  Manny
 * Initial revision
 *
 */
#if !defined (_PARSELINE_H)
#   define _PARSELINE_H

#define COMMENT '#'

enum parse_state
{
	S_LWSP, S_WORD, S_QUOTED, S_ESCAPED, S_TILDE, S_DOLLAR, S_TRANSLATE
};

int             parseline(char *, int *, char **, int);

#endif
