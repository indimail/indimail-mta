/* $Id: datastore_db_trans.c 6484 2006-05-29 14:28:00Z relson $ */

/*****************************************************************************

NAME:
datastore_db_trad.c -- implements bogofilter's traditional
		       (non-transactional) datastore, 
		       using Berkeley DB

AUTHORS:
Gyepi Sam <gyepi@praxis-sw.com>   2002 - 2003
Matthias Andree <matthias.andree@gmx.de> 2003 - 2004
David Relson	<relson@osagesoftware.com> 2005

******************************************************************************/

#include "common.h"

#include <assert.h>
#include <error.h>
#include <errno.h>

#include <db.h>

#include "datastore.h"
#include "datastore_db_private.h"
#include "datastore_db.h"
#include "datastore_dbcommon.h"

#include "bool.h"
#include "db_lock.h"
#include "longoptions.h"
#include "mxcat.h"
#include "rand_sleep.h"
#include "xmalloc.h"
#include "xstrdup.h"

static int lockfd = -1;	/* fd of lock file to prevent concurrent recovery */

/** Default flags for DB_ENV->open() */
static const u_int32_t dbenv_defflags = DB_INIT_MPOOL
					| DB_INIT_LOG | DB_INIT_TXN;

/* public -- used in datastore.c */
static int	   dbx_begin		(void *vhandle);
static int	   dbx_abort		(void *vhandle);
static int	   dbx_commit		(void *vhandle);
/* private -- used in datastore_db_*.c */
static DB_ENV	  *dbx_get_env_dbe	(dbe_t *env);
static const char *dbx_database_name	(const char *db_file);
static DB_ENV	  *dbx_recover_open	(bfpath *bfp);
static int	   dbx_auto_commit_flags(void);
static int	   dbx_get_rmw_flag	(int open_mode);
static ex_t	   dbx_common_close	(DB_ENV *dbe, bfpath *bfp);
static int	   dbx_sync		(DB_ENV *dbe, int ret);
static void	   dbx_log_flush	(DB_ENV *dbe);
static dbe_t	  *dbx_init		(bfpath *bfp);
static void	   dbx_cleanup		(dbe_t *env);
static void	   dbx_cleanup_lite	(dbe_t *env);
static ex_t	   dbe_env_purgelogs	(DB_ENV *dbe);

static ex_t	   dbx_checkpoint	(bfpath *bfp);
static ex_t	   dbx_purgelogs	(bfpath *bfp);
static ex_t	   dbx_recover		(bfpath *bfp, bool catastrophic, bool force);
static ex_t	   dbx_remove		(bfpath *bfp);

static ex_t	   dbx_list_logfiles	(bfpath *bfp, int argc, char **argv);

/* OO function lists */

dsm_t dsm_transactional = {
    /* public -- used in datastore.c */
    &dbx_begin,
    &dbx_abort,
    &dbx_commit,

    /* private -- used in datastore_db_*.c */
    &dbx_init,
    &dbx_cleanup,
    &dbx_cleanup_lite,
    &dbx_get_env_dbe,
    &dbx_database_name,
    &dbx_recover_open,
    &dbx_auto_commit_flags,
    &dbx_get_rmw_flag,
    &db_lock,
    &dbx_common_close,
    &dbx_sync,
    &dbx_log_flush,
    &db_pagesize,
    &dbx_checkpoint,
    &dbx_purgelogs,
    &dbx_recover,
    &dbx_remove,
    &db_verify,
    &dbx_list_logfiles,
    &db_leafpages
};

/* non-OO static function prototypes */

static int plock(const char *path, short locktype, int mode);
static int db_try_glock(bfpath *bfp, short locktype, int lockcmd);
static int bf_dbenv_create(DB_ENV **dbe);
static void dbe_config(void *vhandle);
static dbe_t *dbe_xinit(dbe_t *env, bfpath *bfp, u_int32_t flags);
static DB_ENV *dbe_recover_open(bfpath *bfp, u_int32_t flags);

