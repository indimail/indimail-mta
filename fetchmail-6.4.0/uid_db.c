/*
  POP3 UID db

	Copyright (c) 2010 MAD Partners, Ltd. (rweikusat@mssgmbh.com)

	This file is being published in accordance with the GPLv2 terms
	contained in the COPYING file being part of the fetchmail
	6.3.17 release, including the OpenSSL exemption.
*/

/* Have Solaris expose ffs() from strings.h: */
#define __EXTENSIONS__
#define _XOPEN_SOURCE 700

/*  includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>  /* ffs() lives here - needs #define on Solaris. */

#include "xmalloc.h"
#include "uid_db.h"

/*  constants */
enum {
    MIN_RECORDS =	16	/* arbitrary */
};

/*  types */
struct pat_node {
    struct pat_node *ptrs_[3];

    /*
      The bit mask is stored in the nodes (as opposed to the
      offset, which is (re-)calculated on demand) because
      calculating the mask is a non-trivial operation (at
      least on x86).
     */
    unsigned bit_ndx, bit_mask;

    struct uid_db_record *rec;
};

/*
  The idea behind this is that the 'left' pointer of
  a node is accessible as ptrs(np)[-1] and the right
  one a ptrs(np)[1]. This implies that no separate codepaths
  for 'symmetric left- and right-cases' are needed.
*/
#define ptrs(np) ((np)->ptrs_ + 1)

/*  routines */
/**  various helpers */
static inline unsigned bit_ofs(unsigned bit_ndx)
{
    return bit_ndx >> 3;
}

static inline unsigned bit_mask(unsigned bit_ndx)
{
    return 1 << (bit_ndx & 7);
}

/**  PATRICIA trie insertion */
/***  walkers */
static struct pat_node *walk_down(struct uid_db *db, struct uid_db_record *rec,
				  struct pat_node ***edgep, struct pat_node **parentp)
{
    /*
      Find the pat node whose id is 'most similar' to the id
      stored in rec->id. Return a pointer to this node.
      'parentp' and 'edgep' are output-only parameters which
      will point to the parent of returned node and to the edge
      pointer going from the parent to the returned node after
      the call has returned.

      This routine is intended for inserts only.
     */
    struct pat_node *cur, **edge;
    unsigned bit_ndx, v = 0, ofs;

    cur = db->pat_root;
    ofs = -1;
    do {
	bit_ndx = cur->bit_ndx;

	if (bit_ofs(bit_ndx) != ofs) {
	    ofs = bit_ofs(bit_ndx);
	    v = ofs < rec->id_len ? rec->id[ofs] : 0;
	}

	edge = ptrs(cur) + (v & cur->bit_mask ? 1 : -1);
    } while ((cur = *edge) && cur->bit_ndx > bit_ndx);

    *parentp =
	(struct pat_node *)
	((unsigned char *)edge - (v & bit_mask(bit_ndx) ?
				  offsetof(struct pat_node, ptrs_) + 2 * sizeof(struct pat_node *)
				  : offsetof(struct pat_node, ptrs_)));
    *edgep = edge;
    return cur;
}

static inline struct pat_node *walk_up(unsigned diff_ndx, struct pat_node **parent)
{
    /*
      Walk the chain of parent pointers starting with *parent until a node
      is found whose parent has a bit_ndx smaller than diff_ndx. Return
      a pointer to this node and update *parent to point to its parent.
    */
    struct pat_node *p, *np;

    np = *parent;

    while ((p = *ptrs(np)) && p->bit_ndx > diff_ndx)
	np = p;

    *parent = p;
    return np;
}

/***  bit fiddling */
static inline unsigned first_set_bit_in_char(unsigned v)
{
    return ffs(v) - 1;
}

static int find_first_diff_bit(struct uid_db_record const *r0,
			       struct uid_db_record const *r1)
{
    /*
      Return the bit_ndx of the first differing bit in
      r0->id and r1->id or -1 if the strings are identical.
    */
    struct uid_db_record const *long_id;
    unsigned ofs, max;
    unsigned char v;

    max = r0->id_len;
    if (max > r1->id_len) {
	max = r1->id_len;
	long_id = r0;
    } else
	long_id = r1;

    ofs = 0;
    do
	v = r0->id[ofs] ^ r1->id[ofs];
    while (!v && ++ofs < max);

