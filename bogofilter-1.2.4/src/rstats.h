/* $Id: rstats.h 6777 2009-02-01 03:49:48Z relson $ */

/*****************************************************************************

NAME:
   rstats.h -- support for debug routines for printing robinson data.

AUTHOR:
   David Relson <relson@osagesoftware.com>

******************************************************************************/

#ifndef RSTATS_H
#define RSTATS_H

void rstats_init(void);
void rstats_cleanup(void);

void rstats_add(const word_t *token,
		double prob,
		bool used,
		wordcnts_t *cnts);

void rstats_fini(size_t robn, 
		 FLOAT P, 
		 FLOAT Q, 
		 double spamicity);

void rstats_print(bool unsure);

void rstats_cnt_rn_ns_sp(uint *cnt, uint *rn, uint *ns, uint *sp);

#endif	/* RSTATS_H */