/* support functions */

static bool get_bool(const char *name, const char *arg)
{
    bool b = str_to_bool(arg);
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %s\n", name,
		b ? "Yes" : "No");
    return b;
}

static e_txn get_txn(const char *name, const char *arg)
{
    e_txn t = get_bool(name, arg) ? T_ENABLED : T_DISABLED;
    if (DEBUG_CONFIG(2))
	fprintf(dbgout, "%s -> %s\n", name,
		t ? "enabled" : "disabled");
    return t;
}


/* non-OO static functions */

DB_ENV *dbx_get_env_dbe(dbe_t *env)
{
    return env->dbe;
}

const char *dbx_database_name(const char *db_file)
{
    const char *t;

    t = strrchr(db_file, DIRSEP_C);
    if (t != NULL)
	t += 1;

    return t;
}

int  dbx_auto_commit_flags(void)
{
#if DB_AT_LEAST(4,1)
    return DB_AUTO_COMMIT;
#else
    return 0;
#endif
}

int dbx_get_rmw_flag(int open_mode)
{
    (void)open_mode;
    return 0;
}

/** print user-readable diagnostics and instructions after DB_ENV->open
 * failed. */
static void diag_dbeopen(
	/** DB_ENV->open() flags value  */ u_int32_t flags,
	/** env directory tried to open */ bfpath *bfp)
{
    if (flags & DB_RECOVER) {
	fprintf(stderr,
		"\n"
		"### Standard recovery failed. ###\n"
		"\n"
		"Please check section 3.3 in bogofilter's README.db file\n"
		"for help.\n");
	/* ask that the user runs catastrophic recovery */
    } else if (flags & DB_RECOVER_FATAL) {
	fprintf(stderr,
		"\n"
		"### Catastrophic recovery failed. ###\n"
		"\n"
		"Please check the README.db file that came with bogofilter for hints,\n"
		"section 3.3, or remove all __db.*, log.* and *.db files in \"%s\"\n"
		"and start from scratch.\n", bfp->dirname);
	/* catastrophic recovery failed */
    } else {
	fprintf(stderr, "To recover, run: bogoutil -v --db-recover \"%s\"\n",
		bfp->dirname);
    }
}

/** run recovery, open environment and keep the exclusive lock */
static DB_ENV *dbe_recover_open(bfpath *bfp, u_int32_t flags)
{
    const u_int32_t local_flags = flags | DB_CREATE;
    DB_ENV *dbe;
    int e;

    if (DEBUG_DATABASE(0))
        fprintf(dbgout, "trying to lock database directory\n");
    db_try_glock(bfp, F_WRLCK, F_SETLKW); /* wait for exclusive lock */

    /* run recovery */
    bf_dbenv_create(&dbe);

    if (DEBUG_DATABASE(0))
        fprintf(dbgout, "running regular data base recovery%s\n",
	       flags & DB_PRIVATE ? " and removing environment" : "");

    /* quirk: DB_RECOVER requires DB_CREATE and cannot work with DB_JOINENV */

    /*
     * Hint from Keith Bostic, SleepyCat support, 2004-11-29,
     * we can use the DB_PRIVATE flag, that rebuilds the database
     * environment in heap memory, so we don't need to remove it.
     */

    e = dbe->open(dbe, bfp->dirname,
		  dbenv_defflags | local_flags | DB_RECOVER, DS_MODE);
    if (e != 0) {
	print_error(__FILE__, __LINE__, "Cannot recover environment \"%s\": %s",
		bfp->dirname, db_strerror(e));
	if (e == DB_RUNRECOVERY)
	    diag_dbeopen(flags, bfp);
	exit(EX_ERROR);
    }

    return dbe;
}

static DB_ENV *dbx_recover_open(bfpath *bfp)
{
    return dbe_recover_open(bfp, 0);
}

