/* $Id: lexer.h 6988 2013-01-20 14:02:48Z m-a $ */

/*****************************************************************************

NAME:
   lexer.h -- prototypes and definitions for lexer.c

******************************************************************************/

#ifndef	LEXER_H
#define	LEXER_H

#include "buff.h"
#include "word.h"

extern FILE *yyin;

extern	bool	block_on_subnets;

#define YY_NULL 0

/* lexer interface */
typedef enum {
    NONE,
    TOKEN,	/* regular token */
    MONEY,	/* dollars & cents */
    HEADKEY,	/* header keyword */
    EOH,	/* end-of-header (empty line) */
    BOUNDARY,	/* MIME multipart boundary line */
	SUBJECT,
    QUEUE_ID,	/* Queue ID of message */
    MESSAGE_ID,	/* Message ID of message */
    MESSAGE_ADDR,/* Message's IP address */
    IPADDR,	/* Generic IP address */
    VERP,	/* Variable Envelope Return Path */
    MSG_COUNT_LINE,
    BOGO_LEX_LINE
} token_t;

/* in lexer.c */
extern int yylineno;
extern bool msg_header;
extern bool have_body;

/* Define a struct for interfacing to a lexer */

typedef token_t yylex_t(void);

typedef struct lexer_s {
    yylex_t  *yylex;
    long (*get_parser_token)(byte **data);
} lexer_t;

extern lexer_t *lexer;
extern lexer_t	msg_count_lexer;

/* in lexer_v3.l */
extern token_t	yylex(void);
extern void	lexer_v3_init(FILE *fp);
extern long	lexer_v3_get_token(byte **output);

/* in lexer_v?.c */
extern char yy_get_state(void);
extern void yy_set_state_initial(void);

/* in lexer.c */
extern void 	lexer_init(void);
extern void	yyinit(void);
extern int	yyinput(byte *buf, size_t used, size_t size);

extern int	buff_fill(buff_t *buff, size_t used, size_t need);

extern word_t  *text_decode(word_t *w);
extern size_t	html_decode(word_t *w);

#endif	/* LEXER_H */
