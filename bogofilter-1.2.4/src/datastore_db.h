/* $Id: datastore_db.h 6895 2010-03-23 17:48:52Z m-a $ */

/*****************************************************************************

NAME:
datastore.h - API for bogofilter datastore.  

   The idea here is to make bogofilter independent of the database
   system used to store words.  The interface specified by this file
   determines the entire interaction between bogofilter and the
   database.  Writing a new database backend merely requires the
   implementation of the interface.

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>   2002 - 2003
Matthias Andree <matthias.andree@gmx.de> 2003

******************************************************************************/

#ifndef DATASTORE_DB_H
#define DATASTORE_DB_H

#include "datastore.h"

extern	bool	  db_log_autoremove;	/* DB_LOG_AUTOREMOVE  */
extern	bool	  db_txn_durable;	/* not DB_TXN_NOT_DURABLE */

/** Initialize database, open and lock files, etc.
 * params: char * path to database file, char * name of database
 * \return opaque pointer to database handle, which must be saved and
 * passed as the first parameter in all subsequent database function calls. 
 */
/*@only@*/ /*@null@*/
void *db_open(void *env,	/**< database environment to open DB in */
	      bfpath *bfp,	/**< path info for database file */
	      dbmode_t mode	/**< open mode, DS_READ or DS_WRITE */);

/** Close file and clean up. */
void  db_close(/*@only@*/ void *vhandle);

/** Flush pending writes to disk */
void db_flush(void *handle);

/** Do global initializations. \return pointer to environment for success, NULL for
 * error. */
void *dbe_init(bfpath *bfp);

/** Cleanup storage allocation */
void dbe_cleanup(void *);

/** Retrieve the value associated with a given word in a list.
 * \return zero if the word does not exist in the database.
 */
int db_get_dbvalue(
	void *vhandle,		/**< database handle */
	const dbv_t *token,	/**< key (token) to look for */
	/*@out@*/ dbv_t *val	/**  output, note: this must be
				 * pre-allocated and val->leng must
				 * specify how many bytes val->data can
				 * hold.
				 */
);

/** Delete the key */
int db_delete(void *handle, const dbv_t *data);

/** Set the value associated with a given word in a list. */
int db_set_dbvalue(void *handle, const dbv_t *token, const dbv_t *val);

/** Callback hook used by db_foreach, passes the original \p userdata
 * down as well as \a token and \a data. If the function returns a
 * nonzero value, the traversal is aborted. */
typedef ex_t (*db_foreach_t)(dbv_t *token, dbv_t *data, void *userdata);
/** Iterate over all elements in data base and call \p hook for each item.
 * \p userdata is passed through to the hook function unaltered. */
ex_t db_foreach(void *handle, db_foreach_t hook, void *userdata);

/** Returns error string associated with \a code. */
const char *db_str_err(int code);

/** Returns version string (pointer to a static buffer). */
const char *db_version_str(void);

/* help messages and option processing */
const char **dsm_help_bogofilter(void);
const char **dsm_help_bogoutil(void);

/** Begin new transaction. */
int db_txn_begin(void *vhandle);

/** Abort a pending transaction. */
int db_txn_abort(void *vhandle);

/** Commit a pending transaction. */
int db_txn_commit(void *vhandle);

/** Recover the environment given in \a directory. */
ex_t dbe_recover(bfpath *bfp, bool catastrophic, bool force);

/** Remove the environment from \a directory. */
ex_t dbe_remove(bfpath *bfp);

/** Return page size of \a databasefile, 0xffffffff for error, 0 for
 * unknown. */
u_int32_t db_pagesize(bfpath *bfp);

/** Return leaf page count of \a databasefile, 0xffffffff for error, 0 for
 * unknown. */
u_int32_t db_leafpages(bfpath *bfp);

/** Check if \a databasefile is a valid database. */
ex_t db_verify(bfpath *bfp);

/** Returns true if the database is byteswapped. */
bool db_is_swapped(void *vhandle);

/** Returns true if the database has been created in this session. */
bool db_created(void *vhandle);

/** Returns parent environment handle. */
void *db_get_env(void *vhandle);

/** Subroutine to place file lock */
int subr_db_lock(int fd, int cmd, short int type);

/** Lock database \a vhandle for \a open_mode */
int db_lock(void *vhandle, int open_mode);

/** Check if \a file is a regular file or missing */
bool is_file_or_missing(const char *file);

#endif