static int dbx_begin(void *vhandle)
{
    DB_TXN *t;
    int ret;

    dbh_t *dbh = vhandle;
    dbe_t *env = dbh->dbenv;

    assert(dbh);
    assert(dbh->magic == MAGIC_DBH);
    assert(dbh->txn == 0);

    assert(env);
    assert(env->dbe);

    ret = BF_TXN_BEGIN(env->dbe, NULL, &t, 0);
    if (ret) {
	print_error(__FILE__, __LINE__, "DB_ENV->txn_begin(%p), err: %d, %s",
		(void *)env->dbe, ret, db_strerror(ret));
	return ret;
    }
    dbh->txn = t;

    if (DEBUG_DATABASE(2))
	fprintf(dbgout, "DB_ENV->dbx_begin(%p), tid: %lx\n",
		(void *)env->dbe, (unsigned long)BF_TXN_ID(t));

    return 0;
}

static int dbx_abort(void *vhandle)
{
    int ret;
    dbh_t *dbh = vhandle;
    DB_TXN *t;

    assert(dbh);
    assert(dbh->magic == MAGIC_DBH);

    t = dbh->txn;

    assert(t);

    ret = BF_TXN_ABORT(t);
    if (ret)
	print_error(__FILE__, __LINE__, "DB_TXN->abort(%lx) error: %s",
		(unsigned long)BF_TXN_ID(t), db_strerror(ret));
    else
	if (DEBUG_DATABASE(2))
	    fprintf(dbgout, "DB_TXN->abort(%lx)\n",
		    (unsigned long)BF_TXN_ID(t));

    dbh->txn = NULL;

    switch (ret) {
	case 0:
	    return DST_OK;
	case DB_LOCK_DEADLOCK:
	    return DST_TEMPFAIL;
	default:
	    return DST_FAILURE;
    }
}

static int dbx_commit(void *vhandle)
{
    int ret;
    dbh_t *dbh = vhandle;
    DB_TXN *t;
    u_int32_t id;

    assert(dbh);
    assert(dbh->magic == MAGIC_DBH);

    t = dbh->txn;

    assert(t);

    id = BF_TXN_ID(t);
    ret = BF_TXN_COMMIT(t, 0);
    if (ret)
	print_error(__FILE__, __LINE__, "DB_TXN->commit(%lx) error: %s",
		(unsigned long)id, db_strerror(ret));
    else
	if (DEBUG_DATABASE(2))
	    fprintf(dbgout, "DB_TXN->commit(%lx, 0)\n",
		    (unsigned long)id);

    dbh->txn = NULL;

    switch (ret) {
	case 0:
	    /* push out buffer pages so that >=15% are clean - we
	     * can ignore errors here, as the log has all the data */
	    BF_MEMP_TRICKLE(dbh->dbenv->dbe, 15, NULL);

	    return DST_OK;
	case DB_LOCK_DEADLOCK:
	    return DST_TEMPFAIL;
	default:
	    return DST_FAILURE;
    }
}

/** set an fcntl-style lock on \a path.
 * \a locktype is F_RDLCK, F_WRLCK, F_UNLCK
 * \a mode is F_SETLK or F_SETLKW
 * \return file descriptor of locked file if successful
 * negative value in case of error
 */
static int plock(const char *path, short locktype, int mode)
{
    struct flock fl;
    int fd, r;

    fd = open(path, O_RDWR);
    if (fd < 0) return fd;

    fl.l_type = locktype;
    fl.l_whence = SEEK_SET;
    fl.l_start = (off_t)0;
    fl.l_len = (off_t)0;
    r = fcntl(fd, mode, &fl);
    if (r < 0)
	return r;
    return fd;
}

