/* $Id: lexer_v3.l 6988 2013-01-20 14:02:48Z m-a $ */

%{
/*
 * NAME
 *   lexer_v3.l -- bogofilter's lexical analyzer for message headers
 *
 *   01/01/2003 - split out of lexer.l
 *
*/

/*
 * Our lexical analysis is different from Paul Graham's rules: 
 *
 * We throw away headers that are readily identifiable as dates.
 * We throw away all digit strings that don't look like IP address parts.
 * We thow away lines beginning with <tab>id<space> -- mailer UDs.
 *
 * These are optimizations to keep the token lists from bloating.
 * The big win is recognizing machine-generated unique IDs that
 * we'll never see again and shouldn't 
 *
 * We don't treat dot between two alphanumerics as a separator,
 * because we want to keep domain names and IP addresses together as 
 * recognizable units. 
 *
 * Having done the above, there isn't much need to recognize URLs.  
 * If a URL is a spam indicator, very likely any other URL from the
 * same site is as well, so the hostname part should be an adequate
 * statistical trigger.  
 *
 * LEXED_TOKENS, which are found in "msg-count" files need a special pattern
 * because they can be:
 *	1 - normal bogofilter tokens
 *	2 - url:xxx and from: tokens
 *	3 - mime boundaries
 */

/* 12 May 2003
 * Added Paul Graham's latest ideas on parsing.
 * (From http://www.paulgraham.com/better.html)
 *
 * 1. Case is preserved.
 *
 * 2. Exclamation points are constituent characters.
 *
 * 3. Periods and commas are constituents if they occur between two
 *    digits. This lets me get ip addresses and prices intact.
 *
 * 4. A price range like $20-25 yields two tokens, $20 and $25.
 *
 * 5. Tokens that occur within the To, From, and Return-Path
 *    lines, or within urls, get marked accordingly.
 *    For example. "foo" in the Subject line becomes "subj:foo".
*/

/* DR 08/29/2003:
**
** With flex-2.5.31 and '%option never-interactive noreject', file
** msg.dr.0118.base64 (in tests/bogofilter/inputs/split.d) parses
** incorrectly because line 24 isn't base64 decoded.
*/

#define YY_NO_INPUT

#include "common.h"

#include <ctype.h>
#include <stdlib.h>

#include "buff.h"
#include "charset.h"
#include "lexer.h"
#include "mime.h"		/* for mime_*() */
#include "msgcounts.h"
#include "textblock.h"
#include "token.h"
#include "xmalloc.h"

#define YY_DECL token_t yylex(void)
    YY_DECL;			/* declare function */

#define YY_INPUT(buf,result,max_size) result = yyinput((byte *)buf, result, max_size)
#define YY_EXIT_FAILURE EX_ERROR

#undef	stderr
#define	stderr	dbgout		/* for debug & -D options */

static int lineno;

#define	FLEX_VER(MAJ, MIN, SUB) ( MAJ * 1000 + MIN * 100 + SUB)

#ifndef	YY_FLEX_SUBMINOR_VERSION
#define	FLEX_VERSION_BF FLEX_VER(YY_FLEX_MAJOR_VERSION, YY_FLEX_MINOR_VERSION, 0)
#else
#define	FLEX_VERSION_BF FLEX_VER(YY_FLEX_MAJOR_VERSION, YY_FLEX_MINOR_VERSION, YY_FLEX_SUBMINOR_VERSION)
#endif

#if	FLEX_VERSION_BF != 2531
int yylineno;
#endif

/* Function Prototypes/Forward Declarations */

static word_t *yy_text(void);
static void html_char(void);
static void html_reorder(void);

static void url_char(void);

static void skip_to(char chr);
static void yy_unput(const byte *txt, uint len);

char yy_get_state(void);
void yy_set_state_initial(void);

static void header(void);

/* Function Definitions */

static word_t *yy_text(void)
{
    static word_t yyt;
    yyt.u.text = (byte *)yytext;
    yyt.leng = yyleng;
    return &yyt;
}

%}

%option warn
%option nodebug debug
%option align caseless 8bit
/***********************************************************************
 WARNING: The scanner must be interactive so as not to look ahead past
 \n = EOH and other header/body delimiters, else it will fail when MIME
 decoding, because part of the body has already been read ahead.
 ***********************************************************************/
%option interactive always-interactive
%option noreject noyywrap

