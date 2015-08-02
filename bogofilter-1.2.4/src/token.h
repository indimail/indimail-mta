/* $Id: token.h 6493 2006-06-20 22:22:00Z relson $ */

/*****************************************************************************

NAME:
   token.h -- prototypes and definitions for token.c

******************************************************************************/

#ifndef	HAVE_TOKEN_H
#define	HAVE_TOKEN_H

#include "lexer.h"

extern word_t *msg_addr;	/* First IP Address in Received: statement */
extern word_t *msg_id;		/* Message ID */
extern word_t *subject;		/* Subject */
extern word_t *queue_id;	/* Message's first Queue ID */

extern token_t get_token(word_t *token);

extern void got_from(void);
extern void clr_tag(void);
extern void set_tag(const char *text);

extern void set_msg_id(byte *text, uint leng);
extern void set_subject(byte *text, uint leng);

extern void token_init(void);
extern void token_cleanup(void);

/* used by lexer_text_html.l */
extern void html_tag(int level);
extern void html_comment(int level);

#endif	/* HAVE_TOKEN_H */