static int db_try_glock(bfpath *bfp, short locktype, int lockcmd)
{
    int ret;
    char *t;

    /* lock */
    ret = bf_mkdir(bfp->dirname, DIR_MODE);
    if (ret && errno != EEXIST) {
	print_error(__FILE__, __LINE__, "mkdir(%s): %s",
		bfp->dirname, strerror(errno));
	exit(EX_ERROR);
    }

    t = mxcat(bfp->dirname, DIRSEP_S, "lockfile-d", NULL);

    /* All we are interested in is that this file exists, we'll close it
     * right away as plock down will open it again */
    ret = open(t, O_RDWR|O_CREAT|O_EXCL, DS_MODE);
    if (ret < 0 && errno != EEXIST) {
	print_error(__FILE__, __LINE__, "open(%s): %s",
		t, strerror(errno));
	exit(EX_ERROR);
    }

    if (ret >= 0)
	close(ret);

    lockfd = plock(t, locktype, lockcmd);
    if (lockfd < 0 && errno != EAGAIN && errno != EACCES) {
	print_error(__FILE__, __LINE__, "lock(%s): %s",
		t, strerror(errno));
	exit(EX_ERROR);
    }

    xfree(t);
    /* lock set up */

    return lockfd;
}

/** Create environment or exit with EX_ERROR */
static int bf_dbenv_create(DB_ENV **env)
{
    int ret = db_env_create(env, 0);
    if (ret != 0) {
	print_error(__FILE__, __LINE__, "db_env_create, err: %d, %s",
		ret, db_strerror(ret));
	exit(EX_ERROR);
    }
    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "db_env_create: %p\n", (void *)env);
    (*env)->set_errfile(*env, stderr);

    return ret;
}

static void dbe_config(void *vhandle)
{
    dbe_t *env = vhandle;
    int ret = 0;
    u_int32_t logsize = 1048576;    /* 1 MByte (default in BDB 10 MByte) */

    /* configure log file size */
    ret = env->dbe->set_lg_max(env->dbe, logsize);
    if (ret) {
	print_error(__FILE__, __LINE__, "DB_ENV->set_lg_max(%lu) err: %d, %s",
		(unsigned long)logsize, ret, db_strerror(ret));
	exit(EX_ERROR);
    }

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "DB_ENV->set_lg_max(%lu)\n", (unsigned long)logsize);
}

static dbe_t *dbx_init(bfpath *bfp)
{
    u_int32_t flags = 0;
    dbe_t *env = xcalloc(1, sizeof(dbe_t));

    env->magic = MAGIC_DBE;	    /* poor man's type checking */
    env->directory = xstrdup(bfp->dirname);

    /* open lock file, needed to detect previous crashes */
    if (init_dbl(bfp->dirname))
	exit(EX_ERROR);

    /* run recovery if needed */
    if (needs_recovery()) {
	dbx_recover(bfp, false, false); /* DO NOT set force flag here, may cause
						 multiple recovery! */

	/* reinitialize */
	if (init_dbl(bfp->dirname))
	    exit(EX_ERROR);
    }

    /* set (or demote to) shared/read lock for regular operation */
    db_try_glock(bfp, F_RDLCK, F_SETLKW);

    /* set our cell lock in the crash detector */
    if (set_lock()) {
	exit(EX_ERROR);
    }

    /* initialize */
#ifdef	FUTURE_DB_OPTIONS
#ifdef	DB_BF_TXN_NOT_DURABLE
    if (db_txn_durable)
	flags ^= DB_BF_TXN_NOT_DURABLE;
#endif
#endif

    dbe_xinit(env, bfp, flags);

    return env;
}

/* dummy infrastructure, to be expanded by environment
 * or transactional initialization/shutdown */
