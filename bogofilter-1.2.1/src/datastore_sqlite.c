/**
 * \file datastore_sqlite.c SQLite 3 database driver back-end
 * \author Matthias Andree <matthias.andree@gmx.de>
 * \date 2004, 2005
 *
 * This file handles a static table named "bogofilter" in a SQLite3
 * database. The table has two "BLOB"-typed columns, key and value.
 *
 * GNU GENERAL PUBLIC LICENSE v2
 */

#include "common.h"

#include <errno.h>
#include <sqlite3.h>

#include "datastore_db.h"

#include "error.h"
#include "rand_sleep.h"
#include "xmalloc.h"
#include "xstrdup.h"

/** Structure to hold database handle and associated data. */
struct dbhsqlite_t {
    char *path;	   /**< directory to hold database */
    char *name;	   /**< database file name */
    sqlite3 *db;   /**< pointer to SQLite3 handle */
    sqlite3_stmt *select; /**< prepared SELECT statement for DB retrieval */
    sqlite3_stmt *insert; /**< prepared INSERT OR REPLACE for DB update */
    sqlite3_stmt *delete; /**< prepared DELETE statement */
    bool created;  /**< gets set by db_open if it created the database new */
    bool swapped;  /**< if endian swapped on disk vs. current host */
};

/** Convenience shortcut to avoid typing "struct dbh_t" */
typedef struct dbhsqlite_t dbh_t;

static const char *ENDIAN32 = ".ENDIAN32";

void db_flush(void *unused) { (void)unused; }

static int sql_txn_begin(void *vhandle);
static int sql_txn_abort(void *vhandle);
static int sql_txn_commit(void *vhandle);
static u_int32_t sql_pagesize(bfpath *bfp);
static ex_t sql_verify(bfpath *bfp);

/** The layout of the bogofilter table, formatted as SQL statement.
 *
 * The additional index, although making writes a bit slower, speeds up
 * queries noticably as it improves locality of referenced data and
 * reduces complexity of the retrieval of the value column.
 */
#define LAYOUT \
	"CREATE TABLE bogofilter (" \
	"   key   BLOB PRIMARY KEY," \
	"   value BLOB);" \
	"CREATE INDEX bfidx ON bogofilter(key,value);"
/*
 * another experimental layout is as follows,
 * but does not appear to make a lot of difference
 * performance-wise (evaluation in other environments
 * is required though):
 *
#define LAYOUT \
    "CREATE TABLE bogofilter (key BLOB, value BLOB); " \
    "CREATE INDEX bfidx ON bogofilter(key,value);" \
    "CREATE TRIGGER bfuniquekey BEFORE INSERT ON bogofilter " \
    " FOR EACH ROW WHEN EXISTS(SELECT key FROM bogofilter WHERE (key=NEW.key) LIMIT 1) " \
    " BEGIN UPDATE bogofilter SET value=NEW.value WHERE (key=NEW.key); SELECT RAISE(IGNORE); END;"
#endif
 */

dsm_t dsm_sqlite = {
    /* public -- used in datastore.c */
    &sql_txn_begin,
    &sql_txn_abort,
    &sql_txn_commit,

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
    &sql_pagesize,/* dsm_pagesize       */
    NULL,	/* dsm_purgelogs        */
    NULL,	/* dsm_checkpoint       */
    NULL,	/* dsm_recover          */
    NULL,	/* dsm_remove           */
    &sql_verify,/* dsm_verify           */
    NULL,	/* dsm_list_logfiles    */
    NULL	/* dsm_leafpages        */
};

dsm_t *dsm = &dsm_sqlite;

/** The command to begin a regular transaction. */
#define BEGIN \
	"BEGIN TRANSACTION;"

/* real functions */
/** Initialize database handle and return it.
 * \returns non-NULL, as it exits with EX_ERROR in case of trouble. */
static dbh_t *dbh_init(bfpath *bfp)
{
    dbh_t *handle;

    dsm = &dsm_sqlite;

    handle = xmalloc(sizeof(dbh_t));
    memset(handle, 0, sizeof(dbh_t));

    handle->name = xstrdup(bfp->filepath);

    return handle;
}

/** Free internal database handle \a dbh. */
static void free_dbh(dbh_t *dbh) {
    if (!dbh)
	return;
    xfree(dbh->name);
    xfree(dbh->path);
    xfree(dbh);
}

