/* $Id: wordhash.h 6799 2009-02-14 22:10:54Z relson $ */

#ifndef WORDHASH_H
#define WORDHASH_H

#include "word.h"

/* Hash entry. */
typedef struct hashnode_t {
  /*@dependent@*/ struct hashnode_t *iter_next;	/* Next item added to hash. For fast traversal */
  struct hashnode_t *next;			/* Next item in linked list of items with same hash */ 
  word_t *key;					/* word key */
  void   *data;					/* Associated data. To be used by caller. */
} hashnode_t;

typedef struct wh_alloc_node {
  hashnode_t *buf;
  /*@refs@*/ size_t avail;
  /*@refs@*/ size_t used;
  /*@null@*/ struct wh_alloc_node *next;
} wh_alloc_node;

typedef struct wh_alloc_str {
  char *buf;
  /*@refs@*/ size_t avail;
  /*@refs@*/ size_t used;
  /*@null@*/ struct wh_alloc_str *next;
} wh_alloc_str;

typedef /*@null@*/ hashnode_t *hashnode_pt;

typedef enum wh_e { WH_NORMAL,
		    WH_PROPS,
		    WH_CNTS } wh_t;

typedef struct wordhash_s {
  /*@null@*/  /*@dependent@*/ wh_t type;		/* normal, ordered, props, or cnts */
  /*@null@*/  /*@dependent@*/ bool freeable;
  /*@null@*/  /*@dependent@*/ uint index;		/* access index */
  /*@null@*/  /*@dependent@*/ uint count;		/* count of words */
  /*@null@*/  /*@dependent@*/ uint size;		/* size of array */

  hashnode_pt *bin;
  /*@null@*/ /*@owned@*/ wh_alloc_node *nodes;		/* list of node buffers */
  /*@null@*/  		 wh_alloc_str  *strings;	/* list of string buffers */

  /*@null@*/  /*@dependent@*/ hashnode_t *iter_ptr;
  /*@null@*/  /*@dependent@*/ hashnode_t *iter_head;
  /*@null@*/  /*@dependent@*/ hashnode_t *iter_tail;

  /*@null@*/  /*@dependent@*/ hashnode_t *props;	/* array of nodes */
  /*@null@*/  /*@dependent@*/ wordcnts_t *cnts;		/* array of counts */
} wordhash_t;

/*@only@*/ wordhash_t *wordhash_new(void);
/*@only@*/ wordhash_t *wordhash_init(wh_t type, uint count);

void wordhash_free(/*@only@*/ wordhash_t *);
size_t wordhash_count(wordhash_t * h);
void wordhash_sort(wordhash_t * h);
void wordhash_add(wordhash_t *dst, wordhash_t *src, void (*initializer)(void *));
void wordhash_set_counts(wordhash_t *wh, int good, int bad);

void *wordhash_search (const wordhash_t *wh, const word_t *t, uint hash);

/* Given h, s, n, search for key s.
 * If found, return pointer to associated buffer.
 * Else, insert key and return pointer to allocated buffer of size n. */
/*@observer@*/ void *wordhash_insert(wordhash_t *, word_t *, size_t, void (*)(void *));

/* Starts an iteration over the hash entries */
/*@null@*/ /*@exposed@*/ void *wordhash_first(wordhash_t *);

/* returns next entry or NULL if at end */
/*@null@*/ /*@exposed@*/ void *wordhash_next(wordhash_t *);

typedef void wh_foreach_t(word_t *token, void *data, void *userdata);
void wordhash_foreach(wordhash_t *wh, wh_foreach_t *hook, void *userdata);
wordhash_t *convert_propslist_to_countlist(wordhash_t *wh);
wordhash_t *convert_wordhash_to_propslist(wordhash_t *wh, wordhash_t *db);

/* for bogotune */

void *wordhash_search_memory (const word_t *t);

#endif
