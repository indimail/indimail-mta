/* $Id: memdebug.c 5708 2005-03-24 04:04:01Z relson $ */

/*
* NAME:
*    memdebug.c -- memory usage debugging layer for malloc(), free(), etc.
*
* AUTHOR:
*    David Relson <relson@osagesoftware.com>
*
* NOTE:
*    These routines are useful, though not especially polished.
*
*    Capabilities:
*
*	- Count calls to malloc(), free(), realloc(), calloc().
*	- For each such call, track current, maximum, and total bytes allocated.
*	- Display summary statistics.
*	- Display size and address of each block as allocated or freed to locate the unfreed blocks.
*	- Trap routine than can be called by allocation index or allocation size.
*	- Program exit when current allocation is "too much".
*/

#define	NO_MEMDEBUG_MACROS

#include "common.h"

#include <stdlib.h>

#include "memdebug.h"
#include "xmalloc.h"

int  memtrace= 1 * (M_MALLOC+M_FREE);

uint32_t dbg_trap_index= 36;
uint32_t dbg_size_min  = 0;
uint32_t dbg_size_max  = 0;
uint32_t dbg_index_min = 0;	/* 1 */
uint32_t dbg_index_max = 0;	/* 1000 */
uint32_t dbg_size_trap = 0;	/* 100000 */

#define	MB 1000000
#define	GB 1000*MB
uint32_t dbg_too_much  = 0; 	/* GB */

uint32_t dbg_size_delt = 0;	/* MB */
uint32_t dbg_delt_save = 0;

const uint32_t md_tag = (uint32_t) 0xABCD55AA;

uint32_t cnt_alloc  = 0;
uint32_t cnt_free   = 0;
uint32_t cnt_malloc = 0;
uint32_t cnt_realloc= 0;
uint32_t cnt_calloc = 0;
uint32_t cur_malloc = 0;
uint32_t max_malloc = 0;
uint32_t tot_malloc = 0;

void md_trap(const char *why);
void md_too_much(void);

void md_trap(const char *why) { (void)why; }

void md_too_much()
{
    static bool first = true;
    if (first) {
	first = false;
	fprintf(stderr, "max_malloc = %12lu, tot_malloc = %12lu\n", 
		(ulong) max_malloc, (ulong) tot_malloc);
	md_trap("dbg_too_much");
    }
}

typedef struct memheader {
    uint32_t	size;
    uint32_t	indx;
    uint32_t	tag;
} mh_t;

typedef struct memtrailer {
    uint32_t	tag;
} mt_t;

void mh_disp(const char *s, mh_t *p);
void mh_disp(const char *s, mh_t *p)
{
    if (!DEBUG_MEMORY(1))
	return;

    if (dbg_index_min != 0 && p->indx < dbg_index_min)
	return;
    if (dbg_index_max != 0 && p->indx > dbg_index_max)
	return;

    if (dbg_size_min != 0 && p->size < dbg_size_min)
	return;
    if (dbg_size_max != 0 && p->size > dbg_size_max)
	return;

    fprintf(dbgout, "::%3d  %08lX  %s  %lu\n", p->indx, (ulong) (p+1), s, (ulong) p->size);

    return;
}

void *
md_malloc(size_t size)
{
    void *ptr;
    mt_t *mt;
    mh_t *mh = NULL;

    ++cnt_malloc;
    cur_malloc += size;
    
    if (dbg_size_delt != 0 &&
	max_malloc - dbg_delt_save > dbg_size_delt) {
	dbg_delt_save = max_malloc / dbg_size_delt * dbg_size_delt;
	fprintf(dbgout, ":: max_malloc = %12lu\n", (ulong) max_malloc);
    }

    max_malloc = max(max_malloc, cur_malloc);
    tot_malloc += size;
    size += sizeof(mh_t) + sizeof(md_tag);		/* Include size storage */

    if (dbg_size_trap != 0 && size > dbg_size_trap)
	md_trap("dbg_size_trap");

    if (dbg_too_much != 0 && max_malloc > dbg_too_much)
	md_too_much();

    ptr = malloc(size);

    mh = (mh_t *) ptr;
    mh->size = size - sizeof(mh_t) - sizeof(md_tag);
    mh->indx = ++cnt_alloc;
    mh->tag  = md_tag;

    if (memtrace & M_MALLOC)
	mh_disp( "a", mh );
    if (dbg_trap_index != 0 && mh->indx == dbg_trap_index)
	md_trap("dbg_index");

    mt = (mt_t *)((char *)(mh+1)+mh->size);
    mt->tag = md_tag;

    ptr = (void *) (mh+1);

    return ptr;
}