    if (!v) {
	if (r0->id_len == r1->id_len) return -1;
	v = long_id->id[ofs];
    }

    return first_set_bit_in_char(v) + ofs * 8;
}

static inline unsigned bit_set(unsigned bit_ndx, struct uid_db_record const *rec)
{
    /*
      Return non-zero if the bit corresponding with bit_ndx is set
      in rec->id
    */
    unsigned ofs;

    ofs = bit_ofs(bit_ndx);
    if (ofs >= rec->id_len) return 0;
    return rec->id[ofs] & bit_mask(bit_ndx);
}

/***  node allocation */
static struct pat_node *get_pat_node(struct uid_db_record *rec)
{
    /*
      Allocate a pat_node, set its rec pointer to rec and the
      next pointer of rec to NULL. Return pointer to the pat_node.
    */
    struct pat_node *np;

    np = (struct pat_node *)xmalloc(sizeof(*np));
    np->rec = rec;
    rec->next = NULL;
    return np;
}

static struct pat_node *get_standalone_node(struct uid_db_record *rec)
{
    /*
      Return a pat_node suitable for being inserted on the 'left edge'
      of the trie, ie either linked to a node whose left pointer was zero
      or being inserted as root node into an empty trie. The bit_ndx of
      the pat_node is set to the index corresponding with the highest
      set bit in rec->id.

      NB: This is a bad choice when UIDs share a common prefix because
      this implies that the root node will cause a bit to be tested which
      is non-zero in all other nodes, adding a theoretically redundant
      level to the trie. This is (to the best of my knowledge) un-
      fortunately unavoidable if nodes with different key lengths need
      to be supported.
    */
    struct pat_node *np;

    np = get_pat_node(rec);
    np->bit_ndx = first_set_bit_in_char(*rec->id);
    np->bit_mask = bit_mask(np->bit_ndx);
    return np;
}

/***  various helpers */
#if 0
static inline int record_id_equal(struct uid_db_record const *r0,
				  struct uid_db_record const *r1)
{
    return
	r0->id_len == r1->id_len
	&& memcmp(r0->id, r1->id, r0->id_len) == 0;
}
#endif

static struct uid_db_record *append_to_list(struct uid_db_record **recp,
					    struct uid_db_record *rec)
{
    /*
      Append the uid_db_record pointed to by rec to the uid_db_record
      list accessible as *recp and return rec.
    */
    while (*recp) recp = &(*recp)->next;
    *recp = rec;

    rec->next = NULL;
    return rec;
}

/***  insert routine */
static struct uid_db_record *pat_insert(struct uid_db *db,
					struct uid_db_record *rec)
{
    /*
      Insert the record pointed to by rec in the (potentially empty)
      PATRICIA trie pointed to by db->pat_root and return rec.
    */
    struct pat_node *np, *closest, *parent, **edge;
    int me, bit_ndx;

    if (!db->pat_root) {
	np = get_standalone_node(rec);
	ptrs(np)[-1] = *ptrs(np) = NULL;
	ptrs(np)[1] = np;

	db->pat_root = np;
	return rec;
    }

    closest = walk_down(db, rec, &edge, &parent);

    if (closest) {
	bit_ndx = find_first_diff_bit(closest->rec, rec);
	if (bit_ndx < 0)
	    return append_to_list(&closest->rec->next, rec);

	np = get_pat_node(rec);
	np->bit_ndx = bit_ndx;
	np->bit_mask = bit_mask(bit_ndx);
    } else
	np = get_standalone_node(rec);

    if (parent->bit_ndx > np->bit_ndx) {
	closest = walk_up(np->bit_ndx, &parent);

	if (!parent) edge = &db->pat_root;
	else edge = ptrs(parent)[-1] == closest ?
		 ptrs(parent) - 1 : ptrs(parent) + 1;
	*ptrs(closest) = np;
    }

    *edge = np;
    *ptrs(np) = parent;

    me = bit_set(np->bit_ndx, rec) ? 1 : -1;
    ptrs(np)[me] = np;
    ptrs(np)[-me] = closest;

    return rec;
}

