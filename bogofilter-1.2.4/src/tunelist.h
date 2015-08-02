/* $Id: tunelist.h 6714 2008-04-20 15:27:06Z relson $ */

/*****************************************************************************

NAME:
   tunelist.h -- definitions and prototypes of list structures for bogotune.
		 includes msglist_t, filelist_t, and tunelist_t.

******************************************************************************/

#ifndef TUNELIST_H
#define TUNELIST_H

#include "wordhash.h"

/***** msglist *****/

typedef struct mlitem_s mlitem_t;
typedef struct msglist_s mlhead_t;
typedef struct wordprops_s wordprops_t;

struct msglist_s {		/* message list header */
    char *name;
    uint     count;
    mlitem_t *head;
    mlitem_t *tail;
};

struct mlitem_s {		/* message list item */
    mlitem_t *next;
    wordhash_t *wh;
    wordprops_t *wp;
};

struct wordprops_s {
    uint	count;
    wordprop_t *wp;
};

extern	mlhead_t *msglist_new(const char *label);
extern	void msglist_add(mlhead_t *list, wordhash_t *wh);
extern	void msglist_print(mlhead_t *list);
extern	void msglist_free(mlhead_t *list);

/***** filelist *****/

typedef struct flitem_s flitem_t;
typedef struct flhead_s flhead_t;

struct flhead_s {		/* file list header */
    char *name;
    uint      count;
    flitem_t *head;
    flitem_t *tail;
};

struct flitem_s {		/* file list item */
    flitem_t *next;
    char *name;
};

extern	flhead_t *filelist_new(const char *name);
extern	void filelist_add(flhead_t *list, const char *name);
extern	void filelist_free(flhead_t *list);

/***** tunelist *****/

typedef struct tunelist_s tunelist_t;

struct tunelist_s {
    const char *name;
    uint	count;
    wordhash_t *train;	/* training */
    mlhead_t   *msgs;
    union {
	mlhead_t *sets[3];
	struct runs {
	    mlhead_t *r0;
	    mlhead_t *r1;
	    mlhead_t *r2;
	} r;
    } u;
};

uint count_messages(tunelist_t *list);
tunelist_t *tunelist_new(const char *label);
void tunelist_print(tunelist_t *list);
void tunelist_free(tunelist_t *list);
#endif
