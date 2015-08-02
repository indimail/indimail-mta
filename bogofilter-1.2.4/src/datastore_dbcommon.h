#ifndef DATASTORE_DBCOMMON_H
#define DATASTORE_DBCOMMON_H

#define DB_AT_LEAST(maj, min)	((DB_VERSION_MAJOR > (maj)) || ((DB_VERSION_MAJOR == (maj)) && (DB_VERSION_MINOR >= (min))))
#define DB_AT_MOST(maj, min)	((DB_VERSION_MAJOR < (maj)) || ((DB_VERSION_MAJOR == (maj)) && (DB_VERSION_MINOR <= (min))))
#define DB_EQUAL(maj, min)	((DB_VERSION_MAJOR == (maj)) && (DB_VERSION_MINOR == (min)))

/* wrapper for the API that changed in 4.0, to
 * collect the junk in a location separate from the implementation */
#if DB_AT_LEAST(4,0)
/* BerkeleyDB 4.0, 4.1, 4.2 */
#define BF_LOG_FLUSH(e, i) ((e)->log_flush((e), (i)))
#define BF_MEMP_SYNC(e, l) ((e)->memp_sync((e), (l)))
#define BF_MEMP_TRICKLE(e, p, n) ((e)->memp_trickle((e), (p), (n)))
#define BF_TXN_BEGIN(e, f, g, h) ((e)->txn_begin((e), (f), (g), (h)))
#define BF_TXN_ID(t) ((t)->id(t))
#define BF_TXN_ABORT(t) ((t)->abort((t)))
#define BF_TXN_COMMIT(t, f) ((t)->commit((t), (f)))
#define BF_TXN_CHECKPOINT(e, k, m, f) ((e)->txn_checkpoint((e), (k), (m), (f)))
#define BF_LOG_ARCHIVE(e, l, f) ((e)->log_archive((e), (l), (f)))
#else
/* BerkeleyDB 3.1, 3.2, 3.3 */
#define BF_LOG_FLUSH(e, i) (log_flush((e), (i)))
#define BF_MEMP_SYNC(e, l) (memp_sync((e), (l)))
#define BF_MEMP_TRICKLE(e, p, n) (memp_trickle((e), (p), (n)))
#define BF_TXN_BEGIN(e, f, g, h) (txn_begin((e), (f), (g), (h)))
#define BF_TXN_ID(t) (txn_id(t))
#define BF_TXN_ABORT(t) (txn_abort((t)))
#define BF_TXN_COMMIT(t, f) (txn_commit((t), (f)))
#define BF_TXN_CHECKPOINT(e, k, m, f) (txn_checkpoint((e), (k), (m), (f)))
#if DB_AT_LEAST(3,3)
#define BF_LOG_ARCHIVE(e, l, f) (log_archive((e), (l), (f)))
#else
#define BF_LOG_ARCHIVE(e, l, f) (log_archive((e), (l), (f), NULL))
#endif
#endif

/* DB->stat interface changed in 3.3 and in 4.3 */
#if DB_AT_MOST(3,2)
#define BF_DB_STAT(d, t, s, f) ((d)->stat((d), (s), NULL, (f)))
#endif

#if DB_AT_LEAST(4,3)
#define BF_DB_STAT(d, t, s, f) ((d)->stat((d), (t), (s), (f)))
#endif

#ifndef BF_DB_STAT
#define BF_DB_STAT(d, t, s, f) ((d)->stat((d), (s), (f)))
#endif

#endif
