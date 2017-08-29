/*
  POP3 UID database

	Copyright (c) 2010 MAD Partners, Ltd. (rweikusat@mssgmbh.com)

	This file is being published in accordance with the GPLv2 terms
	contained in the COPYING file being part of the fetchmail
	6.3.17 release, including the OpenSSL exemption.
*/
#ifndef fetchmail_uid_db_h
#define fetchmail_uid_db_h

/*  includes */
#include <stddef.h>

/*  types */
struct uid_db_record {
    char *id;
    size_t id_len;

    /*
      num	-	message number assigned by server
      status	-	message status (eg seen, deleted, ...)
      pos	-	position in record list
    */
    unsigned num, status, pos;

    struct uid_db_record *next;
};

struct num_ndx {
    /*
      Used to find uid records by message number.

      pos_0_value	-	highest message number
      end_value		-	lowest known message number

      Grows downwards because the fastuidl-code may record
      message numbers in non-ascending order but the
      lookup array should ideally only be large enough to
      store pointers to interesting ('new') messages.
    */
    struct uid_db_record **records;
    unsigned pos_0_value, end_value;
};

struct uid_db
{
    struct pat_node *pat_root;

    struct uid_db_record **records;
    unsigned records_max, records_next;

    struct num_ndx num_ndx;
};

typedef int uid_db_traversal_routine(struct uid_db_record *, void *);

/*  routines */
/**  initialization/ finalization */
void init_uid_db(struct uid_db *db);

void free_uid_db(struct uid_db *db);

static inline void clear_uid_db(struct uid_db *db)
{
    free_uid_db(db);
    init_uid_db(db);
}

/**  message number index handling */
static inline unsigned uid_db_num_ofs(struct num_ndx const *num_ndx, unsigned num)
{
    return num_ndx->pos_0_value - num;
}

void set_uid_db_num(struct uid_db *db, struct uid_db_record *rec,
		    unsigned num);

static inline void set_uid_db_num_pos_0(struct uid_db *db, unsigned num)
{
    db->num_ndx.pos_0_value = num;
    db->num_ndx.end_value = num + 1;
}

void reset_uid_db_nums(struct uid_db *db);

/**  various uidl db manipulatiors */
struct uid_db_record *uid_db_insert(struct uid_db *db,
				    char const *id, unsigned status);

void swap_uid_db_data(struct uid_db *db_0, struct uid_db *db_1);

/**  search routines */
struct uid_db_record *find_uid_by_id(struct uid_db *db, char const *id);

static inline struct uid_db_record *
find_uid_by_num(struct uid_db *db, unsigned num)
{
    struct num_ndx *num_ndx;

    num_ndx = &db->num_ndx;
    return num >= num_ndx->end_value ?
	num_ndx->records[uid_db_num_ofs(num_ndx, num)] : NULL;
}

static inline struct uid_db_record *
find_uid_by_pos(struct uid_db *db, unsigned pos)
{
    return pos < db->records_next ? db->records[pos] : NULL;
}

static inline struct uid_db_record *
first_uid_in_db(struct uid_db *db, char const *id)
{
    return find_uid_by_id(db, id);
}

struct uid_db_record *last_uid_in_db(struct uid_db *db, char const *id);

/**  various accessors */
static inline unsigned uid_db_n_records(struct uid_db const *db)
{
    return db->records_next;
}

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
int traverse_uid_db(struct uid_db *db,
		    uid_db_traversal_routine *r, void *arg);

#endif
