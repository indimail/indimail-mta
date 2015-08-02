/* $Id: base64.h 5267 2005-01-04 15:46:33Z m-a $ */

/** \file base64.h
 * prototypes and definitions for base64.c
 */

#ifndef	HAVE_BASE64_H
#define	HAVE_BASE64_H

#include "word.h"

/** decode base64 in-place, \return the count of decoded bytes in the \a
 * word */
uint	base64_decode(word_t *word /** string to decode in-place, will be modified */);
/** check if \a word contains conformant base64-encoded data */
bool	base64_validate(const word_t *word);

#endif	/* HAVE_BASE64_H */
