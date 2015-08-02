/* $Id: mime.c 6914 2010-07-05 15:22:00Z m-a $ */

/**
 * \file mime.c - lexer MIME processing
 *
 * NOTES:
 *
 * RFC2045:
 * Header fields occur in at least two contexts:
 *-# As part of a regular RFC 822 message header.
 *-# In a MIME body part header within a multipart construct.
 *
 * \author Matthias Andree <matthias.andree@gmx.de>
 * \author David Relson <relson@osagesoftware.com>
 * \author Gyepi Sam <gyepi@praxis-sw.com>
 */

#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include "base64.h"
#include "lexer.h"
#include "mime.h"
#include "qp.h"
#include "uudecode.h"
#include "xstrdup.h"
#include "xmalloc.h"

/* Global Variables */

mime_t *msg_state = NULL;
static mime_t *mime_stack_top = NULL;
static mime_t *mime_stack_bot = NULL;

/** MIME media types (or prefixes thereof) that we detect. */
static const struct type_s {
    enum mimetype type;	/**< internal representation of MIME type */
    const char *name;	/**< prefix of MIME type to match */
} mime_type_table[] = {
    { MIME_TEXT_HTML,	"text/html"	},
    { MIME_TEXT_PLAIN,	"text/plain"	},
    { MIME_TEXT,	"text"		}, /* NON-COMPLIANT; should be "text/"*/
    { MIME_APPLICATION,	"application/"	},
    { MIME_MESSAGE,	"message/"	},
    { MIME_MULTIPART,	"multipart/"	},
    { MIME_IMAGE,	"image/"	},
    { MIME_AUDIO,	"audio/"	},
    { MIME_VIDEO,	"video/"	},
};

/** MIME encodings that we detect. */
static const struct encoding_s {
    enum mimeencoding encoding;	/**< internal representation of encoding */
    const char *name;		/**< encoding name to match */
} mime_encoding_table[] = {
    { MIME_7BIT,	"7BIT" },
    { MIME_8BIT,	"8BIT" },
    { MIME_BINARY,	"BINARY" },
    { MIME_QP,		"QUOTED-PRINTABLE" },
    { MIME_BASE64,	"BASE64" },
    { MIME_UUENCODE,	"X-UUENCODE" },
};

/** MIME content dispositions that we detect. */
static const struct disposition_s {
    enum mimedisposition disposition;	/**< internal representation of disposition */
    const char *name;			/**< disposition name to match */
} mime_disposition_table[] = {
    { MIME_INLINE,	"inline" },
    { MIME_ATTACHMENT,	"attachment" },
};

/** properties of a MIME boundary */
typedef struct {
    bool is_valid;	/**< valid boundary of an enclosing MIME container */
    bool is_final;	/**< boundary is a closing one (two trailing dashes) */
    int depth;		/**< stack level the boundary was found at */
} boundary_t;

/* Function Prototypes */

static void mime_disposition(word_t * text);
static void mime_encoding(word_t * text);
static void mime_type(word_t * text);

static void mime_push(mime_t * parent);
static void mime_pop(void);

/* Function Definitions */

#if	0			/* Unused */
const char *mime_type_name(enum mimetype type)
{
    size_t i;
    for (i = 0; i < COUNTOF(mime_type_table); i += 1) {
	struct type_s *typ = mime_type_table + i;
	if (typ->type == type)
	    return typ->name;
    }
    return "unknown";
}
#endif

static const char *str_mime_type(enum mimetype m)
{
    switch (m) {
	case MIME_TYPE_UNKNOWN:	return "unknown";
	case MIME_MULTIPART:	return "multipart/*";
	case MIME_MESSAGE:	return "message/*";
	case MIME_TEXT:		return "text/*";
	case MIME_TEXT_PLAIN:	return "text/plain";
	case MIME_TEXT_HTML:	return "text/html";
	case MIME_APPLICATION:	return "application/*";
	case MIME_IMAGE:	return "image/*";
	case MIME_AUDIO:	return "audio/*";
	case MIME_VIDEO:	return "video/*";
}
    return "INTERNAL_ERROR";
}

static const char *str_mime_enc(enum mimeencoding e)
{
    switch (e) {
	case MIME_ENCODING_UNKNOWN:	return "unknown";
	case MIME_7BIT:			return "7bit";
	case MIME_8BIT:			return "8bit";
	case MIME_BINARY:		return "binary";
	case MIME_QP:			return "quoted-printable";
	case MIME_BASE64:		return "base64";
	case MIME_UUENCODE:		return "x-uuencode";
    }
    return "INTERNAL_ERROR";
}

