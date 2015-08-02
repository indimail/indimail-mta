/* $Id: lexer.c 6988 2013-01-20 14:02:48Z m-a $ */

/**
 * \file lexer.c
 * bogofilter's lexical analyzer (control routines)
 *
 * \date 2003-01-01 split out of lexer.l
 */

#include "common.h"

#include <ctype.h>
#include <stdlib.h>

#include "base64.h"
#include "bogoconfig.h"
#include "bogoreader.h"
#include "charset.h"
#include "error.h"
#ifndef	DISABLE_UNICODE
#include "convert_unicode.h"
#include "iconvert.h"
#endif
#include "lexer.h"
#include "memstr.h"
#include "mime.h"
#include "msgcounts.h"
#include "qp.h"
#include "textblock.h"
#include "token.h"
#include "word.h"
#include "xmalloc.h"

/* Global Variables */

extern int yylineno;
extern int yyleng;

bool msg_header = true;
bool have_body  = false;
lexer_t *lexer = NULL;

/* Local Variables */

static lexer_t v3_lexer = {
    yylex,
    lexer_v3_get_token
};

lexer_t msg_count_lexer = {
    read_msg_count_line,
    msg_count_get_token
};

/* Function Prototypes */

static int yy_get_new_line(buff_t *buff);
static int get_decoded_line(buff_t *buff);
static int skip_folded_line(buff_t *buff);

/* Function Definitions */

void lexer_init(void)
{
    mime_reset();
    token_init();
    lexer_v3_init(NULL);
    init_charset_table(charset_default);
}

static void lexer_display_buffer(buff_t *buff)
{
    fprintf(dbgout, "*** %2d %c%c %2ld ",
	    yylineno-1, msg_header ? 'h' : 'b', yy_get_state(),
	    (long)(buff->t.leng - buff->read));
    buff_puts(buff, 0, dbgout);
    if (buff->t.leng > 0 && buff->t.u.text[buff->t.leng-1] != '\n')
	fputc('\n', dbgout);
}

/**
 * Check for lines wholly composed of printable characters as they can
 * cause a scanner abort "input buffer overflow, can't enlarge buffer
 * because scanner uses REJECT"
 *
 * \bug this function must go, we need to fix the lexer
 */
static bool long_token(byte *buf, uint count)
{
    uint i;
    for (i=0; i < count; i += 1) {
	byte c = buf[i];
	/* 10/23/05 - fix SIGSEGV with msg.1023.6479.txt
	** evidently caused by 09/07/05 patch for 0.96.2
	*/
	if (c == '\0')
	    break;
	if ((iscntrl(c) || isspace(c) || ispunct(c)) && (c != '_'))
	    return false;
    }
    return true;
}

static int yy_get_new_line(buff_t *buff)
{
    int count = (*reader_getline)(buff);
    const byte *buf = buff->t.u.text;

    static size_t hdrlen = 0;
    if (hdrlen==0)
	hdrlen=strlen(spam_header_name);

    if (count > 0)
	yylineno += 1;

    if (count == EOF) {
	if (fpin == NULL || !ferror(fpin)) {
	    return YY_NULL;
	}
	else {
	    print_error(__FILE__, __LINE__, "input in flex scanner failed\n");
	    exit(EX_ERROR);
	}
    }

    /* Mime header check needs to be performed on raw input
    ** -- before mime decoding.  Without it, flex aborts:
    ** "fatal flex scanner internal error--end of buffer missed" */

    if (buff->t.leng > 2 &&
	buf[0] == '-' && buf[1] == '-' &&
	got_mime_boundary(&buff->t)) {
	yy_set_state_initial();
    }

    if (count >= 0 && DEBUG_LEXER(0))
	lexer_display_buffer(buff);

    /* skip spam_header ("X-Bogosity:") lines */
    while (msg_header
	   && count != EOF
/* don't skip if inside message/rfc822 */
	   && msg_state->parent == NULL
	   && memcmp(buff->t.u.text,spam_header_name,hdrlen) == 0) {
	count = skip_folded_line(buff);
    }

    return count;
}