/**  general db insertion */
static struct uid_db_record *get_uid_db_record(char const *id, unsigned status)
{
    /*
      Allocate a uid_db_record structure and set its id pointer to a
      dynamically allocated copy of id. The status member of the
      new record is set to status and its message number to zero (invalid).
      A pointer to it is then returned.
     */
    struct uid_db_record *rec;
    size_t id_len;

    rec = (struct uid_db_record *)xmalloc(sizeof(*rec));

    id_len = strlen(id);
    rec->id = (char *)memcpy(xmalloc(id_len + 1), id, id_len + 1);
    rec->id_len = id_len;
    rec->status = status;
    rec->num = 0;

    return rec;
}

static void insert_into_records(struct uid_db *db,
				struct uid_db_record *rec)
{
    /*
      Insert rec into the records array of the uid_db pointed
      to by db. The array is grown as necessary and the
      corresponding state variables of the db are updated
      accordingly. The pos member of rec is set to its position
      in the array.
    */
    unsigned next, want;

    next = db->records_next;

    if (next == db->records_max) {
	want = db->records_max *= 2;
	db->records = (struct uid_db_record **)xrealloc(db->records, want * sizeof(rec));
    }

    rec->pos = next;
    db->records[next] = rec;
    db->records_next = next + 1;
}

struct uid_db_record *uid_db_insert(struct uid_db *db,
				    char const *id, unsigned status)
{
    /*
      Create an uid_db_record whose id is id and whose status is
      status and insert it into the uid_db pointed to by db.
      Return a pointer to the newly created record.
    */
    struct uid_db_record *rec;

    rec = get_uid_db_record(id, status);
    insert_into_records(db, rec);
    return pat_insert(db, rec);
}

/**  message number index */
void set_uid_db_num(struct uid_db *db, struct uid_db_record *rec,
		    unsigned num)
{
    /*
      Set the message number of the record pointed to by rec to num
      and insert it into the num_ndx of the uid_db pointed to by db
      at position corresponding with num. The num_ndx lookup array
      is grown as needed. Message numbers are expected to 'generally'
      be recorded in ascending order and hence, no provisions are
      made to deal with the potentially quadratic complexity of
      inserting a sequence of numbers into an array such that it
      needs to be grown continuously.
    */
    struct num_ndx *num_ndx;
    unsigned have, want;

    num_ndx = &db->num_ndx;

    if (num_ndx->end_value > num) {
	have = num_ndx->pos_0_value - num_ndx->end_value + 1;
	want = num_ndx->pos_0_value - num + 1;
	num_ndx->end_value = num;

	num_ndx->records = (struct uid_db_record **)xrealloc(num_ndx->records, want * sizeof(rec));
	do num_ndx->records[--want] = NULL; while (want > have);
    }

    num_ndx->records[uid_db_num_ofs(num_ndx, num)] = rec;
}

void reset_uid_db_nums(struct uid_db *db)
{
    /*
      Reset the message numbers of all uid_db_records stored
      in the uid_db pointed to by db. The corresponding num_ndx
      lookup array is afterwards freed and the num_ndx end_value
      adjusted in order to indicate an 'empty' message number
      index.
    */
    struct uid_db_record **rec;
    struct num_ndx *num_ndx;
    unsigned ndx;

    num_ndx = &db->num_ndx;

    if (num_ndx->end_value < num_ndx->pos_0_value) {
	ndx = num_ndx->pos_0_value - num_ndx->end_value;
	while (ndx) {
	    rec = num_ndx->records + --ndx;
	    if (*rec) (*rec)->num = 0;
	}

	num_ndx->end_value = num_ndx->pos_0_value + 1;

	free(num_ndx->records);
	num_ndx->records = NULL;
    }
}

/**  search routines */
struct uid_db_record *find_uid_by_id(struct uid_db *db, char const *id)
{
    /*
      Search for an uid_db_record whose id is id in the uid_db pointed
      to by db and return a pointer to it or NULL if no such record was
      found.
    */
    struct pat_node *np;
    struct uid_db_record *rec;
    unsigned v = 0, bit_ndx, ofs;
    size_t len;