/** Executes the SQL statement \a cmd on the database \a db and returns
 * the sqlite3_exec return code. If the return code is nonzero, this
 * routine will have printed an error message.
 */
static int sqlexec(sqlite3 *db, const char *cmd) {
    char *e = NULL;
    int rc;

    rc = sqlite3_exec(db, cmd, NULL, NULL, &e);
    if (rc) {
	print_error(__FILE__, __LINE__,
		"Error executing \"%s\": %s (#%d)\n",
		cmd, e ? e : "NULL", rc);
	if (e)
	    sqlite3_free(e);
    }
    return rc;
}

static sqlite3_stmt *sqlprep(dbh_t *dbh, const char *cmd, bool bailout /** exit on error? */) {
    const char *tail; /* dummy */
    sqlite3_stmt *ptr;
    if (sqlite3_prepare_v2(dbh->db, cmd, strlen(cmd), &ptr, &tail) != SQLITE_OK) {
	print_error(__FILE__, __LINE__, "cannot compile %s: %s\n", cmd, sqlite3_errmsg(dbh->db));
	if (bailout)
	    exit(EX_ERROR);
	return NULL;
    }
    return ptr;
}

/** Short trace handler function, passed to SQLite if debugging is
 * enabled. */
static void db_trace(void *userdata /** unused */,
	const char *log /** log message */) {
    (void)userdata;
    fprintf(dbgout, "SQLite[%ld]: %s\n", (long)getpid(), log);
}

/** Foreach function, we call \a hook for
 * each (key, value) tuple in the database.
 */
static int db_loop(sqlite3 *db,	/**< SQLite3 database handle */
	const char *cmd,	/**< SQL command to obtain data */
	db_foreach_t hook,	/**< if non-NULL, called for each value */
	void *userdata		/**  this is passed to the \a hook */
	) {
    const char *tail;
    sqlite3_stmt *stmt;
    int rc;
    bool loop, found = false;
    dbv_t key, val;

    /* sqlite3_exec doesn't allow us to retrieve BLOBs */
    rc = sqlite3_prepare_v2(db, cmd, strlen(cmd), &stmt, &tail);
    if (rc) {
	print_error(__FILE__, __LINE__,
		"Error preparing \"%s\": %s (#%d)\n",
		cmd, sqlite3_errmsg(db), rc);
	sqlite3_finalize(stmt);
	return rc;
    }
    loop = true;
    while (loop) {
	rc = sqlite3_step(stmt);
	switch (rc) {
	    case SQLITE_ROW:
		found = true;
		if (hook != NULL)
		{
		    key.leng = sqlite3_column_bytes(stmt, /* column */ 0);
		    key.data = xmalloc(key.leng);
		    memcpy(key.data, sqlite3_column_blob(stmt, 0), key.leng);

		    val.leng = sqlite3_column_bytes(stmt, /* column */ 1);
		    val.data = xmalloc(val.leng);
		    memcpy(val.data, sqlite3_column_blob(stmt, 1), val.leng);

		    /* skip ENDIAN32 token */
		    if (key.leng != strlen(ENDIAN32)
			    || memcmp(key.data, ENDIAN32, key.leng) != 0)
			rc = hook(&key, &val, userdata);
		    else
			rc = 0;

		    xfree(val.data);
		    xfree(key.data);
		    if (rc) {
			sqlite3_finalize(stmt);
			return rc;
		    }
		}
		break;
	    case SQLITE_DONE:
		loop = false;
		break;
	    default:
		print_error(__FILE__, __LINE__, "Error executing \"%s\": %s (#%d)\n",
		cmd, sqlite3_errmsg(db), rc);
		sqlite3_finalize(stmt);
		return rc;
	}
    }
    /* free resources */
    sqlite3_finalize(stmt);
    return found ? 0 : DS_NOTFOUND;
}

/** This busy handler just sleeps a while and retries */
static int busyhandler(void *dummy, int count)
{
    (void)dummy;
    (void)count;
    rand_sleep(1000, 1000000);
    return 1;
}