/** Dump the current MIME boundary stack. For debugging. */
#ifndef	NODEBUG
static void mime_stack_dump(void)
{
    mime_t *ptr;
    fprintf(dbgout, "**** MIME stack is:\n");

    for (ptr = mime_stack_top; ptr != NULL; ptr = ptr->child)
    {
	fprintf(dbgout, "**** %3d type: %-16s enc: %-16s chr: %-8.8s bnd: %s\n",
		ptr->depth,
		str_mime_type(ptr->mime_type),
		str_mime_enc(ptr->mime_encoding),
		ptr->charset,
		ptr->boundary ? ptr->boundary : "NIL");
    }
}
#endif

static void mime_init(mime_t * parent)
{
    msg_state->mime_type = MIME_TEXT;
    msg_state->mime_encoding = MIME_7BIT;
    msg_state->boundary = NULL;
    msg_state->boundary_len = 0;
    msg_state->parent = parent;
    msg_state->charset = xstrdup("US-ASCII");
    msg_state->depth = (parent == NULL) ? 0 : msg_state->parent->depth + 1;
    msg_state->child  = NULL;
    msg_state->mime_dont_decode = false;
    msg_state->mime_disposition = MIME_DISPOSITION_UNKNOWN;

    if (parent)
	parent->child = msg_state;

    return;
}

static void mime_free(mime_t * t)
{
    if (t == NULL)
	return;

    if (mime_stack_bot == t)
	mime_stack_bot = t->parent;

    if (mime_stack_top == t)
	mime_stack_top = NULL;

    if (t->boundary) {
	xfree(t->boundary);
	t->boundary = NULL;
    }

    if (t->charset) {
	xfree(t->charset);
	t->charset = NULL;
    }

    t->parent = NULL;

    xfree(t);
}

void mime_cleanup()
{
    if (msg_state == NULL)
	return;

    while (mime_stack_top->parent)
	mime_pop();
    mime_pop();
    msg_state = NULL;

    mime_stack_top = NULL;
    mime_stack_bot = NULL;
}

static void mime_push(mime_t * parent)
{
    msg_state = (mime_t *) xmalloc(sizeof(mime_t));

    if (parent == NULL)
	mime_stack_top = msg_state;

    mime_stack_bot = msg_state;

    mime_init(parent);

    if (DEBUG_MIME(1))
	fprintf(dbgout, "*** mime_push. stackp: %d\n", msg_state->depth);

    if (DEBUG_MIME(2))
	mime_stack_dump();
}

static void mime_pop(void)
{
    if (DEBUG_MIME(1))
	fprintf(dbgout, "*** mime_pop. stackp: %d\n", msg_state->depth);

    if (msg_state)
    {
	mime_t *parent = msg_state->parent;

	mime_free(msg_state);

	msg_state = parent;
	if (msg_state)
	    msg_state->child = NULL;
    } else {
	fprintf(stderr, "Attempt to underflow mime stack\n");
    }

    if (DEBUG_MIME(2))
	mime_stack_dump();
}

/**
 * check if the media type of the MIME entity \a m is a container type
 * (message/anything or multipart/anything)
 */
static bool is_mime_container(mime_t * m)
{
    return (m
	    && ((m->mime_type == MIME_MESSAGE)
		|| (m->mime_type == MIME_MULTIPART)));
}

void mime_reset(void)
{
    if (DEBUG_MIME(0))
	fprintf(dbgout, "*** mime_reset\n");

    mime_cleanup();

    mime_push(NULL);
}

void mime_add_child(mime_t * parent)
{
    mime_push(parent);
}

/**
 * Check if the line given in \a boundary is a boundary of one of the
 * outer MIME containers and store the results in \a b.
 * container we'd previously seen. \return a copy of b->is_valid
 */
static bool get_boundary_props(const word_t * boundary, /**< input line */
	boundary_t * b /*@out@*/ /**< output properties, must be pre-allocated by caller */)
{
    mime_t *ptr;
    const byte *buf = boundary->u.text;
    size_t blen = boundary->leng;

    b->is_valid = false;

    /* a boundary line must begin with two dashes */
    if (blen > 2 && buf[0] == '-' && buf[1] == '-') {

	/* strip EOL characters */
	while (blen > 2 &&
	       (buf[blen - 1] == '\r' || buf[blen - 1] == '\n'))
	    blen--;

	/* skip initial -- */
	buf += 2;
	blen -= 2;

	/* skip and note ending --, if any */
	if (blen > 2 && buf[blen - 1] == '-' && buf[blen - 2] == '-') {
	    b->is_final = true;
	    blen -= 2;
	} else {
	    b->is_final = false;
	}

	/* search stack for matching boundary, in reverse order */
	for (ptr = mime_stack_bot; ptr != NULL; ptr = ptr->parent)
	{
	    if (is_mime_container(ptr)
		&& ptr->boundary != NULL
		&& ptr->boundary_len == blen
		&& (memcmp(ptr->boundary, buf, blen) == 0))
	    {
		b->depth = ptr->depth;
		b->is_valid = true;
		break;
	    }
	}
    }

    return b->is_valid;
}

