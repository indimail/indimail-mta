/* $Id: datastore_qdbm.c,v 1.49 2005/04/04 11:16:21 relson Exp $ */

/*****************************************************************************

NAME:
datastore_tc.c -- implements the datastore, using tokyocabinet.

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>          2003
Matthias Andree <matthias.andree@gmx.de> 2003
Stefan Bellon <sbellon@sbellon.de>       2003-2004
Pierre Habouzit <madcoder@debian.org>    2007

******************************************************************************/

#include "common.h"

#include <tcutil.h>
#include <tchdb.h>
#include <tcbdb.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "datastore.h"
#include "datastore_db.h"
#include "error.h"
#include "paths.h"
#include "xmalloc.h"
#include "xstrdup.h"

#define UNUSED(x) ((void)&x)

typedef struct {
    char *path;
    char *name;
    bool locked;
    bool created;
    TCBDB *dbp;
} dbh_t;

/* transaction stuff */

static int tc_txn_begin(void *vhandle) {
    dbh_t *dbh = vhandle;
    if (!dbh->dbp->wmode || tcbdbtranbegin(dbh->dbp))
        return DST_OK;
    print_error(__FILE__, __LINE__, "tc_txn_begin(%p), err: %d, %s", dbh->dbp,
                tcbdbecode(dbh->dbp), tcbdberrmsg(tcbdbecode(dbh->dbp)));
    return DST_FAILURE;
}

static int tc_txn_abort(void *vhandle) {
    dbh_t *dbh = vhandle;
    if (!dbh->dbp->wmode || tcbdbtranabort(dbh->dbp))
        return DST_OK;
    print_error(__FILE__, __LINE__, "tc_txn_abort(%p), err: %d, %s", dbh->dbp,
                tcbdbecode(dbh->dbp), tcbdberrmsg(tcbdbecode(dbh->dbp)));
    return DST_FAILURE;
}

static int tc_txn_commit(void *vhandle) {
    dbh_t *dbh = vhandle;
    if (!dbh->dbp->wmode || tcbdbtrancommit(dbh->dbp))
        return DST_OK;
    print_error(__FILE__, __LINE__, "tc_txn_commit(%p), err: %d, %s",
                dbh->dbp, tcbdbecode(dbh->dbp),
                tcbdberrmsg(tcbdbecode(dbh->dbp)));
    return DST_FAILURE;
}

static dsm_t dsm_tc = {
    /* public -- used in datastore.c */
    &tc_txn_begin,
    &tc_txn_abort,
    &tc_txn_commit,

    /* private -- used in datastore_db_*.c */
    NULL,	/* dsm_env_init         */
    NULL,	/* dsm_cleanup          */
    NULL,	/* dsm_cleanup_lite     */
    NULL,	/* dsm_get_env_dbe      */
    NULL,	/* dsm_database_name    */
    NULL,	/* dsm_recover_open     */
    NULL,	/* dsm_auto_commit_flags*/
    NULL,	/* dsm_get_rmw_flag     */
    NULL,	/* dsm_lock             */
    NULL,	/* dsm_common_close     */
    NULL,	/* dsm_sync             */
    NULL,	/* dsm_log_flush        */
    NULL,	/* dsm_pagesize         */
    NULL,	/* dsm_purgelogs        */
    NULL,	/* dsm_checkpoint       */
    NULL,	/* dsm_recover          */
    NULL,	/* dsm_remove           */
    NULL,	/* dsm_verify           */
    NULL,	/* dsm_list_logfiles    */
    NULL	/* dsm_leafpages        */
};

dsm_t *dsm = &dsm_tc;


/* Function definitions */

const char *db_version_str(void)
{
    static char v[80];
    if (!v[0])
	snprintf(v, sizeof(v), "TokyoCabinet (version %s, B+tree API)", tcversion);
    return v;
}


static dbh_t *dbh_init(bfpath *bfp)
{
    dbh_t *handle;

    handle = xmalloc(sizeof(dbh_t));
    memset(handle, 0, sizeof(dbh_t));	/* valgrind */

    handle->name = xstrdup(bfp->filepath);

    handle->locked = false;
    handle->created = false;

    return handle;
}


static void dbh_free(/*@only@*/ dbh_t *handle)
{
    if (handle != NULL) {
      xfree(handle->name);
      xfree(handle->path);
      xfree(handle);
    }
    return;
}


/* Returns is_swapped flag */
bool db_is_swapped(void *vhandle)
{
    UNUSED(vhandle);

    return false;
}


/* Returns created flag */
bool db_created(void *vhandle)
{
    dbh_t *handle = vhandle;
    return handle->created;
}


/*
  Initialize database.
  Returns: pointer to database handle on success, NULL otherwise.
*/
void *db_open(void * dummy, bfpath *bfp, dbmode_t open_mode)
{
    dbh_t *handle;

    bool res;
    int open_flags;
    TCBDB *dbp;

    UNUSED(dummy);

    if (open_mode & DS_WRITE)
	open_flags = BDBOWRITER;
    else
	open_flags = BDBOREADER;

    handle = dbh_init(bfp);

    if (handle == NULL) return NULL;

    dbp = handle->dbp = tcbdbnew();
    res = tcbdbopen(dbp, handle->name, open_flags);
    if (!res && (open_mode & DS_WRITE)) {
        res = tcbdbopen(dbp, handle->name, open_flags | BDBOCREAT);
        handle->created |= res;
    }

    if (!res)
	goto open_err;

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "(tc) tcbdbopen( %s, %d )\n", handle->name, open_mode);

    return handle;

 open_err:
    print_error(__FILE__, __LINE__, "(tc) tcbdbopen(%s, %d), err: %d, %s",
		handle->name, open_flags, 
		tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));
    dbh_free(handle);

    return NULL;
}


