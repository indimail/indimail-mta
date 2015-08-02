/* $Id: textblock.c 5232 2004-12-28 13:45:52Z m-a $ */

/*****************************************************************************

NAME:
   textblock.c -- implementation of textblock linked lists.

******************************************************************************/

#include "common.h"

#include "textblock.h"
#include "xmalloc.h"

/* Global Variables */

static textblock_t *textblocks = NULL;

static size_t cur_mem, max_mem, tot_mem;

/* Function Definitions */

textdata_t *textblock_head(void)
{
    return textblocks->head;
}

void textblock_init(void)
{
    textblock_t *t = (textblock_t *) xcalloc(1, sizeof(*t));
    size_t mem = sizeof(*t)+sizeof(textdata_t);
    t->head = (textdata_t *) xcalloc(1, sizeof(textdata_t));
    t->tail = t->head;
    cur_mem += mem;
    tot_mem += mem;
    max_mem = max(max_mem, cur_mem);
    if (DEBUG_TEXT(2))
	fprintf(dbgout, "%s:%d  %p %p %3lu *ini* cur: %lu, max: %lu, tot: %lu\n", __FILE__,__LINE__,
		(void *)t, (void *)t->head,
		(unsigned long)mem, (unsigned long)cur_mem,
		(unsigned long)max_mem, (unsigned long)tot_mem);
    textblocks = t;
}

void textblock_add(const byte *text, size_t size)
{
    textblock_t *t = textblocks;
    size_t mem = size+sizeof(textdata_t);
    textdata_t *cur = t->tail;

    cur->size = size;
    if (size == 0)
	cur->data = NULL;
    else {
	cur->data = (byte *)xmalloc(size+D);
	memcpy((char *)cur->data, (const char *)text, size+D);
	Z(((char *)cur->data)[size]);	/* for easier debugging - removable */
    }
    cur_mem += mem;
    tot_mem += mem;
    max_mem = max(max_mem, cur_mem);
    if (DEBUG_TEXT(2))
	fprintf(dbgout, "%s:%d  %p %p %3lu *add* cur: %lu, max: %lu, tot: %lu\n", 
			       __FILE__,__LINE__,
			       (void *)cur, (void *)cur->data,
			       (unsigned long)cur->size,
			       (unsigned long)cur_mem,
			       (unsigned long)max_mem,
			       (unsigned long)tot_mem );
    cur = cur->next = (textdata_t *) xcalloc(1, sizeof(textdata_t));
    t->tail = cur;
}

void textblock_free(void)
{
    size_t mem;
    textdata_t *cur, *nxt;
    textblock_t *t = textblocks;

    for (cur = t->head; cur ; cur = nxt) {
	nxt = cur->next;
	mem = cur->size + sizeof(*cur);
	cur_mem -= mem;
	if (DEBUG_TEXT(2)) fprintf(dbgout, "%s:%d  %p %p %3lu *rel* cur: %lu, max: %lu, tot: %lu\n", 
				   __FILE__,__LINE__, (void *)cur, cur->data,
				   (unsigned long)cur->size,
				   (unsigned long)cur_mem,
				   (unsigned long)max_mem,
				   (unsigned long)tot_mem);
	xfree((void*)cur->data);
	xfree((void*)cur);
    }

    mem = sizeof(*t->head);
    cur_mem -= mem;

    if (DEBUG_TEXT(2)) fprintf(dbgout, "%s:%d  %p %p *rel* cur: %lu, max: %lu, tot: %lu\n", 
			       __FILE__,__LINE__,
			       (void *)t, (void *)t->head,
			       (unsigned long)cur_mem, (unsigned long)max_mem,
			       (unsigned long)tot_mem);
    xfree(t);
    cur_mem -= sizeof(t->head) + sizeof(t);
    if (DEBUG_TEXT(1)) fprintf(dbgout, "cur: %lu, max: %lu, tot: %lu\n", (unsigned long)cur_mem, (unsigned long)max_mem, (unsigned long)tot_mem );
}