static void check_sqlite_version(void)
{
    const unsigned int wmaj = 3, wmin = 5, wpl = 4;	/* desired version of sqlite3 library */
    unsigned int vmaj, vmin, vpl;			/* actual version of sqlite3 library */
    static int complained;
    const char *v;

    if (complained)
	return;
    complained = 1;
    v = sqlite3_libversion();
    sscanf(v, "%u.%u.%u", &vmaj, &vmin, &vpl);
    if (vmaj > wmaj) return;
    if (vmaj == wmaj && vmin > wmin) return;
    if (vmaj == wmaj && vmin == wmin && vpl >= wpl) return;
    if (!getenv("BF_USE_OLD_SQLITE"))
	fprintf(stderr,
		"\n"
		"WARNING: please update sqlite to %d.%d.%d or newer.\n"
		"\n", wmaj, wmin, wpl);
}

void *db_open(void *dummyenv, bfpath *bfp, dbmode_t mode)
{
    int rc;
    dbh_t *dbh;
    dbv_t k, v;

    (void)dummyenv;

    check_sqlite_version();

    dbh = dbh_init(bfp);

    /* open database file */
    if (DEBUG_DATABASE(1) || getenv("BF_DEBUG_DB")) {
	fprintf(dbgout, "SQLite: db_open(%s)\n", dbh->name);
	fflush(dbgout);
    }
    rc = sqlite3_open(dbh->name, &dbh->db);
    if (rc) {
	print_error(__FILE__, __LINE__, "Can't open database %s: %s\n",
		dbh->name, sqlite3_errmsg(dbh->db));
	goto barf;
    }

    /* request extended result codes for improved error reporting */
    (void)sqlite3_extended_result_codes(dbh->db, true);

    /* set trace mode */
    if (DEBUG_DATABASE(1) || getenv("BF_DEBUG_DB"))
	sqlite3_trace(dbh->db, db_trace, NULL);

    /* set busy handler */
    if (sqlite3_busy_handler(dbh->db, busyhandler, NULL)) {
	print_error(__FILE__, __LINE__, "Can't set busy handler: %s\n",
		sqlite3_errmsg(dbh->db));
	goto barf;
    }

    /* check/set endianness marker and create table if needed */
    if (mode != DS_READ) {
	/* using IMMEDIATE or DEFERRED here locks up in t.lock3
	 * or t.bulkmode
	 * using EXCLUSIVE locks up in t.lock3 on MAC OSX
	 */
	if (sqlexec(dbh->db, BEGIN)) goto barf;
	/*
	 * trick: the sqlite_master table (see SQLite FAQ) is read-only
	 * and lists all tables, indexes etc. so we use it to check if
	 * the bogofilter table is already there, the error codes are
	 * too vague either way, for "no such table" and "table already
	 * exists" we always get SQLITE_ERROR, which we'll also get for
	 * syntax errors, such as "EXCLUSIVE" not supported on older
	 * versions :-(
	 */
	rc = db_loop(dbh->db, "SELECT name FROM sqlite_master "
		"WHERE type='table' AND name='bogofilter';",
		NULL, NULL);
	switch (rc) {
	    case 0:
		if (sqlexec(dbh->db, "COMMIT;")) goto barf;
		break;
	    case DS_NOTFOUND:
		{
		    u_int32_t p[2] = { 0x01020304, 0x01020304 };

		    if (sqlexec(dbh->db, LAYOUT)) goto barf;

		    /* set endianness marker */
		    k.data = xstrdup(ENDIAN32);
		    k.leng = strlen(k.data);
		    v.data = p;
		    v.leng = sizeof(p);
		    rc = db_set_dbvalue(dbh, &k, &v);
		    xfree(k.data);
		    if (rc)
			goto barf;

		    if (sqlexec(dbh->db, "COMMIT;")) goto barf;
		    dbh->created = true;
		}
		break;
	    default:
		goto barf;
	}
    }

    /*
     * initialize common statements
     * dbh->insert is not here as it's needed earlier,
     * so it sets itself up lazily
     */
    dbh->select = sqlprep(dbh, "SELECT value FROM bogofilter WHERE key=? LIMIT 1;", false);
    if (dbh->select == NULL)
    {
	fprintf(stderr,
		"\nRemember to register some spam and ham messages before you\n"
		"use bogofilter to evaluate mail for its probable spam status!\n\n");
	exit(EX_ERROR);
    }

    dbh->delete = sqlprep(dbh, "DELETE FROM bogofilter WHERE(key = ?);", true);

    /* check if byteswapped */
    {
	u_int32_t t, b[2];
	int ee;

	k.data = xstrdup(ENDIAN32);
	k.leng = strlen(k.data);
	v.data = b;
	v.leng = sizeof(b);

	ee = db_get_dbvalue(dbh, &k, &v);
	xfree(k.data);
	switch (ee) {
	    case 0: /* found endian marker token, read it */
		if (v.leng < 4)
		    goto barf;
		t = ((u_int32_t *)v.data)[0];
		switch (t) {
		    case 0x01020304: /* same endian, "UNIX" */
			dbh->swapped = false;
			break;
		    case 0x04030201: /* swapped, "XINU" */
			dbh->swapped = true;
			break;
		    default: /* NUXI or IXUN or crap */
			print_error(__FILE__, __LINE__,
				"Unknown endianness on %s: %08x.\n",
				dbh->name, ((u_int32_t *)v.data)[0]);
			goto barf2;
		}
		break;
	    case DS_NOTFOUND: /* no marker token, assume not swapped */
		dbh->swapped = false;
		break;
	    default:
		goto barf;
	}
    }

    return dbh;
barf:
    print_error(__FILE__, __LINE__, "Error on database %s: %s\n",
	    dbh->name, sqlite3_errmsg(dbh->db));
barf2:
    db_close(dbh);
    return NULL;
}

