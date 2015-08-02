/* $Id: datastore_qdbm.c 6649 2007-02-14 20:30:15Z m-a $ */

/*****************************************************************************

NAME:
datastore_qdbm.c -- implements the datastore, using qdbm.

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>          2003
Matthias Andree <matthias.andree@gmx.de> 2003
Stefan Bellon <sbellon@sbellon.de>       2003-2004

******************************************************************************/

#include "common.h"

#include <depot.h>
#include <cabin.h>
#include <villa.h>
#include <stdlib.h>

#include "datastore.h"
#include "datastore_db.h"
#include "datastore_qdbm.h"
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
    VILLA *dbp;
} dbh_t;

/* Function definitions */

const char *db_version_str(void)
{
    static char v[80];
    if (!v[0])
	snprintf(v, sizeof(v), "QDBM (Depot version %s, Villa API)", dpversion);
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

    int open_flags;
    VILLA *dbp;

    UNUSED(dummy);

    if (open_mode & DS_WRITE)
	open_flags = VL_OWRITER;
    else
	open_flags = VL_OREADER;

    handle = dbh_init(bfp);

    if (handle == NULL) return NULL;

    dbp = handle->dbp = vlopen(handle->name, open_flags, cmpkey);

    if ((dbp == NULL) && (open_mode & DS_WRITE)) {
	dbp = handle->dbp = vlopen(handle->name, open_flags|VL_OCREAT, cmpkey);
	if (dbp != NULL)
	    handle->created = true;
    }

    if (dbp == NULL)
	goto open_err;

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "(qdbm) vlopen( %s, %d )\n", handle->name, open_mode);

    return handle;

 open_err:
    print_error(__FILE__, __LINE__, "(qdbm) vlopen(%s, %d), err: %d, %s",
		handle->name, open_flags, 
		dpecode, dperrmsg(dpecode));
    dbh_free(handle);

    return NULL;
}


int db_delete(void *vhandle, const dbv_t *token)
{
    int ret;
    dbh_t *handle = vhandle;
    VILLA *dbp;

    dbp = handle->dbp;
    ret = vlout(dbp, token->data, token->leng);

    if (ret == 0) {
	print_error(__FILE__, __LINE__, "(qdbm) vlout('%.*s'), err: %d, %s",
		    CLAMP_INT_MAX(token->leng),
		    (char *)token->data, 
		    dpecode, dperrmsg(dpecode));
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
    VILLA *dbp = handle->dbp;

    data = vlget(dbp, token->data, token->leng, &dsiz);

    if (data == NULL)
	return DS_NOTFOUND;

    if (val->leng < (unsigned)dsiz) {
	print_error(__FILE__, __LINE__,
		    "(qdbm) db_get_dbvalue( '%.*s' ), size error %lu: %lu",
		    CLAMP_INT_MAX(token->leng),
		    (char *)token->data, (unsigned long)val->leng,
		    (unsigned long)dsiz);
	exit(EX_ERROR);
    }

    val->leng = dsiz;		/* read count */
    memcpy(val->data, data, dsiz);

    free(data); /* not xfree() as allocated by vlget() */

    return 0;
}


/*
   Re-organize database according to some heuristics
*/
static inline void db_optimize(VILLA *dbp, char *name)
{
    UNUSED(dbp);
    UNUSED(name);

    /* The Villa API doesn't need optimizing like the formerly used
       Depot API because Villa uses B+ trees and Depot uses hash tables.
       Database size may grow larger and could get compacted with
       vloptimize() however as the database size with Villa is smaller
       anyway, I don't think it is worth it. -- David Relson */
}


int db_set_dbvalue(void *vhandle, const dbv_t *token, const dbv_t *val)
{
    int ret;
    dbh_t *handle = vhandle;
    VILLA *dbp = handle->dbp;

    ret = vlput(dbp, token->data, token->leng, val->data, val->leng, VL_DOVER);

    if (ret == 0) {
	print_error(__FILE__, __LINE__,
		    "(qdbm) db_set_dbvalue( '%.*s' ) err: %d, %s",
		    CLAMP_INT_MAX(token->leng), (char *)token->data,
		    dpecode, dperrmsg(dpecode));
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
    VILLA *dbp;

    if (handle == NULL) return;

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "(qdbm) vlclose(%s)\n", handle->name);

    dbp = handle->dbp;

    db_optimize(dbp, handle->name);

    if (!vlclose(dbp))
	print_error(__FILE__, __LINE__, "(qdbm) vlclose for %s err: %d, %s",
		    handle->name, 
		    dpecode, dperrmsg(dpecode));

    handle->dbp = NULL;

    dbh_free(handle);
}


/*
   Flush any data in memory to disk
*/
void db_flush(void *vhandle)
{
    dbh_t *handle = vhandle;
    VILLA * dbp = handle->dbp;

    if (!vlsync(dbp))
	print_error(__FILE__, __LINE__, "(qdbm) vlsync err: %d, %s",
		    dpecode, dperrmsg(dpecode));
}


ex_t db_foreach(void *vhandle, db_foreach_t hook, void *userdata)
{
    int ret = 0;

    dbh_t *handle = vhandle;
    VILLA *dbp = handle->dbp;

    dbv_t dbv_key, dbv_data;
    int ksiz, dsiz;
    char *key, *data;

    ret = vlcurfirst(dbp);
    if (ret) {
	while ((key = vlcurkey(dbp, &ksiz))) {
	    data = vlcurval(dbp, &dsiz);
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

	    vlcurnext(dbp);
	}
    } else {
	print_error(__FILE__, __LINE__, "(qdbm) vlcurfirst err: %d, %s",
		    dpecode, dperrmsg(dpecode));
	exit(EX_ERROR);
    }

    return EX_OK;
}

const char *db_str_err(int e)
{
    return dperrmsg(e);
}