static dbe_t *dbe_xinit(dbe_t *env, bfpath *bfp, u_int32_t flags)
{
    int ret;

    env->magic = MAGIC_DBE;	    /* poor man's type checking */

    ret = bf_dbenv_create(&env->dbe);

    if (db_cachesize != 0 &&
	    (ret = env->dbe->set_cachesize(env->dbe, db_cachesize/1024, (db_cachesize % 1024) * 1024*1024, 1)) != 0) {
	print_error(__FILE__, __LINE__, "DB_ENV->set_cachesize(%u), err: %d, %s",
		db_cachesize, ret, db_strerror(ret));
	exit(EX_ERROR);
    }

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "DB_ENV->set_cachesize(%u)\n", db_cachesize);

    dbe_config(env);

    flags |= DB_CREATE | dbenv_defflags;

    ret = env->dbe->open(env->dbe, bfp->dirname, flags, DS_MODE);
    if (ret != 0) {
	env->dbe->close(env->dbe, 0);
	print_error(__FILE__, __LINE__, "DB_ENV->open, err: %d, %s", ret, db_strerror(ret));
	switch (ret) {
	    case DB_RUNRECOVERY:
		diag_dbeopen(flags, bfp);
		break;
	    case EINVAL:
		fprintf(stderr, "\n"
			"If you have just got a message that only private environments are supported,\n"
			"your Berkeley DB %d.%d was not configured properly.\n"
			"Bogofilter requires shared environments to support Berkeley DB transactions.\n",
			DB_VERSION_MAJOR, DB_VERSION_MINOR);
		fprintf(stderr,
			"Reconfigure and recompile Berkeley DB with the right mutex interface,\n"
			"see the docs/ref/build_unix/conf.html file that comes with your db source code.\n"
			"This can happen when the DB library was compiled with POSIX threads\n"
			"but your system does not support NPTL.\n");
		break;
	}

	exit(EX_ERROR);
    }

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "DB_ENV->open(home=%s)\n", bfp->dirname);

    return env;
}

static void dbx_cleanup(dbe_t *env)
{
    dbx_cleanup_lite(env);
}

/* close the environment, but do not release locks */
static void dbx_cleanup_lite(dbe_t *env)
{
    if (env) {
	if (env->dbe) {
	    int ret;

	    /* checkpoint if more than 64 kB of logs have been written
	     * or 120 min have passed since the previous checkpoint */
	    /*                                kB  min flags */
	    ret = BF_TXN_CHECKPOINT(env->dbe, 64, 120, 0);
	    ret = dbx_sync(env->dbe, ret);
	    if (ret)
		print_error(__FILE__, __LINE__, "DBE->dbx_checkpoint err: %d, %s", ret, db_strerror(ret));

	    if (db_log_autoremove)
		dbe_env_purgelogs(env->dbe);

	    ret = env->dbe->close(env->dbe, 0);
	    if (DEBUG_DATABASE(1) || ret)
		fprintf(dbgout, "DB_ENV->close(%p): %s\n", (void *)env->dbe,
			db_strerror(ret));
	    clear_lock();
	    if (lockfd >= 0)
		close(lockfd); /* release locks */
	}

	xfree(env->directory);
	xfree(env);
    }
}

static int dbx_sync(DB_ENV *dbe, int ret)
{
#if DB_AT_LEAST(3,0) && DB_AT_MOST(4,0)
    /* flush dirty pages in buffer pool */
    while (ret == DB_INCOMPLETE) {
	rand_sleep(10000,1000000);
	ret = BF_MEMP_SYNC(dbe, NULL);
    }
#else
    (void)dbe;
    ret = 0;
#endif

    return ret;
}

ex_t dbx_recover(bfpath *bfp, bool catastrophic, bool force)
{
    dbe_t *env = xcalloc(1, sizeof(dbe_t));

    /* set exclusive/write lock for recovery */
    while ((force || needs_recovery())
	    && (db_try_glock(bfp, F_WRLCK, F_SETLKW) <= 0))
	rand_sleep(10000,1000000);

    /* ok, when we have the lock, a concurrent process may have
     * proceeded with recovery */
    if (!(force || needs_recovery()))
	return EX_OK;

    if (DEBUG_DATABASE(0))
        fprintf(dbgout, "running %s data base recovery\n",
	    catastrophic ? "catastrophic" : "regular");
    env = dbe_xinit(env, bfp,
		    catastrophic ? DB_RECOVER_FATAL : DB_RECOVER);
    if (env == NULL) {
	exit(EX_ERROR);
    }

    clear_lockfile();
    dbx_cleanup_lite(env);

    return EX_OK;
}