static int get_decoded_line(buff_t *buff)
{
    int count;
    buff_t *linebuff;

#ifdef	DISABLE_UNICODE
    linebuff = buff;
#else
    if (encoding == E_RAW ||
	msg_state->mime_dont_decode ) {
	linebuff = buff;
    }
    else {
	static buff_t *tempbuff = NULL;

	if (tempbuff == NULL)
	    tempbuff = (buff_t *) calloc(sizeof(buff_t), 1);

	/* UTF-8 uses up to six octets per character.  Make input buffer
	 * sufficiently small that the UTF-8 text can fit in the output
	 * buffer */
	if (tempbuff->size < buff->size / 6) {
	    xfree(tempbuff->t.u.text);
	    tempbuff->size = buff->size / 6;
	    tempbuff->t.u.text = (byte *) xmalloc(tempbuff->size+D);
	}

	tempbuff->t.leng = tempbuff->read = 0;
	linebuff = tempbuff;
    }
#endif

    count = yy_get_new_line(linebuff);

    if (count == EOF) {
	if ( !ferror(fpin))
	    return YY_NULL;
	else {
	    print_error(__FILE__, __LINE__, "input in flex scanner failed\n");
	    exit(EX_ERROR);
	}
    }

    /* Save the text on a linked list of lines.
     * Note that we store fixed-length blocks here, not lines.
     * One very long physical line could break up into more
     * than one of these. */

    if (passthrough && count > 0)
	textblock_add(linebuff->t.u.text+linebuff->read, (size_t) count);

    if ( !msg_header && 
	 !msg_state->mime_dont_decode &&
	 msg_state->mime_type != MIME_TYPE_UNKNOWN)
    {
	word_t temp;
	uint decoded_count;

	temp.leng = (uint) count;
	temp.u.text = linebuff->t.u.text+linebuff->read;

	decoded_count = mime_decode(&temp);
	/*change buffer size only if the decoding worked */
	if (decoded_count != 0 && decoded_count < (uint) count) {
	    linebuff->t.leng -= (uint) (count - decoded_count);
	    count = (int) decoded_count;
	    if (DEBUG_LEXER(1))
		lexer_display_buffer(linebuff);
	}
    }

#ifndef	DISABLE_UNICODE
    if (encoding == E_UNICODE &&
	!msg_state->mime_dont_decode)
    {
	iconvert(linebuff, buff);
	/*
	 * iconvert, treating multi-byte sequences, can shrink or enlarge
	 * the output compared to its input.  Correct count.
	 */
	if (count > 0)
	    count = buff->t.leng;
    }
#endif

#ifdef EXCESSIVE_DEBUG
    /* debug */
    fprintf(dbgout, "%d: ", count);
    buff_puts(buff, 0, dbgout);
    fprintf(dbgout, "\n");
#endif

    /* CRLF -> NL */
    if (count >= 2) {
	byte *buf = buff->t.u.text;
	if (memcmp(buf + count - 2, CRLF, 2) == 0) {
	    count --;
	    *(buf + count - 1) = (byte) '\n';
	}
    }

    if (buff->t.leng < buff->size)     /* for easier debugging - removable */
	Z(buff->t.u.text[buff->t.leng]);  /* for easier debugging - removable */

    return count;
}

static int skip_folded_line(buff_t *buff)
{
    for (;;) {
	int count;
	buff->t.leng = 0;
	count = reader_getline(buff);
	yylineno += 1;
	/* only check for LWSP-char (RFC-822) aka. WSP (RFC-2822),
	 * these only include SP and HTAB */
	if (buff->t.u.text[0] != ' ' &&
	    buff->t.u.text[0] != '\t')
	    return count;
	/* Check for empty line which terminates message header */
	if (is_eol((char *)buff->t.u.text, count))
	    return count;
    }
}

