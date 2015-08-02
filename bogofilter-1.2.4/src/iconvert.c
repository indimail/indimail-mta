/* $Id: iconvert.c 6986 2012-12-07 21:34:05Z m-a $ */

/*****************************************************************************

NAME:
   iconvert.c -- provide iconv() support for bogofilter's lexer.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

/**
 ** Note: 01/07/05
 **
 ** "make check" changes:
 **
 **    t.systest - msg.3.txt
 **		0x92	It’s
 **
 **    t.lexer.mbx - spam.mbx
 **	  msg**4
 **		0xAE	Club®
 **	  msg**20
 **		0xA0	MyNetOffers 
 **		0x93	“Your 
 **		0x94	Account”
 **/

#include "common.h"

#include <stdlib.h>
#include <errno.h>

#include "buff.h"
#include "iconvert.h"

extern	iconv_t cd;

static void iconv_print_error(int err, buff_t *src)
{
    if (DEBUG_ICONV(1)) {
	const char *msg = NULL;
	switch (err) {
	case EILSEQ:		/* invalid multibyte sequence */
	    msg = "EILSEQ";
	    break;
	case EINVAL:		/* incomplete multibyte sequence */
	    msg = "EINVAL";
	    break;
	case E2BIG:		/* output buffer has no more room */
	    msg = "E2BIG";
	    break;
	}
	if (msg != NULL)
	    fprintf(dbgout, "err: %s (%d), tx: %p, rd: %d, ln: %d, sz: %d\n",
		    msg, err, src->t.u.text, src->read, src->t.leng, src->size);
    }
}

static void convert(iconv_t xd, buff_t *restrict src, buff_t *restrict dst)
{
    bool done = false;

    while (!done) {
	char * inbuf;
	size_t inbytesleft;

	char * outbuf;
	size_t outbytesleft;
	size_t count;

	inbuf = (char *)src->t.u.text + src->read;
	inbytesleft = src->t.leng - src->read;

	outbuf = (char *)dst->t.u.text + dst->t.leng;
	outbytesleft = dst->size - dst->read - dst->t.leng;

	if (outbytesleft == 0)
	    break;

	/*
	 * The iconv function converts one multibyte character at a time, and for
	 * each character conversion it increments *inbuf and decrements
	 * *inbytesleft by the number of converted input bytes, it increments
	 * *outbuf and decrements *outbytesleft by the number of converted output
	 * bytes, and it updates the conversion state contained in cd. The
	 * conversion can stop for four reasons:
	 */

	count = iconv(xd, (ICONV_CONST char **)&inbuf, &inbytesleft, &outbuf, &outbytesleft);

	/*
	 * 1. An invalid multibyte sequence is encountered
	 * in the input. In this case it sets errno to
	 * EILSEQ and returns (size_t)(-1). *inbuf is left
	 * pointing to the beginning of the invalid
	 * multibyte sequence.

	 * 2. The input byte sequence has been entirely
	 * converted, i.e. *inbytesleft has gone down to
	 * 0. In this case iconv returns the number of
	 * non-reversible conversions performed during
	 * this call.

	 * 3. An incomplete multibyte sequence is
	 * encountered in the input, and the input byte
	 * sequence terminates after it. In this case it
	 * sets errno to EINVAL and returns
	 * (size_t)(-1). *inbuf is left pointing to the
	 * beginning of the incomplete multibyte sequence.

	 * 4. The output buffer has no more room for the
	 * next converted character. In this case it sets
	 * errno to E2BIG and returns (size_t)(-1).

	 * A different case is when inbuf is NULL or *inbuf is
	 * NULL, but outbuf is not NULL and *outbuf is not
	 * NULL. In this case, the iconv function attempts to
	 * set cd's conversion state to the initial state and
	 * store a corresponding shift sequence at *outbuf. At
	 * most *outbytesleft bytes, starting at *outbuf, will
	 * be written. If the output buffer has no more room
	 * for this reset sequence, it sets errno to E2BIG and
	 * returns (size_t)(-1). Otherwise it increments
	 * *outbuf and decrements *outbytesleft by the number
	 * of bytes written.
	 * 
	 * A third case is when inbuf is NULL or inbuf is
	 * NULL, and outbuf is NULL or outbuf is NULL. In this
	 * case, the iconv function sets cd's conversion state
	 * to the initial state.
	 */

	if (count == (size_t)(-1)) {

	    int err = errno;

	    iconv_print_error(err, src);

	    switch (err) {
	    case EILSEQ:		/* invalid multibyte sequence */
	    case EINVAL:		/* incomplete multibyte sequence */
		if (outbytesleft == 0) {
                    done = true;
                    break;
		}
		/* copy 1 byte (or substitute a '?') */
		if (!replace_nonascii_characters)
		    *outbuf = *inbuf;
		else
		    *outbuf = '?';
		/* update counts and pointers */
		inbytesleft -= 1;
		outbytesleft -= 1;
		inbuf  += 1;
		outbuf += 1;
		break;

	    case E2BIG:			/* output buffer has no more room */
					/* TODO:  Provide proper handling of E2BIG */
		done = true;
		break;

	    default:
		break;
	    }
	}
	src->read = src->t.leng - inbytesleft;
	dst->t.leng = dst->size - dst->read - outbytesleft;

	if (src->read >= src->t.leng)
	    done = true;
    }

    Z(dst->t.u.text[dst->t.leng]);	/* for easier debugging - removable */

    if (DEBUG_ICONV(1) &&
	src->t.leng != src->read)
	fprintf(dbgout, "tx: %p, rd: %d, ln: %d, sz: %d\n",
		src->t.u.text, src->read, src->t.leng, src->size);
}

static void copy(buff_t *restrict src, buff_t *restrict dst)
{
    /* if conversion not available, use memcpy */
    dst->t.leng = min(dst->size, src->t.leng);
    memcpy(dst->t.u.text, src->t.u.text, dst->t.leng+D);
}

void iconvert(buff_t *restrict src, buff_t *restrict dst)
{
    if (cd == NULL)
	copy(src, dst);
    else
	convert(cd, src, dst);
}

void iconvert_cd(iconv_t xd, buff_t *restrict src, buff_t *restrict dst)
{
    if (xd == (iconv_t)-1)
	copy(src, dst);
    else
	convert(xd, src, dst);
}