void
md_free(void *ptr)
{
    mt_t *mt;
    mh_t *mh = (mh_t *) ptr;

    if (!ptr)
	return;

    mh -= 1;
    ptr = (void *) mh;

    ++cnt_free;
    if (memtrace & M_FREE)
	mh_disp( "f", mh );
    if (mh->tag != md_tag)
	md_trap("md_tag");
    if (dbg_trap_index != 0 && mh->indx == dbg_trap_index)
	md_trap("dbg_trap_index");
    if (mh->size > cur_malloc || 
	(dbg_size_trap != 0 && mh->size > dbg_size_trap))
	md_trap("dbg_size_trap");
    mt = (mt_t *)((char *)(mh+1)+mh->size);
    if (mt->tag != md_tag)
	md_trap("md_tag");

    cur_malloc -= mh->size;

    mh->tag = -1;

    free(ptr);
}

void memdisplay(const char *file, int lineno)
{
    const char *pfx  = "";

    if (!DEBUG_MEMORY(0))
	return;

    if (file != NULL) {
	pfx = "\t";
	fprintf(dbgout, "%s:%d:\n", file, lineno);
    }

    fprintf(dbgout, "%smalloc:  cur = %lu, max = %lu, tot = %lu\n", pfx,
	    (ulong) cur_malloc, (ulong) max_malloc, (ulong) tot_malloc );
    fprintf(dbgout, "%scounts:  malloc: %lu, calloc: %lu, realloc: %lu, free: %lu\n", pfx,
	    (ulong) cnt_malloc, (ulong) cnt_calloc, (ulong) cnt_realloc, (ulong) cnt_free);
    if (cnt_alloc == cnt_free)
	fprintf(dbgout, "%s         none active.\n", pfx);
    else
	fprintf(dbgout, "%s         active: %lu, average: %lu\n", pfx, 
		(ulong) cnt_alloc - cnt_free, (ulong) cur_malloc/(cnt_alloc - cnt_free));
    fflush(dbgout);
}

void
*md_calloc(size_t nmemb, size_t size)
{
    void *ptr;
    mt_t *mt;
    mh_t *mh;

    size = size * nmemb;
    nmemb = 1;
    cur_malloc += size;
    max_malloc = max(max_malloc, cur_malloc);
    tot_malloc += size;
    size += sizeof(mh_t) + sizeof(md_tag);		/* Include size storage */
    ++cnt_calloc;

    if (dbg_too_much != 0 && max_malloc > dbg_too_much) 
	md_too_much();

    ptr = calloc(nmemb, size);

    mh = (mh_t *) ptr;

    mh->size = size - sizeof(mh_t) - sizeof(md_tag);
    mh->indx = ++cnt_alloc;
    mh->tag  = md_tag;

    if (memtrace & M_MALLOC)
	mh_disp( "c", mh );
    if (dbg_trap_index != 0 && mh->indx == dbg_trap_index)
	md_trap("dbg_trap_index");

    mt = (mt_t *)((char *)(mh+1)+mh->size);
    mt->tag = md_tag;

    ptr = (void *) (mh+1);

    return ptr;
}

void
*md_realloc(void *ptr, size_t size)
{
    mh_t *mh = ((mh_t *) ptr)-1;
    mt_t *mt;
    size_t oldsize = mh->size;

    cur_malloc += size - oldsize;
    max_malloc = max(max_malloc, cur_malloc);
    tot_malloc += size - oldsize;

    ++cnt_realloc;

    if (dbg_too_much != 0 && max_malloc > dbg_too_much)
	md_too_much();

    if (memtrace & M_FREE)
	mh_disp( "r", mh );

    size = size + sizeof(mh_t) + sizeof(md_tag);
    mh = realloc(mh, size);

    mh->size = size - sizeof(mh_t) - sizeof(md_tag);
    mh->indx = ++cnt_realloc;
    mh->tag  = md_tag;

    if (memtrace & M_MALLOC)
	mh_disp( "r", mh );

    mt = (mt_t *)((char *)mh+mh->size);
    mt->tag = md_tag;

    ptr = (void *) (mh+1);

    return ptr;
}
