/* $Id: token.c 6988 2013-01-20 14:02:48Z m-a $ */

/*****************************************************************************

NAME:
   token.c -- post-lexer token processing

   12/08/02 - split out from lexer.l

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "bogoreader.h"
#include "charset.h"
#include "error.h"
#include "mime.h"
#include "msgcounts.h"
#include "word.h"
#include "token.h"
#include "xmemrchr.h"

#define	MSG_COUNT_PADDING 2 * 10	/* space for 2 10-digit numbers */

/* Local Variables */

word_t	*msg_addr;	/* First IP Address in Received: statement */
word_t	*msg_id;	/* Message ID */
word_t	*subject;	/* Subject */
word_t	*queue_id;	/* Message's first queue ID */

static token_t save_class = NONE;
static word_t *ipsave;

static byte  *yylval_text;
static size_t yylval_text_size;
static word_t yylval;

static word_t *w_to   = NULL;	/* To:          */
static word_t *w_from = NULL;	/* From:        */
static word_t *w_rtrn = NULL;	/* Return-Path: */
static word_t *w_recv = NULL;	/* Received:    */
static word_t *w_head = NULL;	/* Header:      */
static word_t *w_mime = NULL;	/* Mime:        */
static word_t *w_ip   = NULL;	/* ip:          */
static word_t *w_url  = NULL;	/* url:         */

/* Global Variables */

bool block_on_subnets = false;

static word_t *token_prefix = NULL;
static uint32_t token_prefix_len;

#define NONBLANK "spc:invalid_end_of_header"
static word_t *nonblank_line = NULL;

static uint tok_count         = 0;
static uint init_token        = 1;
static word_t *p_multi_words  = NULL;
static byte   *p_multi_buff   = NULL;
static byte   *p_multi_text   = NULL;
static word_t **w_token_array = NULL;

/* Function Prototypes */

static void    token_clear(void);
static token_t parse_new_token(word_t *token);
static void    add_token_to_array(word_t *token);
static void    build_token_from_array(word_t *token);
static uint    token_copy_leng(const char *str, uint leng, byte *dest);

/* Function Definitions */

static void init_token_array(void)
{
    uint i;
    byte *text;
    word_t *words;
		    
    p_multi_words = calloc( max_token_len, sizeof(word_t) );
    p_multi_buff  = malloc( max_multi_token_len+D );
    p_multi_text  = calloc( max_token_len+1+D, multi_token_count );
    w_token_array = calloc( multi_token_count, sizeof(*w_token_array) );

    text = p_multi_text;
    words = p_multi_words;

    for (i = 0; i < multi_token_count; i += 1) {
	words->leng = 0;
	words->u.text = text;
	w_token_array[i] = words;
	words += 1;
	text += max_token_len+1+D;
    }
}

static void free_token_array(void)
{
    free(p_multi_words);
    free(p_multi_text );
    free(p_multi_buff );
    free(w_token_array);
}

static void token_set( word_t *token, byte *text, uint leng )
{
    token->leng = leng;
    memcpy(token->u.text, text, leng);		/* include nul terminator */
    token->u.text[leng] = '\0';			/* ensure nul termination */
}

static inline void token_copy( word_t *dst, word_t *src )
{
    token_set(dst, src->u.text, src->leng);
}

static void build_prefixed_token( word_t *prefix, word_t *token,
				  word_t *temp, uint32_t temp_size )
{
    uint len = token->leng + prefix->leng;
    
    if (len >= temp_size)
	len = temp_size - prefix->leng - 1;

    temp->leng = len;
    memmove(temp->u.text+prefix->leng, token->u.text, len-prefix->leng);
    memcpy(temp->u.text, prefix->u.text, prefix->leng);
    Z(temp->u.text[temp->leng]);

    token->leng = temp->leng;
    token->u.text = temp->u.text;
}

#define WRAP(n)	((n) % multi_token_count)

