/* $Id: xmalloc.h 5649 2005-03-16 00:52:05Z relson $ */

#ifndef XMALLOC_H
#define XMALLOC_H

#include <stddef.h>
#include <stdlib.h>

/* special defines for xmalloc.c, xcalloc.c, etc */
#ifndef	ENABLE_MEMDEBUG
  #define bf_malloc  malloc
  #define bf_calloc  calloc
  #define bf_realloc realloc
  #define bf_free    free
#else
  #include "memdebug.h"
#endif

/*@noreturn@*/
/** print out of memory error and exit program */
void xmem_error(const char *)
#ifdef __GNUC__
 __attribute__((noreturn))
#endif
   ;

/*@noreturn@*/
/** print string too long error and exit program */
void xmem_toolong(const char *)
#ifdef __GNUC__
 __attribute__((noreturn))
#endif
   ;

/*@only@*/ /*@out@*/ /*@notnull@*/
/** allocate \a size bytes of memory, exit program on allocation failure
 */
void *xmalloc(size_t size);

/** free memory area at \a ptr if ptr is non-NULL, do nothing if \a ptr
 * is NULL */
void xfree(/*@only@*/ /*@null@*/ void *ptr);

/** allocate and clear \a nmemb blocks of \a size bytes of memory, exit
 * program on allocation failure */
/*@only@*/ /*@out@*/ /*@notnull@*/
void *xcalloc(size_t nmemb, size_t size);

/** reallocate \a size bytes of memory and initialize it with the
 * first bytes of the shorter area (old in \a ptr vs.\ newly allocated),
 * exit program on allocation failure */
/*@only@*/ /*@out@*/ /*@notnull@*/
void *xrealloc(/*@only@*/ void *ptr, size_t size);

#endif
