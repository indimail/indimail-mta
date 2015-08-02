/* $Id: msgcounts.h 6988 2013-01-20 14:02:48Z m-a $ */

/*****************************************************************************

NAME:
   msgcounts.h -- routines for setting & computing .MSG_COUNT values

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef MSGCOUNTS_H
#define MSGCOUNTS_H

#include "lexer.h"

/* Globals */

#define	MSG_COUNT_MAX_LEN 100
extern	yylex_t	 read_msg_count_line;
extern	bool	 msgcount_more(void);

extern	char	*msg_count_text;
extern	int	 msg_count_leng; /* DO NOT MAKE THIS SIZE_T */

/* Function prototypes */

void init_msg_counts(void);
void set_msg_counts(u_int32_t good, u_int32_t spam);
void set_msg_counts_from_str(char  *str);

long msg_count_get_token(byte **output);

#endif	/* MSGCOUNTS_H */