static ex_t dbx_common_close(DB_ENV *dbe, bfpath *bfp)
{
    int e;

    if (db_log_autoremove)
	dbe_env_purgelogs(dbe);

    if (DEBUG_DATABASE(0))
	fprintf(dbgout, "closing environment\n");

    e = dbe->close(dbe, 0);
    if (e != 0) {
	print_error(__FILE__, __LINE__, "Error closing environment \"%s\": %s",
		    bfp->dirname, db_strerror(e));
	exit(EX_ERROR);
    }

    clear_lock();
    db_try_glock(bfp, F_UNLCK, F_SETLKW); /* release lock */
    return EX_OK;
}

static ex_t dbe_env_purgelogs(DB_ENV *dbe)
{
    char **i, **list;
    ex_t e;

    /* figure redundant log files and nuke them */
    e = BF_LOG_ARCHIVE(dbe, &list, DB_ARCH_ABS);
    if (e != 0) {
	print_error(__FILE__, __LINE__,
		"DB_ENV->log_archive failed: %s",
		db_strerror(e));
	exit(EX_ERROR);
    }

    if (list != NULL) {
	if (DEBUG_DATABASE(0))
	    fprintf(dbgout, "removing inactive logfiles\n");

	for (i = list; *i != NULL; i++) {
	    if (DEBUG_DATABASE(1))
		fprintf(dbgout, " removing logfile %s\n", *i);
	    if (unlink(*i)) {
		if (errno != ENOENT)
		    print_error(__FILE__, __LINE__,
			    "cannot unlink \"%s\": %s", *i, strerror(errno));
		/* proceed anyways */
	    }
	}
	xfree(list);
    }
    return EX_OK;
}

/** checkpoint the open environment \a dbe once and unconditionally */
static ex_t dbe_env_checkpoint(DB_ENV *dbe) {
    int e;

    if (DEBUG_DATABASE(0))
	fprintf(dbgout, "checkpoint database\n");

    /* checkpoint the transactional system */
    e = BF_TXN_CHECKPOINT(dbe, 0, 0, 0);
    e = dbx_sync(dbe, e);
    if (e != 0) {
	print_error(__FILE__, __LINE__, "DB_ENV->txn_checkpoint failed: %s",
		db_strerror(e));
	exit(EX_ERROR);
    }

    return EX_OK;
}

static ex_t dbe_simpleop(bfpath *bfp, ex_t func(DB_ENV *env))
{
    ex_t e;
    DB_ENV *dbe = dbe_recover_open(bfp, 0);

    if (dbe == NULL)
	exit(EX_ERROR);

    e = func(dbe);
    dbx_common_close(dbe, bfp);
    return e;
}

ex_t dbx_checkpoint(bfpath *bfp)
{
    return dbe_simpleop(bfp, dbe_env_checkpoint);
}

static ex_t i_purgelogs(DB_ENV *dbe)
{
    int e = dbe_env_checkpoint(dbe);

    if (e != 0)
	return e;
    else
	return dbe_env_purgelogs(dbe);
}

ex_t dbx_purgelogs(bfpath *bfp)
{
    return dbe_simpleop(bfp, i_purgelogs);
}

ex_t dbx_remove(bfpath *bfp)
{
    DB_ENV *dbe = dbe_recover_open(bfp, DB_PRIVATE);

    if (dbe == NULL)
	exit(EX_ERROR);

    return dbx_common_close(dbe, bfp);
}

void dbx_log_flush(DB_ENV *dbe)
{
    int ret;

    ret = BF_LOG_FLUSH(dbe, NULL);

    if (DEBUG_DATABASE(1))
	fprintf(dbgout, "DB_ENV->log_flush(%p): %s\n", (void *)dbe,
		db_strerror(ret));
}

