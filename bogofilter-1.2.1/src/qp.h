/* $Id: qp.h 4803 2004-10-08 23:50:04Z m-a $ */

/*****************************************************************************

NAME:
   qp.h -- prototypes and definitions for qp.c

******************************************************************************/

#ifndef	HAVE_QP_H
#define	HAVE_QP_H

#include "word.h"

enum	qp_mode { RFC2045=2045, RFC2047=2047 };
typedef enum qp_mode qp_mode;
uint	qp_decode(word_t *word, qp_mode mode);
bool	qp_validate(const word_t *word, qp_mode mode);

#endif	/* HAVE_QP_H */
