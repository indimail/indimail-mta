/* $Id: datastore.h 6895 2010-03-23 17:48:52Z m-a $ */

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

#ifndef DATASTORE_H
#define DATASTORE_H

#include "common.h"

#include <stdlib.h>

#ifdef	ENABLE_DB_DATASTORE	/* if Berkeley DB */
#include <db.h>
#endif

#include "paths.h"
#include "word.h"

extern YYYYMMDD today;		/* date as YYYYMMDD */

/** Name of the special token that counts the spam and ham messages
 * in the data base.
 */
#define MSG_COUNT ".MSG_COUNT"

/** Datastore handle type
** - used to communicate between datastore layer and database layer
** - known to program layer as a void*
*/
typedef struct {
    /** database handle from db_open() */
    void   *dbh;
    /** tracks endianness */
    bool is_swapped;
} dsh_t;

/** Datastore value type, used to communicate between program layer and
 * datastore layer.
 */
typedef struct {
    /** spam and ham counts */
    u_int32_t count[IX_SIZE];
    /** time stamp */
    u_int32_t date;
} dsv_t;

#define	spamcount count[IX_SPAM]
#define	goodcount count[IX_GOOD]

/** Status value used when a key is not found in the data base. */
#define DS_NOTFOUND (-1)
/** Status value when the transaction was aborted to resolve a deadlock
 * and should be retried. */
#define DS_ABORT_RETRY (-2)

/** Macro that clamps its argument to INT_MAX and casts it to int. */
#define CLAMP_INT_MAX(i) ((int)min(INT_MAX, (i)))

/** Database value type, used to communicate between datastore layer and
 * database layer.
 */
typedef struct {
    /** address of buffer    */
    void     *data;
    /** number of data bytes */
    u_int32_t leng;
} dbv_t;

#ifndef	ENABLE_DB_DATASTORE	/* if not Berkeley DB */
typedef	void DB;
typedef	void DB_ENV;
typedef	void DB_TXN;
#endif

/** type to keep track of database environments we have opened. */
typedef struct {
    int		magic;
    DB_ENV	*dbe;		/* stores the environment handle */
    char	*directory;	/* stores the home directory for this environment */
} dbe_t;

/* public -- used in datastore.c */
typedef int	dsm_i_pv	(void *vhandle);
/* private -- used in datastore_db_*.c */
typedef int	dsm_i_i		(int open_mode);
typedef int	dsm_i_pnvi	(DB_ENV *dbe, int ret);
typedef int	dsm_i_pvi	(void *handle, int open_mode);
typedef int	dsm_i_v		(void);
typedef void	dsm_v_pc	(const char *str);
typedef void	dsm_v_pbe	(dbe_t *env);
typedef void	dsm_v_pnv	(DB_ENV *dbe);
typedef void	dsm_v_pvuiui	(void *vhandle, u_int32_t numlocks, u_int32_t numobjs);
typedef const char *dsm_pc_pc	(const char *db_file);
typedef ex_t	dsm_x_ppbb	(bfpath *bfp, bool, bool);
typedef ex_t	dsm_x_pnvpp	(DB_ENV *dbe, bfpath *bfp);
typedef dbe_t  *dsm_pbe_pp	(bfpath *bfp);
typedef ex_t	dsm_x_pp	(bfpath *bfp);
typedef u_int32_t dsm_u_pp	(bfpath *bfp);
typedef DB_ENV *dsm_pnv_pp	(bfpath *bfp);
typedef DB_ENV *dsm_pnv_pbe	(dbe_t *env);
typedef ex_t	dsm_x_ppsi	(bfpath *bfp, int argc, char **argv);

/** Datastore methods type, used by datastore/database layers to switch
 * implementations after detection of database type. */