bool mime_is_boundary(word_t * boundary)
{
    boundary_t b;
    return get_boundary_props(boundary, &b);
}

bool got_mime_boundary(word_t * boundary)
{
    mime_t *parent = NULL;
    boundary_t b;

    get_boundary_props(boundary, &b);

    if (!b.is_valid)
	return false;

    if (DEBUG_MIME(0))
	fprintf(dbgout,
		"*** got_mime_boundary:  stackp: %d, boundary: '%s'\n",
		mime_stack_top->depth, boundary->u.text);

    if (msg_state != NULL)
    {
	/* This handles explicit and implicit boundaries - pop stack
	 * until we reach the boundary level on the stack */
	while (msg_state->depth > b.depth)
	    mime_pop();

	/* explicit end boundary */
	if (b.is_final)
	    return true;

	parent = is_mime_container(msg_state) ? msg_state : msg_state->parent;
    }

    if (parent != NULL)
	mime_push(parent); /* push for the next part */
    else
	mime_push(msg_state); /* push for the next part */
    return true;
}

/** Skip leading whitespace from t.
 * \return - pointer to first non-whitespace character,
 *         - NULL if the string is all whitespace or empty */
static const byte *skipws(
	const byte * t, /**< string to find non-whitespace in */
	const byte * e  /**< pointer to the byte after the last byte in \a t */)
{
    while (t < e && (*t == ' ' || *t == '\t'))
	t++;
    if (t < e)
	return t;
    return NULL;
}

/**
 * get next MIME word, \return malloc'd NUL-terminated string containing
 * a copy of the word, or NULL when none found. It is the caller's
 * responsibility to xfree() the returned string!
 */
static byte *getword(
	const byte * t, /**< string to extract word from */
	const byte * e  /**< pointer to byte after last byte in \a t */)
{
    int quote = 0;
    int l;
    const byte *ts;
    byte *n;

    t = skipws(t, e);
    if (!t)
	return NULL;
    if (*t == '"') {
	quote++;
	t++;
    }
    ts = t;
    while ((t < e) && (quote ? *t != '"' : (*t != ' ' && *t != '\t'))) {
	t++;
    }
    l = t - ts;
    n = (byte *) xmalloc(l + 1);
    memcpy(n, ts, l);
    n[l] = (byte) '\0';
    return n;
}

void mime_content(word_t * text)
{
    char *key = (char *) text->u.text;
    switch (tolower((unsigned char)key[9])) {
    case 'r':			/*  Content-Transfer-Encoding: */
	mime_encoding(text);
	break;
    case 'y':			/*  Content-Type: */
	mime_type(text);
	break;
    case 'i':			/*  Content-Disposition: */
	mime_disposition(text);
	break;
    }
}

static void mime_disposition(word_t * text)
{
    size_t i;
    const size_t l = sizeof("Content-Disposition:") - 1;
    byte *w = getword(text->u.text + l, text->u.text + text->leng);

    if (!w)
	return;

    msg_state->mime_disposition = MIME_DISPOSITION_UNKNOWN;
    for (i = 0; i < COUNTOF(mime_disposition_table); i += 1) {
	const struct disposition_s *dis = mime_disposition_table + i;
	if (strcasecmp((const char *)w, dis->name) == 0) {
	    msg_state->mime_disposition = dis->disposition;
	    if (DEBUG_MIME(1))
		fprintf(dbgout, "*** mime_disposition: %s\n", text->u.text);
	    break;
	}
    }

    if (DEBUG_MIME(0)
	&& msg_state->mime_disposition == MIME_DISPOSITION_UNKNOWN)
	fprintf(stderr, "Unknown mime disposition - '%s'\n", w);

    xfree(w);

    return;
}

/*********
**
** RFC2045, Section 6.1.  Content-Transfer-Encoding Syntax
**
**     encoding := "Content-Transfer-Encoding" ":" mechanism
**
**     mechanism := "7bit" / "8bit" / "binary" /
**                  "quoted-printable" / "base64" /
**                  ietf-token / x-token
**
*********/