int buff_fill(buff_t *buff, size_t used, size_t need)
{
    int cnt = 0;
    size_t leng = buff->t.leng;
    size_t size = buff->size;

    /* check bytes needed vs. bytes in buff */
    while (size - leng > 2 && need > leng - used) {
	/* too few, read more */
	int add = get_decoded_line(buff);
	if (add == EOF) return EOF;
	if (add == 0) break ;
	cnt += add;
	leng += add;
    }
    return cnt;
}

void yyinit(void)
{
    yylineno = 0;

    if ( !msg_count_file)
	lexer = &v3_lexer;
}

int yyinput(byte *buf, size_t used, size_t size)
/* input getter for the scanner */
{
    int cnt;
    int count = 0;
    buff_t buff;

    buff_init(&buff, buf, 0, (uint) size);

    /* After reading a line of text, check if it has special characters.
     * If not, trim some, but leave enough to match a max length token.
     * Then read more text.  This will ensure that a really long sequence
     * of alphanumerics, which bogofilter will ignore anyway, doesn't crash
     * the flex lexer.
     */

    while ((cnt = get_decoded_line(&buff)) != 0) {

	count += cnt;

	/* Note: some malformed messages can cause xfgetsl() to report
	** "Invalid buffer size, exiting."  ** and then abort.  This
	** can happen when the parser is in html mode and there's a
	** leading '<' but no closing '>'.
	**
	** The "fix" is to check for a nearly full lexer buffer and
	** discard most of it.
	*/

	/* if not nearly full */
	if (used < 1000 || used < size * 10)
	    break;

	if (count >= MAX_TOKEN_LEN * 2 && 
	    long_token(buff.t.u.text, (uint) count)) {
	    uint start = buff.t.leng - count;
	    uint length = count - max_token_len;
	    buff_shift(&buff, start, length);
	    count = buff.t.leng;
	}
	else
	    break;
    }

    if (msg_state &&
	msg_state->mime_dont_decode &&
	(msg_state->mime_disposition != MIME_DISPOSITION_UNKNOWN)) {
	return (count == EOF ? 0 : count);   /* not decode at all */
    }

#if	defined(CP866) && !defined(ENABLE_ICONV)
    /* EK -  decoding things like &#1084 and charset_table */
    count = decode_and_htmlUNICODE_to_cp866(buf, count);
#endif

    if (replace_nonascii_characters) {
	/* do non-ascii replacement */
	int i;
	for (i = 0; i < count; i++ )
	{
	    byte ch = buf[i];
	    buf[i] = charset_table[ch];
	}
    }

    if (DEBUG_LEXER(2))
	fprintf(dbgout, "*** yyinput(\"%-.*s\", %lu, %lu) = %d\n", count, buf, (unsigned long)used, (unsigned long)size, count);

    return (count == EOF ? 0 : count);
}

static char *charset_as_string(const byte *txt, const size_t len)
{
    static char *charset_text = NULL;
    static unsigned short charset_leng = 0;

    if (charset_text == NULL)
	charset_text = xmalloc(len+D);
    else {
	if (charset_leng < len) {
	    charset_leng = len;
	    charset_text = xrealloc(charset_text, charset_leng+D);
	}
    }

    memcpy(charset_text, txt, len);
    Z(charset_text[len]);			/* for easier debugging - removable */

    return charset_text;
}