typedef struct {
    /* public -- used in datastore.c */
    dsm_i_pv	 *dsm_begin;
    dsm_i_pv	 *dsm_abort;
    dsm_i_pv	 *dsm_commit;
    /* private -- used in datastore_db_*.c */
    dsm_pbe_pp	 *dsm_env_init;
    dsm_v_pbe	 *dsm_cleanup;
    dsm_v_pbe	 *dsm_cleanup_lite;
    dsm_pnv_pbe	 *dsm_get_env_dbe;
    dsm_pc_pc	 *dsm_database_name;
    dsm_pnv_pp	 *dsm_recover_open; /**< exits on failure */
    dsm_i_v	 *dsm_auto_commit_flags;
    dsm_i_i	 *dsm_get_rmw_flag;
    dsm_i_pvi	 *dsm_lock;
    dsm_x_pnvpp	 *dsm_common_close;
    dsm_i_pnvi	 *dsm_sync;
    dsm_v_pnv	 *dsm_log_flush;
    dsm_u_pp	 *dsm_pagesize;
    dsm_x_pp	 *dsm_checkpoint;
    dsm_x_pp	 *dsm_purgelogs;
    dsm_x_ppbb	 *dsm_recover;
    dsm_x_pp	 *dsm_remove;
    dsm_x_pp	 *dsm_verify;
    dsm_x_ppsi	 *dsm_list_logfiles;
    dsm_u_pp	 *dsm_leafpages;
} dsm_t;

extern dsm_t *dsm;

/** Type of the callback function that ds_foreach calls. */
typedef int ds_foreach_t(
	/** current token that ds_foreach is looking at */
	word_t *token,
	/** data store value */
	dsv_t *data,
	/** unaltered value from ds_foreach call. */
	void *userdata);
/** Iterate over all records in data base and call \p hook for each item.
 * \p userdata is passed through to the hook function unaltered.
 */
extern ex_t ds_foreach(void *vhandle	  /** data store handle */,
		       ds_foreach_t *hook /** callback function */,
		       void *userdata	  /** opaque data that is passed to the callback function
					      unaltered */);

/** Wrapper for ds_foreach that opens and closes file */
extern ex_t ds_oper(void *dbenv,	/**< parent environment */
		    bfpath *bfp,	/**< path to database file */
		    dbmode_t open_mode,	/**< open mode, DS_READ or DS_WRITE */
		    ds_foreach_t *hook,	/**< function for actual operations */
		    void *userdata	/**< user data for \a hook */);

/** Initialize database, open and lock files, etc.
 * params: char * path to database file, char * name of database
 * \return opaque pointer to database handle, which must be saved and
 * passed as the first parameter in all subsequent database function calls. 
 */
/*@only@*/ /*@null@*/
extern void *ds_open(void *dbe,		/**< parent environment */
		     bfpath *bfp,	/**< path to database file */
		     dbmode_t open_mode	/**< open mode, DS_READ or DS_WRITE */);

/** Close file and clean up. */
extern void  ds_close(/*@only@*/ void *vhandle);

/** Flush pending writes to disk */
extern void ds_flush(void *vhandle);

/** Global initialization of datastore layer. Implies call to \a dsm_init. */
extern void *ds_init(bfpath *bfp);

/** Cleanup storage allocation of datastore layer. After calling this,
 * datastore access is no longer permitted. */
extern void ds_cleanup(void *);

/** Initialize datastore handle. */
dsh_t *dsh_init(void *dbh);		/* database handle from db_open() */

/** Free data store handle that must not be used after calling this
 * function. */
void dsh_free(void *vhandle);

/** Retrieve the value associated with a given word in a list. 
 * \return zero if the word does not exist in the database. Front-end
 */
extern int  ds_read  (void *vhandle, const word_t *word, /*@out@*/ dsv_t *val);

/** Retrieve the value associated with a given word in a list. 
 * \return zero if the word does not exist in the database. Implementation
 */
extern int ds_get_dbvalue(void *vhandle, const dbv_t *token, /*@out@*/ dbv_t *val);

/** Delete the key. */
extern int  ds_delete(void *vhandle, const word_t *word);

/** Set the value associated with a given word in a list. Front end. */
extern int  ds_write (void *vhandle, const word_t *word, dsv_t *val);

/** Set the value associated with a given word in a list. Implementation. */
extern int ds_set_dbvalue(void *vhandle, const dbv_t *token, dbv_t *val);

/** Update the value associated with a given word in a list. */
extern void ds_updvalues(void *vhandle, const dbv_t *token, const dbv_t *updval);

/** Get the database message count. */
extern int ds_get_msgcounts(void *vhandle, dsv_t *val);

/** Set the database message count. */
extern int ds_set_msgcounts(void *vhandle, dsv_t *val);

/** Get the parent environment. */
extern void *ds_get_dbenv(void *vhandle);