static void mime_encoding(word_t * text)
{
    size_t i;
    const size_t l =  sizeof("Content-Transfer-Encoding:") - 1;
    byte *w = getword(text->u.text + l, text->u.text + text->leng);

    if (!w)
	return;

    msg_state->mime_encoding = MIME_ENCODING_UNKNOWN;
    for (i = 0; i < COUNTOF(mime_encoding_table); i += 1) {
	const struct encoding_s *enc = mime_encoding_table + i;
	if (strcasecmp((const char *)w, enc->name) == 0) {
	    msg_state->mime_encoding = enc->encoding;
	    if (DEBUG_MIME(1))
		fprintf(dbgout, "*** mime_encoding: %s\n", text->u.text);
	    break;
	}
    }

    if (DEBUG_MIME(0)
	&& msg_state->mime_encoding == MIME_ENCODING_UNKNOWN)
	fprintf(stderr, "Unknown mime encoding - '%s'\n", w);

    xfree(w);

    return;
}

static void mime_type(word_t * text)
{
    const struct type_s *typ;
    const size_t l = sizeof("Content-Type:") - 1;
    byte *w = getword(text->u.text + l, text->u.text + text->leng);

    if (!w)
	return;

    msg_state->mime_type = MIME_TYPE_UNKNOWN;
    for (typ = mime_type_table;
	 typ < mime_type_table + COUNTOF(mime_type_table); typ += 1) {
	if (strncasecmp((const char *)w, typ->name, strlen(typ->name)) == 0) {
	    msg_state->mime_type = typ->type;
	    if (DEBUG_MIME(1) || DEBUG_LEXER(1))
		fprintf(dbgout, "*** mime_type: %s\n", text->u.text);
	    break;
	}
    }
    if (DEBUG_MIME(0) && msg_state->mime_type == MIME_TYPE_UNKNOWN)
	fprintf(stderr, "Unknown mime type - '%s'\n", w);
    xfree(w);

    switch (msg_state->mime_type) {
    case MIME_TEXT:		return;	/* XXX: read charset */
    case MIME_TEXT_PLAIN:	return;	/* XXX: read charset */
    case MIME_TEXT_HTML:	return;
    case MIME_TYPE_UNKNOWN:	return;
    case MIME_MULTIPART:	return;	/* XXX: read boundary */
    case MIME_MESSAGE:		return;
    case MIME_APPLICATION:
    case MIME_IMAGE:
    case MIME_AUDIO:
    case MIME_VIDEO:		msg_state->mime_dont_decode = true;	return;
    }

    return;
}

void mime_boundary_set(word_t * text)
{
    byte *boundary = text->u.text;
    size_t blen = text->leng;

    if (DEBUG_MIME(1)) {
	int len = blen;
	if (blen > INT_MAX)
	    len = INT_MAX;
	fprintf(dbgout, "*** --> mime_boundary_set: %d '%-.*s'\n",
		msg_state->depth, len, boundary);
    }

    boundary = getword(boundary + strlen("boundary="), boundary + blen);
    xfree(msg_state->boundary);
    msg_state->boundary = (char *) boundary;
    msg_state->boundary_len = strlen((char *) boundary);

    if (DEBUG_MIME(1))
	fprintf(dbgout, "*** <-- mime_boundary_set: %d '%s'\n",
		msg_state->depth, boundary);

    return;
}

uint mime_decode(word_t * text)
{
    uint count = text->leng;

    /* early out for the identity codings */
    if (msg_state->mime_encoding == MIME_7BIT ||
	msg_state->mime_encoding == MIME_8BIT ||
	msg_state->mime_encoding == MIME_BINARY ||
	msg_state->mime_encoding == MIME_ENCODING_UNKNOWN)
	return count;

    if (DEBUG_MIME(3))
	fprintf(dbgout, "*** mime_decode %lu \"%-.*s\"\n",
		(unsigned long) count,
		count > INT_MAX ? INT_MAX : (int) (count - 1), text->u.text);

    /* Do not decode "real" boundary lines */
    if (mime_is_boundary(text) == true)
	return count;

    switch (msg_state->mime_encoding) {
    case MIME_QP:
	count = qp_decode(text, RFC2045);
	break;
    case MIME_BASE64:
	if (count > 4)
	    count = base64_decode(text);
	break;
    case MIME_UUENCODE:
	count = uudecode(text);
	break;
    case MIME_7BIT:
    case MIME_8BIT:
    case MIME_BINARY:
    case MIME_ENCODING_UNKNOWN:
	break;
    }

    return count;
}

enum mimetype get_content_type(void)
{
    return msg_state->mime_type;
}
