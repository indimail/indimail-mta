/* $Id: datastore_db_trad.c 6650 2007-02-14 21:19:32Z m-a $ */

/*****************************************************************************

NAME:
datastore_db_trad.c -- implements bogofilter's traditional
		       (non-transactional) datastore, 
		       using Berkeley DB

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>   2002 - 2003
Matthias Andree <matthias.andree@gmx.de> 2003 - 2004
David Relson	<relson@osagesoftware.com> 2003 - 2005

******************************************************************************/

#include "common.h"

#include <errno.h>

#include <db.h>

#include "datastore.h"
#include "datastore_db_private.h"
#include "datastore_db.h"
#include "datastore_dbcommon.h"

#include "db_lock.h"
#include "error.h"
#include "longoptions.h"
#include "paths.h"
#include "xmalloc.h"
#include "xstrdup.h"

/* public -- used in datastore.c */

/* private -- used in datastore_db_*.c */
static DB_ENV	  *bft_get_env_dbe	(dbe_t *env);
static const char *bft_database_name	(const char *db_file);
static DB_ENV	  *bft_recover_open	(bfpath *bfp);
static int	   bft_get_rmw_flag	(int open_mode);
static void	   bft_log_flush	(DB_ENV *dbe);
static dbe_t	  *bft_init		(bfpath *bfp);
static void 	   bft_cleanup		(dbe_t *env);
static void 	   bft_cleanup_lite	(dbe_t *env);

/* OO function lists */

dsm_t dsm_traditional = {
    /* public -- used in datastore.c */
    NULL,	/* bft_begin           */
    NULL,	/* bft_abort           */
    NULL,	/* bft_commit          */

    /* private -- used in datastore_db_*.c */
    &bft_init,
    &bft_cleanup,
    &bft_cleanup_lite,
    &bft_get_env_dbe,
    &bft_database_name,
    &bft_recover_open,
    NULL,		/* bft_auto_commit_flags*/
    &bft_get_rmw_flag,
    &db_lock,
    NULL,		/* &bft_common_close    */
    NULL,		/* &bft_sync            */
    &bft_log_flush,
    &db_pagesize,	/* dsm_pagesize         */
    NULL,		/* dsm_checkpoint       */
    NULL,		/* dsm_purgelogs        */
    NULL,		/* dsm_recover          */
    NULL,		/* dsm_remove           */
    &db_verify,		/* dsm_verify           */
    NULL,		/* dsm_list_logfiles    */
    &db_leafpages	/* dsm_leafpages        */
};

DB_ENV *bft_get_env_dbe	(dbe_t *env)
{
    (void) env;
    return NULL;
}

const char *bft_database_name(const char *db_file)
{
    return db_file;
}

int bft_get_rmw_flag(int open_mode)
{
    (void) open_mode;
    return 0;
}

DB_ENV *bft_recover_open(bfpath *bfp)
{
    int fd;

    fd = open(bfp->filepath, O_RDWR);
    if (fd < 0) {
	print_error(__FILE__, __LINE__, "bft_recover_open: cannot open %s: %s", bfp->filepath,
		    strerror(errno));
	exit(EX_ERROR);
    }

    if (subr_db_lock(fd, F_SETLKW, (short int)F_WRLCK)) {
	print_error(__FILE__, __LINE__,
		    "bft_recover_open: cannot lock %s for exclusive use: %s", bfp->filepath,
		    strerror(errno));
	close(fd);
	exit(EX_ERROR);
    }

    return NULL;
}

void bft_log_flush(DB_ENV *dbe)
{
    int ret;

    ret = BF_LOG_FLUSH(dbe, NULL);

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "DB_ENV->log_flush(%p): %s\n", (void *)dbe,
		db_strerror(ret));
}

dbe_t *bft_init(bfpath *bfp)
{
    dbe_t *env = xcalloc(1, sizeof(dbe_t));

    env->magic = MAGIC_DBE;	    /* poor man's type checking */
    env->directory = xstrdup(bfp->dirname);

    return env;
}

void bft_cleanup(dbe_t *env)
{
    bft_cleanup_lite(env);
}

void bft_cleanup_lite(dbe_t *env)
{
    xfree(env->directory);
    xfree(env);
}
