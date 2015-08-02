/* $Id: collect.h 3605 2003-11-24 19:27:47Z relson $ */

/*****************************************************************************

NAME:
   collect.h -- global definitions for the collect library, part of bogofilter

******************************************************************************/

#ifndef COLLECT_H
#define COLLECT_H

#include "token.h"
#include "wordhash.h"

extern void	wordprop_init(void *vwordprop);
extern void	wordcnts_init(void *vwordcnts);
extern void	wordcnts_incr(wordcnts_t *w1, wordcnts_t *w2);
extern void	collect_words(wordhash_t *wh);

#endif