/* transactional code */
/** Start a transaction for the data store identified by vhandle.
 * All data base operations, including reading, must be "opened" by
 * ds_txn_begin and must be "closed" by either ds_txn_commit (to keep
 * changes) or ds_txn_abort (to discard changes made since the last
 * ds_txn_begin for the data base). Application or system crash will
 * lose any changes made since ds_txn_begin that have not been
 * acknowledged by successful ds_txn_commit().
 * \returns
 * - DST_OK for success. It is OK to proceed in data base access.
 * - DST_TEMPFAIL for problem. It is unknown whether this actually
 *   happens. You must not touch the data base.
 * - DST_FAILURE for problem. You must not touch the data base.
 */
extern int ds_txn_begin(void *vhandle);

/** Commit a transaction, keeping changes. As with any transactional
 * data base, concurrent updates to the same pages in the data base can
 * cause a deadlock of the writers. The datastore_xxx.c code will handle
 * the detection for you, in a way that it aborts as many transactions
 * until one can proceed. The aborted transactions will return
 * DST_TEMPFAIL and must be retried. No data base access must happen
 * after this call until the next ds_txn_begin().
 * \returns
 * - DST_OK to signify that the data has made it to the disk
 *   (which means nothing if the disk's write cache is enabled and the
 *   kernel has no means of synchronizing the cache - this is unknown for
 *   most kernels)
 * - DST_TEMPFAIL when a transaction has been aborted by the deadlock
 *   detector and must be retried
 * - DST_FAILURE when a permanent error has occurred that cannot be
 *   recovered from by the application (for instance, because corruption
 *   has occurred and needs to be recovered).
 */
extern int ds_txn_commit(void *vhandle);

/** Abort a transaction, discarding all changes since the previous
 * ds_txn_begin(). Changes are rolled back as though the transaction had
 * never been tried. No data base access must happen after this call
 * until the next ds_txn_begin().
 * \returns
 * - DST_OK for success.
 * - DST_TEMPFAIL for failure. It is uncertain if this actually happens.
 * - DST_FAILURE for failure. The application cannot continue.
 */
extern int ds_txn_abort(void *vhandle);

/** Successful return from ds_txn_* operation. */
#define DST_OK (0)
/** Temporary failure return from ds_txn_* operation, the application
 * should retry the failed data base transaction. */
#define DST_TEMPFAIL (1)
/** Permanent failure return from ds_txn_* operation, the application
 * should clean up and exit. */
#define DST_FAILURE (2)

/** Get the database version */
extern int ds_get_wordlist_version(void *vhandle, dsv_t *val);

/** set the database version */
extern int ds_set_wordlist_version(void *vhandle, dsv_t *val);

/** Get the database encoding */
extern int ds_get_wordlist_encoding(void *vhandle, dsv_t *val);

/** set the database encoding */
extern int ds_set_wordlist_encoding(void *vhandle, int enc);

/** Get the current process ID. */
extern unsigned long ds_handle_pid(void *vhandle);

/** Get the database filename. */
extern char *ds_handle_filename(void *vhandle);

/** Locks and unlocks file descriptor. */
extern int ds_lock(int fd, int cmd, short int type);

/** Returns version string. */
extern const char *ds_version_str(void);

/** Runs forced recovery on data base */
extern ex_t ds_recover(bfpath *bfp, bool catastrophic);

/** Remove environment in given directory, \return EX_OK or EX_ERROR */
extern ex_t ds_remove(bfpath *bfp);

/** Verify given database */
extern ex_t ds_verify(bfpath *bfp);

/** Return leaf page count of given database, \return 0xffffffff for error,
 * 0 if unknown. */
extern u_int32_t ds_leafpages(bfpath *bfp);

/** Return page size of given database, \return 0xffffffff for error, 0 for
 * variable sized pages or unpaged datastores. */
extern u_int32_t ds_pagesize(bfpath *bfp);

/** Remove inactive log files in given directory, \return EX_OK. */
extern ex_t ds_purgelogs(bfpath *bfp);

/** Run checkpoint once */
extern ex_t ds_checkpoint(bfpath *bfp);

/** datastore backends must provide this initializing function */
extern void dsm_init(bfpath *bfp);

/** list log files, remaining arguments are backend specific */
extern ex_t ds_list_logfiles(bfpath *bfp, int argc, char **argv);

/** parse bogofilter option
 * \return true if an option was recognized, false otherwise */
extern bool dsm_options_bogofilter(int option, const char *name, const char *val);

/** parse bogoutil option
 * \return true if an option was recognized, false otherwise */
extern bool dsm_options_bogoutil(int option, cmd_t *flag, int *count, const char **ds_file, const char *name, const char *val);

#endif