word_t *text_decode(word_t *w)
{
    word_t *r = w;
    byte *const beg = w->u.text;		/* base pointer, fixed */
    byte *const fin = beg + w->leng;	/* end+1 position */

    byte *txt = (byte *) memstr(w->u.text, w->leng, "=?");	/* input position */
    uint size = (uint) (txt - beg);				/* output offset */

#ifndef	DISABLE_UNICODE
    size_t max = w->leng * 4;
    static buff_t * buf = NULL;
#endif

    if (txt == NULL)
	return r;

#ifndef	DISABLE_UNICODE
    if (encoding == E_UNICODE) {
	if (buf == NULL)
	    buf = buff_new(xmalloc(max+D), 0, max);
	r = &buf->t;				/* Use buf to return unicode result */

	buf->t.leng = 0;
	if (buf->size < max) {
	    buf->size = max;
	    buf->t.u.text = (byte *) xrealloc(buf->t.u.text, buf->size+D);
	}

	buf->t.leng = size;
	memcpy(buf->t.u.text, beg, size );
	Z(buf->t.u.text[buf->t.leng]);		/* for easier debugging - removable */
    }
#endif

    if (DEBUG_LEXER(2)) {
	fputs("**1**  ", dbgout);
	word_puts(w, 0, dbgout);
	fputs("\n", dbgout);
    }

    while (txt < fin) {
	byte *typ, *tmp, *end;
	uint len;
	bool adjacent;

	char *charset;

	txt += 2;
	typ = (byte *) memchr((char *)txt+1, '?', fin-txt);	/* Encoding type - 'B' or 'Q' */
	*typ++ = '\0';						/* nul terminate */

	charset = charset_as_string(txt, typ - txt - 1);

	tmp = typ + 2;						/* start of encoded word */
	end = (byte *) memstr((char *)tmp, fin-tmp, "?=");	/* last byte of encoded word  */
	len = end - tmp;

	w->u.text = tmp;				/* Start of encoded word */
	w->leng = len;				/* Length of encoded word */
	Z(w->u.text[w->leng]);			/* for easier debugging - removable */

	if (DEBUG_LEXER(2)) {
	    fputs("**2**  ", dbgout);
	    word_puts(w, 0, dbgout);
	    fputs("\n", dbgout);
	}

	switch (tolower(*typ)) {		/* ... encoding type */
	case 'b':
	    if (base64_validate(w))
		len = base64_decode(w);		/* decode base64 */
	    break;
	case 'q':
	    if (qp_validate(w, RFC2047))
		len = qp_decode(w, RFC2047);	/* decode quoted-printable */
	    break;
	}

	/* move decoded word to where the encoded used to be */
	if (encoding == E_RAW) {
	    memmove(beg+size, w->u.text, len);
	    size += len;			/* bump output pointer */
	    Z(beg[size]);			/* for easier debugging - removable */

	    if (DEBUG_LEXER(3))
		fprintf(dbgout, "**3**  %s\n", beg);
	}

#ifndef	DISABLE_UNICODE
	if (encoding == E_UNICODE) {
	    iconv_t cd;
	    buff_t  src;

	    /* convert 'word_t *w' to 'buff_t src' because
	    ** iconvert_cd() needs buff_t pointers
	    */
	    src.t.u.text = w->u.text;
	    src.t.leng = len;
	    src.read   = 0;
	    src.size   = len;

	    cd = bf_iconv_open( charset_unicode, charset );
	    iconvert_cd(cd, &src, buf);
	    iconv_close(cd);

	    if (DEBUG_LEXER(3)) {
		fputs("**4**  ", dbgout);
		word_puts(&buf->t, 0, dbgout);
		fputs("\n", dbgout);
	    }
	}
#endif

	txt = end + 2;	/* skip ?= trailer */
	if (txt >= fin)
	    break;

	/* check for next encoded word */
	end = (byte *) memstr((char *)txt, fin-txt, "=?");
	adjacent = end != NULL;

	/* clear adjacent flag if non-whitespace character found between
	 * adjacent encoded words */
	if (adjacent) {
	    tmp = txt;
	    while (adjacent && tmp < end) {
		if (*tmp && strchr(" \t\r\n", *tmp))
		    tmp += 1;
		else
		    adjacent = false;
	    }
	}

	/* we have a next encoded word and we've had only whitespace
	 * between the current and the next */
	if (adjacent)
	    /* just skip whitespace */
	    txt = end;
	else
	    /* copy everything that was between the encoded words */
	    while (txt < end) {
		if (encoding == E_RAW)
		    beg[size++] = *txt++;
#ifndef	DISABLE_UNICODE
		if (encoding == E_UNICODE)
		    buf->t.u.text[buf->t.leng++] = *txt++;
#endif
	    }
    }

    if (encoding == E_RAW) {
	r->u.text = beg;
	r->leng = size;
    }

    return r;
}

/*
 * The following sets edit modes for GNU EMACS
 * Local Variables:
 * mode:c
 * End:
 */