const char **dsm_help_bogofilter(void)
{
    static const char *help_text[] = {
	NULL
    };
    return &help_text[0]; 
}

const char **dsm_help_bogoutil(void)
{
    static const char *help_text[] = {
	"environment maintenance:\n",
	"      --db-transaction=BOOL   - enable or disable transactions\n",
	"                                (only effective at creation time)\n",
	"      --db-verify=file        - verify data file.\n",
	"      --db-checkpoint=dir     - flush buffer cache and checkpoint database\n",
	"      --db-list-logfiles=dir [FLAGS]\n"
	"                              - list logfiles in environment\n",
	"      --db-prune=dir          - remove inactive log files in dir.\n",
	"      --db-recover=dir        - run recovery on database in dir.\n",
	"      --db-recover-harder=dir - run catastrophic recovery on database.\n",
	"      --db-remove-environment - remove environment.\n",

#ifdef	HAVE_DECL_DB_CREATE
	"      --db-lk-max-locks       - set max lock count.\n",
	"      --db-lk-max-objects     - set max object count.\n",
	"      --db-log-autoremove     - set autoremoving of logs.\n",
#ifdef	FUTURE_DB_OPTIONS
	"      --db-txn-durable        - set durable mode.\n",
#endif
#endif
	"\n",
	NULL
    };
    return &help_text[0]; 
}

bool dsm_options_bogofilter(int option, const char *name, const char *val)
{
    switch (option) {
	case O_DB_TRANSACTION:		
	    eTransaction = get_txn(name, val);								return true;

#ifdef	HAVE_DECL_DB_CREATE
	case O_DB_LOG_AUTOREMOVE:	db_log_autoremove	= get_bool(name, val);			return true;
#ifdef	FUTURE_DB_OPTIONS
	case O_DB_TXN_DURABLE:		db_txn_durable		= get_bool(name, val);			return true;
#endif
#endif
	default:				return false;
    }
}

bool dsm_options_bogoutil(int option, cmd_t *flag, int *count, const char **ds_file, const char *name, const char *val)
{
    switch (option) {
	case O_DB_TRANSACTION:
	    eTransaction = get_txn(name, val);
	    return true;

	case O_DB_RECOVER:
	    *flag = M_RECOVER;
	    *count += 1;
	    *ds_file = val;
	    return true;

	case O_DB_RECOVER_HARDER:
	    *flag = M_CRECOVER;
	    *count += 1;
	    *ds_file = val;
	    return true;

	case O_DB_CHECKPOINT:
	    *flag = M_CHECKPOINT;
	    *count += 1;
	    *ds_file = val;
	    return true;

	case O_DB_LIST_LOGFILES:
	    *flag = M_LIST_LOGFILES;
	    *count += 1;
	    *ds_file = val;
	    return true;

	case O_DB_PRUNE:
	    *flag = M_PURGELOGS;
	    *count += 1;
	    *ds_file = val;
	    return true;

	case O_DB_REMOVE_ENVIRONMENT:
	    *flag = M_REMOVEENV;
	    *ds_file = val;
	    *count += 1;
	    return true;

#ifdef	FUTURE_DB_OPTIONS
	case O_DB_TXN_DURABLE:
	    db_txn_durable    = get_bool(name, val);
	    return true;
#endif

	default:
	    return false;
    }
}

/** probe if the directory contains an environment, and if so,
 * if it has transactions
 */