    np = db->pat_root;
    if (np) {
	len = strlen(id);
	ofs = -1;
	do {
	    bit_ndx = np->bit_ndx;

	    if (bit_ofs(bit_ndx) != ofs) {
		ofs = bit_ofs(bit_ndx);
		v = ofs < len ? id[ofs] : 0;
	    }

	    np = ptrs(np)[v & np->bit_mask ? 1 : -1];
	} while (np && np->bit_ndx > bit_ndx);

	if (!np) return NULL;

	rec = np->rec;
	return rec->id_len == len && memcmp(id, rec->id, len) == 0 ?
	    rec : NULL;
    }

    return NULL;
}

struct uid_db_record *last_uid_in_db(struct uid_db *db, char const *id)
{
    /*
      Return a pointer to the 'last' (insert order) uid_db_record
      contained in the uid_db pointed to by db whose id is id or
      NULL if no such record exists.
    */
    struct uid_db_record *rec;

    rec = find_uid_by_id(db, id);
    if (!rec) return NULL;

    while (rec->next) rec = rec->next;
    return rec;
}

/**  destruction */
static void free_uid_list(struct uid_db_record *rec)
{
    if (!rec) return;

    /*
      Free the list of uid_db_records starting with
      the record pointed to by rec.
    */
    if (rec->next) free_uid_list(rec->next);

    xfree(rec->id);
    xfree(rec);
}

static void free_pat_trie(struct pat_node *np)
{
    /*
      Free the PATRCIA-trie pointed to by np and all
      uid_db_records contained in it.

      The algorithm implemented below is:

	1. Load the left pointer of the node pointed to by
	   np into next.

	2. If the result is not NULL,
		2a) Set the left pointer to NULL.
		2b) Goto 1 if next points to a child of np.

	3. Load the right pointer of the node pointed to by
	   np into next.

	4. If the result is not NULL,
		4a) Set the right pointer to NULL.
		4b) Goto 1 id next points to a child of np.

	5. Load next with the parent pointer of np.

	6. Free np->rec and np.

	7. Set np to next and goto 1 if it is not null.
    */
    struct pat_node *next;

    do {
        next = ptrs(np)[-1];
        if (next) {
            ptrs(np)[-1] = NULL;
            if (next->bit_ndx > np->bit_ndx) continue;
        }

        next = ptrs(np)[1];
        if (next) {
            ptrs(np)[1] = NULL;
            if (next->bit_ndx > np->bit_ndx) continue;
        }

        next = *ptrs(np);

        free_uid_list(np->rec);
        free(np);
    } while ((np = next));
}

void free_uid_db(struct uid_db *db)
{
    /*
      Free all dynamically allocated memory of the uid_db
      pointed to by db. The structure is not reinitialized.
    */
    if (db->pat_root) free_pat_trie(db->pat_root);

    xfree(db->records);
    xfree(db->num_ndx.records);
}

/**  various public interfaces */
void init_uid_db(struct uid_db *db)
{
    /*
      Initialize the uid_db structure pointed to by db 'properly'
      such that it represents an empty database. An array of
      size MIN_RECORDS is allocated and assigned to db->records.
    */
    struct num_ndx *num_ndx;

    db->pat_root = NULL;

    db->records = (struct uid_db_record **)xmalloc(MIN_RECORDS * sizeof(*db->records));
    db->records_max = MIN_RECORDS;
    db->records_next = 0;

    num_ndx = &db->num_ndx;
    num_ndx->pos_0_value = num_ndx->end_value = -1;
    num_ndx->records = NULL;
}

void swap_uid_db_data(struct uid_db *db_0, struct uid_db *db_1)
{
    struct uid_db tmp;

    tmp = *db_0;
    *db_0 = *db_1;
    *db_1 = tmp;
}

int traverse_uid_db(struct uid_db *db,
		     uid_db_traversal_routine *r, void *arg)
{
    /*
      Traverses the struct uid_db records array in insert order,
      invoking the subroutine pointed to by r with a pointer to
      each record and the arg pointer as arguments. If the return
      value of that is non-zero, traverse_uid_db immediately returns
      with this value. Otherwise, zero is returned after the last
      record was visited.

      The uid_db_traversal_routine must not modify the uid_db during
      traversal.
    */
    struct uid_db_record **recs;
    unsigned ndx, max;
    int rc;

    rc = 0;
    ndx = 0;
    max = db->records_next;
    recs = db->records;
    while (ndx < max && (rc = r(recs[ndx], arg)) == 0)
	++ndx;

    return rc;
}
