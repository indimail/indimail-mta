/* $Id: datastore_db_private.h 5840 2005-04-29 11:44:43Z relson $ */

/*****************************************************************************

NAME:

datastore_db_private.h - provide OO interface for datastore methods;
		       	 used for Berkeley DB transactional &
		       	 non-transactional support.

AUTHOR:
David Relson	<relson@osagesoftware.com> 2005

******************************************************************************/

#ifndef DATASTORE_DB_PRIVATE_H
#define DATASTORE_DB_PRIVATE_H

#define MAGIC_DBE 0xdbe
#define MAGIC_DBH 0xdb4

/** implementation internal type to keep track of databases
 * we have opened. */

#ifndef	ENABLE_SQLITE_DATASTORE
typedef struct {
    int		magic;
    char	*path;
    char	*name;
    int		fd;		/* file descriptor of data base file */
    dbmode_t	open_mode;	/* datastore open mode, DS_READ/DS_WRITE */
#ifdef	ENABLE_DB_DATASTORE	/* if Berkeley DB */
    DB		*dbp;		/* data base handle */
#endif
    bool	locked;
    bool	is_swapped;	/* set if CPU and data base endianness differ */
    bool	created;	/* if newly created; for datastore.c (to add .WORDLIST_VERSION) */
    dbe_t	*dbenv;		/* "parent" environment */
#ifdef	ENABLE_DB_DATASTORE	/* if Berkeley DB */
    DB_TXN	*txn;		/* transaction in progress or NULL */
#endif
    /** OO database methods */
    dsm_t	*dsm;
} dbh_t;
#endif

#define DBT_init(dbt)		(memset(&dbt, 0, sizeof(DBT)))

e_txn probe_txn(bfpath *bfp);

#endif