int db_delete(void *vhandle, const dbv_t *token)
{
    int ret;
    dbh_t *handle = vhandle;
    TCBDB *dbp;

    dbp = handle->dbp;
    ret = tcbdbout(dbp, token->data, token->leng);

    if (ret == 0) {
	print_error(__FILE__, __LINE__, "(tc) tcbdbout('%.*s'), err: %d, %s",
		    CLAMP_INT_MAX(token->leng),
		    (char *)token->data, 
		    tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));
	exit(EX_ERROR);
    }
    ret = ret ^ 1;	/* ok is 1 in qdbm and 0 in bogofilter */

    return ret;		/* 0 if ok */
}


int db_get_dbvalue(void *vhandle, const dbv_t *token, /*@out@*/ dbv_t *val)
{
    char *data;
    int dsiz;

    dbh_t *handle = vhandle;
    TCBDB *dbp = handle->dbp;

    data = tcbdbget(dbp, token->data, token->leng, &dsiz);

    if (data == NULL)
	return DS_NOTFOUND;

    if (val->leng < (unsigned)dsiz) {
	print_error(__FILE__, __LINE__,
		    "(tc) db_get_dbvalue( '%.*s' ), size error %lu: %lu",
		    CLAMP_INT_MAX(token->leng),
		    (char *)token->data, (unsigned long)val->leng,
		    (unsigned long)dsiz);
	exit(EX_ERROR);
    }

    val->leng = dsiz;		/* read count */
    memcpy(val->data, data, dsiz);

    free(data); /* not xfree() as allocated by tcbdbget() */

    return 0;
}


/*
   Re-organize database according to some heuristics
*/
static inline void db_optimize(TCBDB *dbp, char *name)
{
    UNUSED(dbp);
    UNUSED(name);

    /* The Villa API doesn't need optimizing like the formerly used
       Depot API because Villa uses B+ trees and Depot uses hash tables.
       Database size may grow larger and could get compacted with
       tcbdboptimize() however as the database size with Villa is smaller
       anyway, I don't think it is worth it. */
}


int db_set_dbvalue(void *vhandle, const dbv_t *token, const dbv_t *val)
{
    int ret;
    dbh_t *handle = vhandle;
    TCBDB *dbp = handle->dbp;

    ret = tcbdbput(dbp, token->data, token->leng, val->data, val->leng);

    if (ret == 0) {
	print_error(__FILE__, __LINE__,
		    "(tc) db_set_dbvalue( '%.*s' ) err: %d, %s",
		    CLAMP_INT_MAX(token->leng), (char *)token->data,
		    tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));
	exit(EX_ERROR);
    }

    db_optimize(dbp, handle->name);

    return 0;
}


/*
   Close files and clean up.
*/
void db_close(void *vhandle)
{
    dbh_t *handle = vhandle;
    TCBDB *dbp;

    if (handle == NULL) return;

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "(tc) tcbdbclose(%s)\n", handle->name);

    dbp = handle->dbp;

    db_optimize(dbp, handle->name);

    if (!tcbdbclose(dbp))
	print_error(__FILE__, __LINE__, "(tc) tcbdbclose for %s err: %d, %s",
		    handle->name, 
		    tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));

    tcbdbdel(dbp);
    handle->dbp = NULL;

    dbh_free(handle);
}


/*
   Flush any data in memory to disk
*/
void db_flush(void *vhandle)
{
    dbh_t *handle = vhandle;
    TCBDB * dbp = handle->dbp;

    if (!tcbdbsync(dbp))
	print_error(__FILE__, __LINE__, "(tc) tcbdbsync err: %d, %s",
		    tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));
}


ex_t db_foreach(void *vhandle, db_foreach_t hook, void *userdata)
{
    int ret = 0;

    dbh_t *handle = vhandle;
    TCBDB *dbp = handle->dbp;
    BDBCUR *cursor;

    dbv_t dbv_key, dbv_data;
    int ksiz, dsiz;
    char *key, *data;

    cursor = tcbdbcurnew(dbp);
    ret = tcbdbcurfirst(cursor);
    if (ret) {
	while ((key = tcbdbcurkey(cursor, &ksiz))) {
	    data = tcbdbcurval(cursor, &dsiz);
	    if (data) {
		/* switch to "dbv_t *" variables */
		dbv_key.leng = ksiz;
		dbv_key.data = xmalloc(dbv_key.leng+1);
		memcpy(dbv_key.data, key, ksiz);
		((char *)dbv_key.data)[dbv_key.leng] = '\0';

		dbv_data.data = data;
		dbv_data.leng = dsiz;		/* read count */

		/* call user function */
		ret = hook(&dbv_key, &dbv_data, userdata);

		xfree(dbv_key.data);

		if (ret != 0)
		    break;
		free(data); /* not xfree() as allocated by dpget() */
	    }
	    free(key); /* not xfree() as allocated by dpiternext() */

	    tcbdbcurnext(cursor);
	}
    } else {
	print_error(__FILE__, __LINE__, "(tc) tcbdbcurfirst err: %d, %s",
		    tcbdbecode(dbp), tcbdberrmsg(tcbdbecode(dbp)));
	exit(EX_ERROR);
    }

    tcbdbcurdel(cursor);
    return EX_OK;
}

const char *db_str_err(int e)
{
    return tcbdberrmsg(e);
}