token_t get_token(word_t *token)
{
    token_t cls;
    
    bool fSingle = (tok_count < 2 ||
		    tok_count <= init_token ||
		    multi_token_count <= init_token);

    if (fSingle) {
	cls = parse_new_token(token);

	if (multi_token_count > 1)
	    add_token_to_array(token);
    }
    else {
	cls = TOKEN;
	build_token_from_array(token);
    }

    if (token_prefix != NULL) {
	/* IP addresses get special prefix */
	if (save_class != IPADDR) {
	    build_prefixed_token(token_prefix, token, &yylval, yylval_text_size);
	}
	else {
	    word_t *prefix = (wordlist_version >= IP_PREFIX) ? w_ip : w_url;
	    build_prefixed_token(prefix, token, &yylval, yylval_text_size);
	}

	/* if excessive length caused by prefix, get another token */
	if (fSingle && token->leng > max_token_len)
	    cls = get_token(token);
    }

    return cls;
}

token_t parse_new_token(word_t *token)
{
    token_t cls = NONE;
    unsigned char *cp;
    bool done = false;

    /* If saved IPADDR, truncate last octet */
    if ( block_on_subnets && save_class == IPADDR )
    {
	byte *t = xmemrchr(ipsave->u.text, '.', ipsave->leng);
	if (t == NULL)
	    save_class = NONE;
	else
	{
	    ipsave->leng = (uint) (t - ipsave->u.text);
	    token_set( token, ipsave->u.text, ipsave->leng);
	    cls = save_class;
	    done = true;
	}
    }

    while (!done) {
	uint leng;
	byte *text;

	cls = (*lexer->yylex)();

	token->leng = lexer->get_parser_token(&token->u.text);
	Z(token->u.text[token->leng]);	/* for easier debugging - removable */

	leng = token->leng;
	text = token->u.text;

	if (DEBUG_TEXT(2)) {
	    word_puts(token, 0, dbgout);
	    fputc('\n', dbgout);
	}
 
	if (cls == NONE) /* End of message */
	    break;

	switch (cls) {

	case EOH:	/* end of header - bogus if not empty */
	    if (leng > max_token_len)
		continue;

	    if (msg_state->mime_type == MIME_MESSAGE)
		mime_add_child(msg_state);
	    if (leng == 1)
		continue;
	    else {	/* "spc:invalid_end_of_header" */
		token_copy( &yylval, nonblank_line);
		done = true;
	    }
	    break;

	case BOUNDARY:	/* don't return boundary tokens to the user */
	    continue;

	case VERP:	/* Variable Envelope Return Path */
	{
	    byte *st = (byte *)text;
	    byte *in;
	    byte *fst = NULL;
	    byte *lst = NULL;

	    for (in = st; *in != '\0'; in += 1) {
		if (*in == '-') {
		    if (fst == NULL)
			fst = in;
		    lst = in;
		}
	    }

	    if (fst != NULL && lst != NULL && lst - fst  > 3) {
		byte *ot = fst;
		*ot++ = '-';
		*ot++ = '#';
		for (in = lst; *in != '\0'; in += 1, ot += 1)
		    *ot = *in;
		token->leng = leng = (uint) (ot - st);
	    }
	    Z(token->u.text[token->leng]);	/* for easier debugging - removable */
	}
	break;

	case HEADKEY:
	{
	    if (!header_line_markup || *text == '\0')
		continue;
	    else {
		const char *delim = strchr((const char *)text, ':');
		leng = (uint) (delim - (const char *)text);
		if (leng > max_token_len)
		    continue;
		token_set( &yylval, text, leng);
	    }
	}

	/*@fallthrough@*/

	case TOKEN:	/* ignore anything when not reading text MIME types */
	    if (leng < min_token_len)
		continue;

	/*@fallthrough@*/

	case MONEY:	/* 2 character money is OK */
	    if (leng > max_token_len)
		continue;

	    token->u.text = text;
	    token->leng = leng;

	    if (token_prefix == NULL) {
		switch (msg_state->mime_type) {
		case MIME_TEXT:
		case MIME_TEXT_HTML:
		case MIME_TEXT_PLAIN:
		case MIME_MULTIPART:
		    break;
		case MIME_MESSAGE:
		case MIME_APPLICATION:
		case MIME_IMAGE:
		    continue;
		default:
		    continue;
		}
	    }
	    break;

	case SUBJECT:
	    /* special token;  saved for formatted output, but not returned to bogofilter */
	    /** \bug: the parser MUST be aligned with lexer_v3.l! */
	    if (leng < max_token_len)
	    {
		while (!isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		while (isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		token_set( subject, text, leng);
	    }
	    continue;

	case MESSAGE_ID:
	    /* special token;  saved for formatted output, but not returned to bogofilter */
	    /** \bug: the parser MUST be aligned with lexer_v3.l! */
	    if (leng < max_token_len)
	    {
		while (!isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		while (isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		token_set( msg_id, text, leng);
	    }
	    continue;

	case QUEUE_ID:
	    /* special token;  saved for formatted output, but not returned to bogofilter */
	    /** \bug: the parser MUST be aligned with lexer_v3.l! */
	    if (*queue_id->u.text == '\0' &&
		leng < max_token_len )
	    {
		while (isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		if (memcmp(text, "id", 2) == 0) {
		    text += 2;
		    leng -= 2;
		}
		while (isspace(text[0])) {
		    text += 1;
		    leng -= 1;
		}
		if (text[0] == '<') {
		    text += 1;
		    leng -= 1;
		}
		if (text[leng-1] == '>') {
		    leng -= 1;
		}
		leng = min(queue_id->leng, leng);
		memcpy( queue_id->u.text, text, leng );
		Z(queue_id->u.text[leng]);
	    }
	    continue;

	case MESSAGE_ADDR:
	{
	    /* trim brackets */
	    text += 1;
	    leng -= 2;
	    Z(text[leng]);	/* for easier debugging - removable */
	    token_set( &yylval, text, leng);
	    /* if top level, no address, not localhost, .... */
	    if (token_prefix == w_recv &&
		msg_state->parent == NULL && 
		*msg_addr->u.text == '\0' &&
		strcmp((char *)text, "127.0.0.1") != 0)
	    {
		/* Not guaranteed to be the originating address of the message. */
		memcpy( msg_addr->u.text, yylval.u.text, min(msg_addr->leng, yylval.leng)+D );
		Z(msg_addr->u.text[yylval.leng]);
	    }
	}

	/*@fallthrough@*/

	case IPADDR:
	    if (block_on_subnets)
	    {
		int q1, q2, q3, q4;
		/*
		 * Trick collected by ESR in real time during John
		 * Graham-Cummings's talk at Paul Graham's spam conference
		 * in January 2003...  Some spammers know that people are
		 * doing recognition on spamhaus IP addresses.  They use
		 * the fact that HTML clients normally interpret IP addresses
		 * by doing a simple accumulate-and-shift algorithm; they
		 * add large random multiples of 256 to the quads to
		 * mask their origin.  Nuke the high bits to unmask the
		 * address.
		 */

		if (sscanf((const char *)text, "%d.%d.%d.%d", &q1, &q2, &q3, &q4) == 4)
		    /* safe because result string guaranteed to be shorter */
		    sprintf((char *)text, "%d.%d.%d.%d",
			    q1 & 0xff, q2 & 0xff, q3 & 0xff, q4 & 0xff);
		leng = strlen((const char *)text);

		token->u.text = text;
		token->leng = leng;

		token_copy( ipsave, token );

		save_class = IPADDR;

		return (cls);
	    }

	    token->u.text = text;
	    token->leng = leng;

	    break;

	case NONE:		/* nothing to do */
	    break;

	case MSG_COUNT_LINE:
	    msg_count_file = true;
	    multi_token_count = 1;
	    header_line_markup = false;
	    token_prefix = NULL;
	    lexer = &msg_count_lexer;
	    if (mbox_mode) {
		/* Allows processing multiple messages, **
		** but only a single file.              */
		reader_more = msgcount_more;
	    }
	    continue;

	case BOGO_LEX_LINE:
	    token_set( &yylval, text, leng);
	    done = true;
	    break;
	}

	if (DEBUG_TEXT(1)) {
	    word_puts(&yylval, 0, dbgout);
	    fputc('\n', dbgout);
	}

	/* eat all long words */
	if (token->leng <= max_token_len)
	    done = true;
    }

   if (!msg_count_file) {
	/* Remove trailing blanks */
	/* From "From ", for example */
	while (token->leng > 1 && token->u.text[token->leng-1] == ' ') {
	    token->leng -= 1;
	    token->u.text[token->leng] = (byte) '\0';
	}

	/* Remove trailing colon */
	if (token->leng > 1 && token->u.text[token->leng-1] == ':') {
	    token->leng -= 1;
	    token->u.text[token->leng] = (byte) '\0';
	}

	if (replace_nonascii_characters) {
	    /* replace nonascii characters by '?'s */
	    for (cp = token->u.text; cp < token->u.text+token->leng; cp += 1)
		*cp = casefold_table[*cp];
	}
    }

    return(cls);
}

/* save token in token array */

static void add_token_to_array(word_t *token)
{
    word_t *w = w_token_array[WRAP(tok_count)];

    w->leng = token->leng;
    memcpy(w->u.text, token->u.text, w->leng);
    Z(w->u.text[w->leng]);	/* for easier debugging - removable */

    if (DEBUG_MULTI(1))
	fprintf(stderr, "%s:%d  %2s  %2d %2d %p %s\n", __FILE__, __LINE__,
		"", tok_count, w->leng, w->u.text, w->u.text);

    tok_count += 1;
    init_token = 1;

    return;
}

static void build_token_from_array(word_t *token)
{
    int tok;

    const char *sep = "";
    uint  leng;
    byte *dest;

    leng = init_token;
    for ( tok = init_token; tok >= 0; tok -= 1 ) {
	uint idx = tok_count - 1 - tok;
	leng += strlen((char *) w_token_array[WRAP(idx)]->u.text);
    }

    if (leng > max_multi_token_len)
	leng = max_multi_token_len;

    token->leng = leng;
    token->u.text = dest = p_multi_buff;

    for ( tok = init_token; tok >= 0; tok -= 1 ) {
	uint  idx = tok_count - 1 - tok;
	uint  len = w_token_array[WRAP(idx)]->leng;
	byte *str = w_token_array[WRAP(idx)]->u.text;

	if (DEBUG_MULTI(1))
	    fprintf(stderr, "%s:%d  %2d  %2d %2d %p %s\n", __FILE__, __LINE__,
		    idx, tok_count, len, str, str);
	
	len = token_copy_leng((const char *)sep, leng, dest);
	leng -= len;
	dest += len;

	len = token_copy_leng((const char *)str, leng, dest);
	leng -= len;
	dest += len;

	sep = "*";
    }

    Z(token->u.text[token->leng]);	/* for easier debugging - removable */
    init_token += 1;			/* progress to next multi-token */

    return;
}

static uint token_copy_leng(const char *str, uint leng, byte *dest)
{
    uint len = strlen(str);
    if (leng < len)
	len  = leng;
    if (len != 0)
	memcpy(dest, str, len);
    return (uint) len;
}

void token_init(void)
{
    static bool fTokenInit = false;

    yyinit();

    if ( fTokenInit) {
	token_clear();
    }
    else {
	fTokenInit = true;

	if (max_multi_token_len == 0)
	    max_multi_token_len = (max_token_len+1) * multi_token_count + MAX_PREFIX_LEN;

	yylval_text_size = max_multi_token_len + MSG_COUNT_PADDING;

	yylval_text = (byte *) malloc( yylval_text_size+D );
	yylval.leng   = 0;
	yylval.u.text   = yylval_text;

	/* First IP Address in Received: statement */
	msg_addr = word_new( NULL, max_token_len );

	/* Message ID */
	msg_id = word_new( NULL, max_token_len * 3 );

	/* Subject */
	subject = word_new( NULL, max_token_len * 3 );

	/* Message's first queue ID */
	queue_id = word_new( NULL, max_token_len );

	ipsave = word_new( NULL, max_token_len );

	/* word_new() used to avoid compiler complaints */
	w_to   = word_news("to:");	/* To:          */
	w_from = word_news("from:");	/* From:        */
	w_rtrn = word_news("rtrn:");	/* Return-Path: */
	w_recv = word_news("rcvd:");	/* Received:    */
	w_head = word_news("head:");	/* Header:      */
	w_mime = word_news("mime:");	/* Mime:        */
	w_ip   = word_news("ip:");	/* ip:          */
	w_url  = word_news("url:");	/* url:         */
	nonblank_line = word_news(NONBLANK);

	/* do multi-word token initializations */
	init_token_array();
    }

    return;
}

void clr_tag(void)
{
    token_prefix = NULL;
    tok_count = 0;
}

void set_tag(const char *text)
{
    word_t *old_prefix = token_prefix;

    if (!header_line_markup)
	return;

    if (msg_state->parent != NULL &&
	msg_state->parent->mime_type == MIME_MESSAGE) {
	clr_tag();			/* don't tag if inside message/rfc822 */
	return;
    }

    switch (tolower((unsigned char)*text)) {
    case 'c':				/* CC: */
    case 't':
	token_prefix = w_to;		/* To: */
	break;
    case 'f':
	token_prefix = w_from;		/* From: */
	break;
    case 'h':
	if (msg_state->parent == NULL)
	    token_prefix = w_head;	/* Header: */
	else
	    token_prefix = w_mime;	/* Mime:   */
	break;
    case 'r':
	if (tolower((unsigned char)text[2]) == 't')
	    token_prefix = w_rtrn;	/* Return-Path: */
	else
	    token_prefix = w_recv;	/* Received: */
	break;
    default:
	fprintf(stderr, "%s:%d  invalid tag - '%s'\n",
		__FILE__, __LINE__,
		text);
	exit(EX_ERROR);
    }

    token_prefix_len = token_prefix->leng;
    assert(token_prefix_len <= MAX_PREFIX_LEN);

    if (DEBUG_LEXER(2)) {
	fprintf(dbgout,"--- set_tag(%s) -> prefix=", text);
	if (token_prefix)
	    word_puts(token_prefix, 0, dbgout);
	fputc('\n', dbgout);
    }

    /* discard tokens when prefix changes */
    if (old_prefix != NULL && old_prefix != token_prefix)
	tok_count = 0;

    return;
}

void set_msg_id(byte *text, uint leng)
{
    (void) leng;		/* suppress compiler warning */
    token_set( msg_id, text, msg_id->leng );
}

void set_subject(byte *text, uint leng)
{
    (void) leng;		/* suppress compiler warning */
    token_set( subject, text, subject->leng );
}

#define WFREE(n)	word_free(n); n = NULL

/* Cleanup storage allocation */
void token_cleanup()
{
    WFREE(w_to);
    WFREE(w_from);
    WFREE(w_rtrn);
    WFREE(w_recv);
    WFREE(w_head);
    WFREE(w_mime);
    WFREE(w_ip);
    WFREE(w_url);
    WFREE(nonblank_line);
	if (subject)
		WFREE(subject);
	if (msg_id)
		WFREE(msg_id);
	if (queue_id)
		WFREE(queue_id);
	if (msg_addr)
		WFREE(msg_addr);
	if (ipsave)
		WFREE(ipsave);

    token_clear();

    /* do multi-word token cleanup */
    free_token_array();
}

void token_clear()
{
    if (msg_addr != NULL)
    {
	*msg_addr->u.text = '\0';
	if (msg_id)
		*msg_id->u.text   = '\0';
	if (subject)
		*subject->u.text   = '\0';
	if (queue_id)
		*queue_id->u.text = '\0';
    }
}
