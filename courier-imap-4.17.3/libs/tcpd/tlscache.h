/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef tlscache_h
#define tlscache_h

#include	"config.h"

#include	<sys/types.h>
#include	<unistd.h>


/*
** This module implements a cache for SSL sessions, however the interface
** is generic enough to accomodate caching of any kind of small object.
** The cache is a disk file, that contains a circular cache (when the
** end of the file is reached, we wrap around and begin adding new cached
** object to the beginning of the file, overwriting the oldest one).
**
** Well, that's the general idea, but technically it's the other way around.
** The cache begins at the end of the file, and grows towards the beginning
** of the file, then wraps around to the end of the file again.  The cache
** is searched by reading the file starting with wherever the current add
** position is, then wrapping around if necessary.  Hence, the cache file
** is searched starting with the most recently added object; which is the
** expected usage pattern.
**
** File locking is used to implement concurrency.
*/

struct CACHE {
	int fd;
	char *filename;
};

/*
** Open a cache file.  If it doesn't exist, create one with the indicated
** size (in bytes).
*/

struct CACHE *tls_cache_open(const char *filename,
			     off_t req_size);	/* In bytes */

/*
** Close and deallocate the CACHE object.
*/
void tls_cache_close(struct CACHE *p);

/*
** Cache a new object, val, vallen bytes long.
*/

int tls_cache_add(struct CACHE *p, const char *val, size_t vallen);

/*
** Read the cache file.  walk_func is a callback function that's repeatedly
** called for each cached object.  walk_func should return 0 to continue
** with the next cached object; a positive value to stop reading the cache
** (object found); a negative value to stopr eading the cache and remove it
** (if it's corrupted, for some reason).  walk_func receives 'arg', a
** transparent pointer.
**
** tls_cache_walk returns 0 when all cached objects were read, or the non-0
** return value from the callback function.  tls_cache_walk will return -1
** if it itself encounters an error.
**
** The callback function may modify rec, and set *doupdate to non-zero in
** order to update the cached record (the return code is still processed in
** the normal way).  The updated record will be saved if the callback function
** terminate with 0 or a positive return code.
*/

int tls_cache_walk(struct CACHE *p,
		   int (*walk_func)(void *rec, size_t recsize,
				    int *doupdate, void *arg),
		   void *arg);


#endif