e_txn probe_txn(bfpath *bfp)
{
    DB_ENV *dbe;
    int r;
#if DB_AT_LEAST(4,2)
    u_int32_t flags;
#endif

    r = db_env_create(&dbe, 0);
    if (r) {
	print_error(__FILE__, __LINE__, "cannot create environment handle: %s",
		db_strerror(r));
	return T_ERROR;
    }

    /* we might call dbe->set_flags here to set DB_NOPANIC, but this is
     * only supported from 4.1 onwards and probably not worth the
     * effort, we'll just check for DB_RUNRECOVERY */

#if DB_AT_LEAST(3,2)
    r = dbe->open(dbe, bfp->dirname, DB_JOINENV, DS_MODE);
#else
    r = ENOENT;
#endif
    if (r == DB_RUNRECOVERY) {
	dbe->close(dbe, 0);
	return T_ENABLED;
    }

    if (r == ENOENT) {
	struct stat st;
	int w;
	char *t = bfp->filepath;
	struct dirent *de;
	e_txn rc = T_DONT_KNOW;
	DIR *d;

	/* no environment found by JOINENV, but clean up handle */
	dbe->close(dbe, 0);

	/* retry, looking for log\.[0-9]{10} files - needed for instance
	 * after bogoutil --db-remove DIR or when DB_JOINENV is
	 * unsupported */
	d = opendir(bfp->dirname);
	if (d == NULL) {
	    print_error(__FILE__, __LINE__, "cannot open directory %s: %s",
		    t, strerror(r));
	    rc = T_ERROR;
	} else {
	    while ((errno = 0, de = readdir(d))) {
		if (strlen(de->d_name) == 14
		    && strncmp(de->d_name, "log.", 4) == 0
		    && strspn(de->d_name + 4, "0123456789") == 10)
		{
		    rc = T_ENABLED;
		    break;
		}
	    }
	    if (errno)
		rc = T_ERROR;
	    closedir(d);

	    if (rc != T_ERROR && rc != T_ENABLED) {
		w = stat(t, &st);
		if (w == 0) {
		    rc = T_DISABLED;
		} else if (errno != ENOENT) {
		    rc = T_ERROR;
		    print_error(__FILE__, __LINE__, "cannot stat %s: %s",
			    t, db_strerror(r));
		}
	    }
	}
	return rc;
    } /* if (r == ENOENT) for environment join */

    if (r != 0) {
	print_error(__FILE__, __LINE__, "cannot join environment: %s",
		db_strerror(r));
	return T_ERROR;
    }

    /* environment found, validate if it has transactions */
#if DB_AT_LEAST(4,2)
    r = dbe->get_open_flags(dbe, &flags);
    if (r) {
	print_error(__FILE__, __LINE__, "cannot query flags: %s",
		db_strerror(r));
	return T_ERROR;
    }

    dbe->close(dbe, 0);
    if ((flags & DB_INIT_TXN) == 0) {
	print_error(__FILE__, __LINE__,
		"environment found but does not support transactions.");
	return T_ERROR;
    }
#else
    dbe->close(dbe, 0);
#endif
    return T_ENABLED;
}

static ex_t dbx_list_logfiles(bfpath *bfp, int argc, char **argv)
{
    ex_t e;
    char **list, **i;
    int j;
    u_int32_t flags = 0;
    DB_ENV *dbe = dbe_recover_open(bfp, 0);

    if (dbe == NULL)
	exit(EX_ERROR);

    for (j = 0; j < argc; j++) {
	if (strcasecmp(argv[j], "all") == 0)
	    flags |= DB_ARCH_LOG;
	if (strcasecmp(argv[j], "absolute") == 0)
	    flags |= DB_ARCH_ABS;
    }

    e = BF_LOG_ARCHIVE(dbe, &list, flags);
    if (e != 0) {
	print_error(__FILE__, __LINE__,
		"DB_ENV->log_archive failed: %s",
		db_strerror(e));
	exit(EX_ERROR);
    }

    if (list) {
	for (i = list; *i; i++) {
	    if (flags & DB_ARCH_ABS)
		puts(*i);
	    else
		printf("%s%s%s\n", bfp->dirname, DIRSEP_S, *i);
	}
    }

    xfree(list);
    fflush(stdout);
    e = ferror(stdout) ? EX_ERROR : EX_OK;

    if (dbx_common_close(dbe, bfp)) e = EX_ERROR;
    return e;
}
