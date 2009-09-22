/* $Id: transaction.h 3200 2003-09-27 18:52:35Z relson $ */

/*****************************************************************************

NAME:
transaction.h -- prototypes and definitions for transaction.c

AUTHORS:
Stefan Bellon <sbellon@sbellon.de>       2003

******************************************************************************/

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "datastore.h"
#include "word.h"

#define	TA_OK	0
#define	TA_ERR	1

struct ta_type;
typedef struct ta_type ta_t;

ta_t *ta_init(void);
int ta_commit(ta_t *ta);
int ta_rollback(ta_t *ta);

int ta_delete(ta_t *ta, void *vhandle, const word_t *word);
int ta_write(ta_t *ta, void *vhandle, const word_t *word, dsv_t *val);
int ta_read(ta_t *ta, void *vhandle, const word_t *word, /*@out@*/ dsv_t *val);

#endif /* TRANSACTION_H */
