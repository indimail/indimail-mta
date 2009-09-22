/* $Id: mime.h 6278 2005-10-25 23:48:03Z relson $ */

/** \file mime.h
 * prototypes and definitions for mime.c
 */

#ifndef	HAVE_MIME_H
#define	HAVE_MIME_H

#include "buff.h"
#include "word.h"

enum mimetype {
    MIME_TYPE_UNKNOWN,
    MIME_MULTIPART,
    MIME_MESSAGE,
    MIME_TEXT,
    MIME_TEXT_PLAIN,
    MIME_TEXT_HTML,
    MIME_APPLICATION,
    MIME_IMAGE,
    MIME_AUDIO,
    MIME_VIDEO
};

enum mimeencoding {
    MIME_ENCODING_UNKNOWN,
    MIME_7BIT,
    MIME_8BIT,
    MIME_BINARY,
    MIME_QP,
    MIME_BASE64,
    MIME_UUENCODE
};

enum mimedisposition {
    MIME_DISPOSITION_UNKNOWN,
    MIME_ATTACHMENT,
    MIME_INLINE
};

typedef struct mime_t mime_t;

/** data element of the MIME stack */
struct mime_t {
    int depth;
    char *charset;
    char *boundary;	/**< only valid if mime_type is
			  MIME_MULTIPART or MIME_MESSAGE */
    size_t boundary_len;
    enum mimetype mime_type;
    bool mime_dont_decode;
    enum mimeencoding mime_encoding;
    enum mimedisposition mime_disposition;
    mime_t *parent;
    mime_t *child;	/* for mime_stack_dump() */
};

extern mime_t *msg_state;
extern mime_t *msg_top;

/** pop all elements from the stack until it is empty */
void mime_reset(void);

void mime_add_child(mime_t *parent);

/** set MIME boundary in current stack level to \a text */
void mime_boundary_set(word_t *text);

/** checks if \a text is a MIME boundary of an enclosing MIME container,
 *  pop stack so it remains on top, and pop it if a final boundary */
bool got_mime_boundary(word_t *text);

/** Parse \a text that is assumed to begin with "Content-" and set the
 *  top stack element's member variable accordingly. */
void mime_content(word_t *text);

/** Decode the line in \a buff in-place according to the current
 * Content-Transfer-Encoding, but do not decode boundary lines. \return
 * the new line length */
uint mime_decode(word_t *buff);

/** checks if \a boundary is a MIME boundary of an enclosing MIME
 * container, leave stack unchanged */
bool mime_is_boundary(word_t *boundary);

/** \return current mime_type */
enum mimetype get_content_type(void);

/** pop all elements from the MIME stack and reinitialize */
void mime_cleanup(void);

/** To be removed. */
void mime_type2(word_t * text);

#endif	/* HAVE_MIME_H */