UINT8		([01]?[0-9]?[0-9]|2([0-4][0-9]|5[0-5]))
IPADDR		{UINT8}\.{UINT8}\.{UINT8}\.{UINT8}
BCHARSNOSPC	[[:alnum:]()+_,-./:=?#\']
BCHARS		[[:alnum:]()+_,-./:=?#\' ]
MIME_BOUNDARY	{BCHARS}*{BCHARSNOSPC}

ID		<?[[:alnum:]\-\.]+>?
CHARSET		[[:alnum:]-]+
VERPID		[[:alnum:]#-]+[[:digit:]]+[[:alnum:]#-]+
MTYPE		[[:blank:]]*[[:alnum:]/-]*

NUM		[[:digit:]]+
NUM_NUM		\ {NUM}\ {NUM}
MSG_COUNT	^\".MSG_COUNT\"

FRONT_CHAR	[^[:blank:][:cntrl:][:digit:][:punct:]]
MID_CHAR	[^[:blank:][:cntrl:]:$*<>;=()&%#@+|/\\{}^\"?,\[\]]
BOGOLEX_CHAR	[^[:blank:][:cntrl:]   <>;=()&%#@+|/\\{}^\"?,\[\]]
BACK_CHAR	[^[:blank:][:cntrl:]:$*<>;=()&%#@+|/\\{}^\"?,\[\]._~\'\`\-]

TOKEN		{FRONT_CHAR}({MID_CHAR}*{BACK_CHAR})?

/*  RFC2047.2
    encoded-word = "=?" charset "?" encoding "?" encoded-text "?="
    charset = token    ; see section 3
    encoding = token   ; see section 4
    token = 1*<Any CHAR except SPACE, CTLs, and especials>
    especials = "(" / ")" / "<" / ">" / "@" / "," / ";" / ":" / "
		<"> / "/" / "[" / "]" / "?" / "." / "="
    encoded-text = 1*<Any printable ASCII character other than "?"
		      or SPACE>
		   ; (but see "Use of encoded-words in message
		   ; headers", section 5)
*/

/* 09/01/03
  Using "[^?]" in the pattern and validating the charset in 'C'
  reduces executable size by approx 290k.
  new: ENCODED_WORD =\?{CHARSET}\?[bq]\?[^?]*\?=
  old: ENCODED_WORD =\?{CHARSET}\?(b\?{BASE64}\|q\?{QP})\?=

  BASE64	[0-9a-zA-Z/+=]+
  QP		[!->@-~]+
*/

WHITESPACE	[[:blank:]\n]
NOTWHITESPACE	[^[:blank:]\n]

HTML_ENCODING	"&#"x?[[:xdigit:]]+";"
URL_ENCODING	"%"[[:xdigit:]][[:xdigit:]]

ENCODED_WORD	=\?{CHARSET}\?[bq]\?[^?\n]*\?=
ENCODED_TOKEN	({FRONT_CHAR}{MID_CHAR}*)?({ENCODED_WORD}{WHITESPACE}+)*{ENCODED_WORD}

/*
HTML_WI_COMMENTS	"<"[^>]*">"
HTML_WO_COMMENTS	"<"[^!][^>]*">"\|"<>"
*/

HTMLTOKEN		"<"[^>]*">"

/*
 * Generally, there are some html tags that cause an "eyebreak" and some
 * that do not. For example, the "P" tag or the "BR" tag cause a break,
 * and can be interpreted in place, while, the B (bold) tag does not.
 * No close tags seem to cause a break.
 * Comments do not.  This is an attempt to make an exhaustive list of
 * tags that cause an "eyebreak". When the exit tag also causes a break,
 * we include the /?. I believe this to be a complete list of tags that
 * can cause a formatting break.
 */

HBREAK		p|br|li|h[1-6]|hr|title|table|center|dd|dt|iframe|img|input|select|td|textarea|th|\/?(div|blockquote|pre|dir|dl|fieldset|legend|form|menu|ol|ul)

BREAKHTML	"<"({HBREAK}({WHITESPACE}[^>]*|""))">"

VERP		{TOKEN}-{VERPID}-{TOKEN}={TOKEN}@{TOKEN}

%s TEXT HTML BOGO_LEX
%s HTOKEN HDISCARD SCOMMENT LCOMMENT
%s PGP_HEAD PGP_BODY

%%

<INITIAL,BOGO_LEX>{MSG_COUNT}{NUM_NUM}		{ if (lineno == 0) { 
						      BEGIN BOGO_LEX; 
						      set_msg_counts_from_str(strchr(yytext, ' ') + 1); 
						  }
						  return MSG_COUNT_LINE;
						}
<BOGO_LEX>^\"{BOGOLEX_CHAR}+\"{NUM_NUM}$	{ return BOGO_LEX_LINE; }
<BOGO_LEX>\n					{ lineno += 1; }

<INITIAL>{ENCODED_TOKEN}			{ word_t *raw = yy_text();
						  word_t *txt = text_decode(raw);
						  yy_unput(txt->u.text, txt->leng);
						}

<INITIAL>^(To|CC|From|Return-Path|Received):	{ set_tag(yytext); }
<INITIAL>^Content-(Transfer-Encoding|Type|Disposition):{MTYPE}	{ mime_content(yy_text()); skip_to(':'); header(); return TOKEN; }

<INITIAL>^Subject:.*				{ /* save token for logging */
						  int off = 9;
						  while(isspace((unsigned char)yytext[off]) && off < yyleng)
						      off++;
						  set_subject((unsigned char *)(yytext+off), yyleng-off);
						  header();
						  return HEADKEY;
						}
<INITIAL>^Message-ID:.*				{ /* save token for logging */
						  unsigned long off = 11;
						  while(isspace((unsigned char)yytext[off]) && off < yyleng)
						      off++;
						  set_msg_id((unsigned char *)(yytext+off), yyleng-off);
						  header();
						  return HEADKEY;
						}
<INITIAL>^(Delivery-)?Date:.*			|
<INITIAL>^Resent-Message-ID:.*			|
<INITIAL>^(In-Reply-To|References):.* 		{ header(); return HEADKEY; }

<INITIAL>boundary=[ ]*\"?{MIME_BOUNDARY}\"?	{ mime_boundary_set(yy_text()); }
<INITIAL>charset=\"?{CHARSET}\"?		{ got_charset(yytext); skip_to('='); header(); return TOKEN; }

<INITIAL>(file)?name=\"?			/* ignore */
<INITIAL>[[:blank:]]id{WHITESPACE}+{ID}		{ return QUEUE_ID; }

 /**********************************************************************
 WARNING: Do NOT add header (<INITIAL>) rules that require characters
 beyond a LF character (\n) - doing so will make the parser read ahead
 parts of the body in the wrong MIME decoding mode and goof up
 seriously.
 **********************************************************************/

<INITIAL>^\n					{ enum mimetype type = get_content_type();
						  have_body = true;
						  msg_header = false;
						  clr_tag();
						  switch (type) { 
						  case MIME_TEXT_HTML:	BEGIN HTML; break;
						  case MIME_MESSAGE:	yy_set_state_initial(); break;
						  default:		BEGIN TEXT; 
						  }
						  if (DEBUG_LEXER(1))
						      fprintf(dbgout, "*** end of header\n");
						  return EOH;
						}
<INITIAL>^{TOKEN}				{ header(); return TOKEN; }
<INITIAL>\n					{ lineno += 1; }

<INITIAL>{VERP} 				{ skip_to('='); return VERP; }

^-----BEGIN\ PGP\ SIGNATURE-----$		{ BEGIN PGP_HEAD;
						  yy_unput((byte *)yytext, yyleng);
						}
<PGP_HEAD>^\n					{ BEGIN PGP_BODY; }
<PGP_BODY>{TOKEN}				{ /* ignore */ }
^-----END\ PGP\ SIGNATURE-----$ 		{ BEGIN TEXT;
						  yy_unput((byte *)yytext, yyleng);
						}
^--{MIME_BOUNDARY}(--)?$			{ if (got_mime_boundary(yy_text())) {
						      yy_set_state_initial();
						      return BOUNDARY;
						  } else {
						      yyless(2);
						  }
						}

  /* This has to match just as much or more than the below rules, so as to be the 
     controlling rule. */
<HTML>{TOKEN}({HTMLTOKEN}*{BREAKHTML}+{HTMLTOKEN}*.?|{HTMLTOKEN}+{WHITESPACE})		{ 
    			char *chr = memchr(yytext, '<', yyleng);	/* find start of html tag */
			size_t len = chr - yytext;
			yyless(len);
			return TOKEN;
			}

<HTML>{TOKEN}{HTMLTOKEN}+/{NOTWHITESPACE}	{ html_reorder(); }

<HTML>"<!--"					{ BEGIN SCOMMENT; }
<HTML>"<!"					{ BEGIN LCOMMENT; }
<HTML>"<"(a|img|font){WHITESPACE}		{ BEGIN HTOKEN; }
<HTML>"<"					{ BEGIN HDISCARD; }	/* unknown tag */

<HTOKEN>{TOKEN}					{ return TOKEN; }
<HDISCARD,LCOMMENT,SCOMMENT>{TOKEN}		{ /* discard innards of html tokens and comments */ }

<HTOKEN,HDISCARD,LCOMMENT>">"			{ BEGIN HTML; }	/* end of tag, loose comment; return to normal html processing */
<SCOMMENT>"-->"					{ BEGIN HTML; }	/* end of strict comment; return to normal html processing */
"<"\!DOCTYPE\ HTML\ PUBLIC\ .*">" 		{ BEGIN HTML; }

{IPADDR}					{ return IPADDR;}
"\["({IPADDR})"\]"				{ return MESSAGE_ADDR;}

{TOKEN}						{ return TOKEN;}

<HTML>{TOKEN}?{HTML_ENCODING}			{ html_char(); }	/* process escaped chars, eg '&#101;' is 'a' */
<HTOKEN>"/"[^/[:blank:]\n%]*{URL_ENCODING}+	{ url_char(); }		/* process escaped chars, eg '%61'    is 'a' */

\${NUM}(\.{NUM})?				{ return MONEY;}	/* Dollars and cents */

.						/* ignore character */
\n						{ lineno += 1; clr_tag(); }
<<EOF>>						{ return NONE; }
%%

static void header(void)
{
    set_tag("Header");
}

void lexer_v3_init(FILE *fp)
{
    lineno = 0;
    have_body = false;
    yy_set_state_initial();
    yyrestart(fp);
}

static void skip_to(char chr)
{
    size_t len = strchr(yytext, chr) - yytext;
    yyless(len);
}

static void html_reorder(void)
{
    char *chr = memchr(yytext, '<', yyleng);	/* find start of html tag */
    size_t len = chr - yytext;
    char *yycopy = xmalloc(yyleng + 1); 	/* +1 for NUL byte below */

    memcpy(yycopy, yytext+len, yyleng-len);	/* copy tag to start of buffer */
    memcpy(yycopy+yyleng-len, yytext, len);	/* copy leading text to end of buffer */
    yycopy[yyleng] = '\0';			/* for debugging */

    yy_unput((byte *)yycopy, yyleng);
    xfree(yycopy);
}

static int xtoi(char *in, size_t len)
{
    int val = 0;
    while (isxdigit((byte) *in) && (len-- > 0)) {
	char c = *in++;
	val <<= 4;
	val += isdigit((byte)c)
	    ? (c - '0')
	    : (tolower((byte)c) - 'a' + 10);
    }
    return val;
}

static void html_char(void)
{
    char *txt = strstr(yytext, "&#");	/* find decodable char */
    size_t len = txt - yytext;
    int  val;
    char *yycopy = NULL;

    if (len != 0) {
	yycopy = xmalloc(yyleng + 1);	/* +1 for NUL byte below */
	memcpy(yycopy, yytext, yyleng);	/* copy tag to start of buffer */
	yycopy[yyleng] = '\0';		/* for debugging */
    }

    txt += 2;
    val = isdigit((byte) *txt) ? atoi(txt) : xtoi(txt+1, 4);

    /* xtoi() limits conversion to 4 characters */
    /* atoi() limits value to 0x7fffffff, i.e. 2147483647 */
    /* no problem on linux */

    if ((val > 0) && (val < 256) && 
	isprint(val)) {			/* use it if printable */
	yyunput(val, yytext);
	yyleng = len;			/* adjust len to pre-char count */
    }
    else {
	if (yycopy)
	    yycopy[yyleng-1] = ' ';	/* prevents parsing loop */
    }

    if (yycopy != NULL) {
	yy_unput((byte *)yycopy, yyleng);
	xfree(yycopy);
    }
}

static void url_char(void)
{
    char *src, *dst;
    src = dst = yytext;

    while (src < yytext + yyleng) {
	char c = *src++;
	if (c == '%') {
	    c = xtoi(src, 2);
	    src += 2;
	}
	*dst++ = c;
    }
    while (dst > yytext) {
	yyunput(*--dst, yytext);
    }
}

static void yy_unput(const byte *txt, uint len)
{
    while (len-- > 0)
	yyunput(txt[len], yytext);
}

char yy_get_state()
{
    switch (YYSTATE) {
    case INITIAL:  return 'i';
    case TEXT:     return 't';
    case HTML:
    case HTOKEN:   return 'h';
    case SCOMMENT: return 's';
    case LCOMMENT: return 'l';
    default:       return 'o';
    }
}

void yy_set_state_initial(void)
{
    BEGIN INITIAL;
    msg_header = true;
    header();

    if (DEBUG_LEXER(1))
	fprintf(dbgout, "BEGIN INITIAL\n");

#ifdef	FLEX_DEBUG
    yy_flex_debug = BOGOTEST('L');
#endif
}

long lexer_v3_get_token(byte **output)
{
	*output = (byte *)yytext;
	return yyleng;
}

/*
 * The following sets edit modes for GNU EMACS
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * End:
 */
