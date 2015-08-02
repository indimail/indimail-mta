/* $Id: buff.h 6894 2010-03-23 17:32:46Z m-a $ */

/** \file buff.h
 * declarations and type definitions for buff.c
 *
 * \author David Relson <relson@osagesoftware.com>
 */

#ifndef	BUFF_H
#define	BUFF_H

#include "word.h"

/** Type to store an arbitrary string with actual length,
 * maximum capacity and a pointer to the last byte read.
 */
typedef struct {
    word_t t;		/**< the string contained here */
    uint   read;	/**< start of last read */
    uint   size;	/**< capacity */
} buff_t;

/** allocate a new buff_t and initialize it from \a buff, \a used
 * and set the capacity to \a size, without allocating memory for the
 * actual string */
extern buff_t  *buff_new(byte *buff, uint used, uint size);

/** initialize an existing buff_t from \a buff, \a used
 * and set the capacity to \a size, without allocating memory for the
 * actual string */
extern buff_t  *buff_init(buff_t *self, byte *buff, uint used, uint size);

/** free the buff_t \a self, without freeing the string */
extern void 	buff_free(buff_t *self);

/** append word \a in to the existing buffer \a self, reallocating more
 * room if necessary */
extern int	buff_add(buff_t *self, word_t *in);

/** read up to a line feed or exhaustion of the buffer capacity,
 * whichever comes first, from the stdio stream \a in into the buff_t \a
 * self. */
#define buff_fgetsl(self, in) buff_fgetsln(self, in, UINT_MAX)

/** read up to \a maxlen characters, a line feed or exhaustion of the
 * buffer capacity, whichever comes first, from the stdio stream \a in
 * into the buff_t \a self. */
extern int	buff_fgetsln(buff_t *self, FILE *in, uint maxlen);

/** print the unread part of the buff_t \a self to the stdio stream fp
 * by means of word_puts(), which see for meaning of \a width.  */
extern void 	buff_puts(const buff_t *self, /**< buff struct to print */
	uint width, /**< passed verbatim to word_puts() */
	FILE *fp /**< stdio.h stream to print to */);

extern void	buff_shift(buff_t *self, uint start, uint length);

#endif	/* BUFF_H */