void db_close(void *handle) {
    int rc;
    dbh_t *dbh = handle;
    if (dbh->delete) sqlite3_finalize(dbh->delete);
    if (dbh->insert) sqlite3_finalize(dbh->insert);
    if (dbh->select) sqlite3_finalize(dbh->select);
    rc = sqlite3_close(dbh->db);
    if (rc) {
	print_error(__FILE__, __LINE__, "Can't close database %s: %d",
		dbh->name, rc);
	exit(EX_ERROR);
    }
    free_dbh(dbh);
}

const char *db_version_str(void) {
    static char buf[80];

    if (!buf[0])
	snprintf(buf, sizeof(buf), "SQLite %s", sqlite3_libversion());
    return buf;
}

static int sql_txn_begin(void *vhandle) {
    dbh_t *dbh = vhandle;
    return sqlexec(dbh->db,  BEGIN );
}

static int sql_txn_abort(void *vhandle) {
    dbh_t *dbh = vhandle;
    return sqlexec(dbh->db, "ROLLBACK;");
}

static int sql_txn_commit(void *vhandle) {
    dbh_t *dbh = vhandle;
    return sqlexec(dbh->db, "COMMIT;");
}

/** common code for db_delete, db_(get|set)_dbvalue.
 * This works by setting variables in precompiled statements (see PREP,
 * sqlite3_prepare, sqlite3_bind_*, sqlite3_reset) and avoids encoding
 * binary data into SQL's hex representation as well as compiling the
 * same SQL statement over and over again. */
static int sql_fastpath(
	dbh_t *dbh,		/**< database handle */
	const char *func,	/**< function name to report in errors */
	sqlite3_stmt *stmt,	/**< SQLite3 statement to execute/reset */
	dbv_t *val,		/**< OUT value from first row, NULL ok */
	int retnotfound		/**  return value if no rows found */
	)
{
    int rc;
    bool found = false;

    while (1) {
	rc = sqlite3_step(stmt);
	switch (rc) {
	    case SQLITE_ROW:	/* this is the only branch that loops */
		if (val) {
		    int len = min(INT_MAX, val->leng);
		    val->leng = min(len, sqlite3_column_bytes(stmt, 0));
		    memcpy(val->data, sqlite3_column_blob(stmt, 0), val->leng);
		}
		found = 1;
		break;
		/* all other branches below return control to the caller */
	    case SQLITE_BUSY:
		sqlite3_reset(stmt);
		sql_txn_abort(dbh);
		return DS_ABORT_RETRY;

	    case SQLITE_DONE:
		sqlite3_reset(stmt);
		return found ? 0 : retnotfound;

	    default:
		print_error(__FILE__, __LINE__,
			"%s: error executing statement on %s: %s (%d)\n",
			func, dbh->name, sqlite3_errmsg(dbh->db), rc);
		sqlite3_reset(stmt);
		return rc;
	}
    }
}

int db_delete(void *vhandle, const dbv_t *key) {
    dbh_t *dbh = vhandle;

    sqlite3_bind_blob(dbh->delete, 1, key->data, key->leng, SQLITE_STATIC);
    return sql_fastpath(dbh, "db_delete", dbh->delete, NULL, 0);
}

