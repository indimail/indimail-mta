#ifndef	maildirnewshared_h
#define	maildirnewshared_h

/*
** Copyright 2003-2004 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<sys/types.h>

#if HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif

#ifdef  __cplusplus
extern "C" {
#endif


/*
** New-style shared folder support.
*/

extern int maildir_newshared_disabled;
	/*
	** Set this to 1 to effectively disable new-style shared folders.
	** The functions below will function normally, but as if there are
	** no shared folders configured (the top level configuration file is
	** empty).
	*/

/*
** Read the shared folder index file, which points to everyone else's
** maildirs.
**
** maildir_newshared_enum() invokes the callback function for each listed
** shared folder.
** The callback function receives a structure with the following fields:
**
** Name, home directory, maildir directory, uid/gid of the shared account, the
** pass-through argument.
**
** The maildir directory will get defaulted to "./Maildir", if not
** specified.
**
** A NULL home directory means that the administrator configured a hierarchy
** of shared folders, and the maildir argument points to another shared
** folder hierarchy, whose name is given.
**
** A nonzero return from the callback function terminates the enumeration,
** and maildir_newshared_enum itself returns non-zero.  A zero return
** continues the enumeration.  After the entire list is enumerated
** maildir_newshared_enum returns 0.
**
** maildir_newshared_enum returns -1 if indexfile cannot be opened.
*/

struct maildir_newshared_enum_cb {
	off_t startingpos; /* In index file */
	const char *name;
	const char *homedir;
	const char *maildir;
	uid_t uid;
	gid_t gid;
	void *cb_arg;
	const char *indexfile;	/* Original index file */
	FILE *fp;		/* INTERNAL USE */
	size_t linenum;		/* INTERNAL USE */
};

/*
** Open an index file.  Returns 0 on success, -1 on failure.
** Initializes maildir_newshared_enum_cb structure.
*/

int maildir_newshared_open(const char *indexfile,
			   struct maildir_newshared_enum_cb *info);

/*
** Read next record from the index file.  Invokes the callback function,
** and returns the return value from the callback function.
**
** If reached end of index file, sets *eof to non-zero and returns 0.
*/

int maildir_newshared_next(struct maildir_newshared_enum_cb *info,
			   int *eof,
			   int (*cb_func)(struct maildir_newshared_enum_cb *),
			   void *cb_arg);

/*
** Same as maildir_newshared_next, but seeks to the given offset first.
*/

int maildir_newshared_nextAt(struct maildir_newshared_enum_cb *info,
			     int *eof,
			     int (*cb_func)(struct maildir_newshared_enum_cb*),
			     void *cb_arg);

/* Close it */

void maildir_newshared_close(struct maildir_newshared_enum_cb *info);

int maildir_newshared_enum(const char *indexfile,
			   int (*cb_func)(struct maildir_newshared_enum_cb *),
			   void *cb_arg);


/****************************************************************
 **
 ** High level access to the shared folder index
 **
 ****************************************************************/

/*
** Application defines this function that returns the filename of the
** top level shared cache index file:
*/
extern const char *maildir_shared_index_file();

/*
** Anticipate common shared folder index usage patterns, buffer levels
** of index cache files in memory.
**
** An entire shared folder index cache file is loaded into the following
** structure:
*/

struct maildir_shindex_cache {
	struct maildir_shindex_cache *next; /* Next level cached, if any */
	char *hierarchy; /* Always "" for the topmost level,
			 ** then "foo", "bar", etc... */

	struct maildir_newshared_enum_cb indexfile; /* Opened index file */

	size_t nrecords; /* # of cached records */
	struct maildir_shindex_record *records; /* The cached array */
};

struct maildir_shindex_record {
	char *name;
	off_t offset; /* Its starting position in indexfile, get rest of data
		      ** there.
		      */
};

/*
** The following function finds a shared index file for the following
** subhierarchy.
**
** The subhierarchy is references by its parent loaded hierarchy, and the
** subhierarchy name.
**
** If parent's next->hierarchy happens to be the same as the name of the
** requested hierarchy, it's already loaded, and nothing needs to be done.
**
*/

struct maildir_shindex_cache *
maildir_shared_cache_read(struct maildir_shindex_cache *parent,
			  const char *indexfile,
			  const char *subhierarchy);
/*
** Set all three argument to NULL in order to cache the topmost cache level.
**
** Else set 'parent' to an existing entry, indexfile to the index file
** (obtained from maildir_newshared_next's callback), and subhierarchy
** to the subhierarchy name (same source) to read a subhierarchy.
**
*/


#ifdef  __cplusplus
}
#endif


#endif
