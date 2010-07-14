/* $Id: datastore_txn_dummies.c 5794 2005-04-11 19:06:34Z m-a $ */

/*****************************************************************************

NAME:
datastore_db_trans_stub.c -- stub routines for bogofilter's transactional
		       datastore, using Berkeley DB

AUTHORS:

David Relson	<relson@osagesoftware.com> 2005

******************************************************************************/

#include "common.h"

#include "datastore.h"
#include "datastore_db.h"
#include "datastore_db_private.h"

/* OO function lists */

dsm_t dsm_dummies = {
    /* public -- used in datastore.c */
    NULL,	/* dsm_begin            */
    NULL,	/* dsm_abort            */
    NULL,	/* dsm_commit           */

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
    NULL,	/* dsm_checkpoint       */
    NULL,	/* dsm_pagesize         */
    NULL,	/* dsm_purgelogs        */
    NULL,	/* dsm_recover          */
    NULL,	/* dsm_remove           */
    NULL,	/* dsm_verify           */
    NULL,	/* dsm_list_logfiles    */
    NULL	/* dsm_leafpages        */
};

dsm_t *dsm = &dsm_dummies;

ex_t dbe_recover(bfpath *bfp, bool catastrophic, bool force)
{
    (void) bfp;
    (void) catastrophic;
    (void) force;

    fprintf(stderr,
	    "ERROR: bogofilter can not recover databases without transaction support.\n"
	    "If you experience hangs, strange behavior, inaccurate output,\n"
	    "you must delete your data base and rebuild it, or restore an older version\n"
	    "that you know is good from your backups.\n");
    exit(EX_ERROR);
}