int db_set_dbvalue(void *vhandle, const dbv_t *key, const dbv_t *val) {
    dbh_t *dbh = vhandle;

    if (!dbh->insert)
	dbh->insert = sqlprep(dbh, "INSERT OR REPLACE INTO bogofilter VALUES(?,?);", true);

    sqlite3_bind_blob(dbh->insert, 1, key->data, key->leng, SQLITE_STATIC);
    sqlite3_bind_blob(dbh->insert, 2, val->data, val->leng, SQLITE_STATIC);
    return sql_fastpath(dbh, "db_set_dbvalue", dbh->insert, NULL, 0);
}

int db_get_dbvalue(void *vhandle, const dbv_t* key, /*@out@*/ dbv_t *val) {
    dbh_t *dbh = vhandle;

    sqlite3_bind_blob(dbh->select, 1, key->data, key->leng, SQLITE_STATIC);
    return sql_fastpath(dbh, "db_get_dbvalue", dbh->select, val, DS_NOTFOUND);
}

ex_t db_foreach(void *vhandle, db_foreach_t hook, void *userdata) {
    dbh_t *dbh = vhandle;
    const char *cmd = "SELECT key, value FROM bogofilter;";
    return db_loop(dbh->db, cmd, hook, userdata);
}

const char *db_str_err(int e) {
    return e == 0 ? "no error" : "unknown condition (not yet implemented)";
}

bool db_created(void *vhandle) {
    dbh_t *dbh = vhandle;
    return dbh->created;
}

bool db_is_swapped(void *vhandle) {
    dbh_t *dbh = vhandle;
    return dbh->swapped;
}

static int pagesize_cb(void *ptr, int argc, char **argv, char **dummy) {
    u_int32_t *uptr = ptr;

    (void)dummy;

    if (argc != 1)
	return -1;
    errno = 0;
    *uptr = strtoul(argv[0], NULL, 0);
    return errno;
}

static u_int32_t sql_pagesize(bfpath *bfp)
{
    dbh_t *dbh;
    int rc;
    u_int32_t size;

    dbh = db_open(NULL, bfp, DS_READ);
    if (!dbh)
	return 0xffffffff;
    rc = sqlite3_exec(dbh->db, "PRAGMA page_size;", pagesize_cb, &size, NULL);
    if (rc != SQLITE_OK) {
	return 0xffffffff;
    }
    db_close(dbh);
    return size;
}

static int cb_first;

/** callback function for sql_verify,
 * \returns 0 to exhaust the full list of error messages. */
static int cb_verify(void *errflag, int columns,
	char * * values, char * * names)
{
    int *e = errflag;

    if (columns != 1) {
	fprintf(stderr, "Strange: got row with more or less than one column.\n"
		"Aborting verification.\n");
	return 1;
    }

    if (cb_first != 1 || 0 != strcmp(values[0], "ok")) {
	*e = 1; /* If we get any output other than a single row with the
		   "ok" text, there's something wrong with the database ->
		   set error flag and print the error. */
	fprintf(stderr, "%s\n", values[0]);
    }

    (void)names;
    cb_first = 0;
    return 0;
}

/** Run database verification PRAGMA. Prints all error messages
 * and returns EX_OK if none were delivered by sqlite3 or EX_ERROR in
 * case of trouble. */
static ex_t sql_verify(bfpath *bfp)
{
    dbh_t *dbh;
    int rc;
    int faulty = 0;
    char *errmsg = NULL;
    const char stmt[] = "PRAGMA integrity_check;";

    if (DEBUG_DATABASE(1) || getenv("BF_DEBUG_DB")) {
	fprintf(dbgout, "SQLite: sql_verify(%s)\n", bfp->filename);
	fflush(dbgout);
    }
    dbh = db_open(NULL, bfp, DS_READ);
    if (!dbh)
	return EX_ERROR;
    cb_first = 1;
    rc = sqlite3_exec(dbh->db, stmt, cb_verify, &faulty, &errmsg);
    if (rc != SQLITE_OK) {
	print_error(__FILE__, __LINE__, "Error while evaluating \"%s\": %s", stmt,
		errmsg ? errmsg : "(unknown)");
	if (errmsg) sqlite3_free(errmsg);
	return EX_ERROR;
    }
    db_close(dbh);
    if (faulty == 0) {
	/* success */
	if (verbose) printf("%s: OK\n", bfp->filename);
    } else {
	/* database faulty */
	fprintf(stderr, "%s: Database integrity check failed.\n", bfp->filename);
    }
    return faulty ? EX_ERROR : EX_OK;
}
