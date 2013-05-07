/* $Id: tunelist.c 6714 2008-04-20 15:27:06Z relson $ */

/*****************************************************************************

NAME:
   tunelist.c -- definitions and prototypes of list structures for bogotune.
		 includes msglist_t, filelist_t, and tunelist_t.

******************************************************************************/

#include "common.h"

#include "tunelist.h"

#include "xmalloc.h"
#include "xstrdup.h"

flhead_t *filelist_new(const char *name)
{
    flhead_t *list = xcalloc(1, sizeof(flhead_t));
    list->name = xstrdup(name);
    return list;
}

void filelist_add(flhead_t *list, const char *name)
{
    flitem_t *item = xcalloc(1, sizeof(flitem_t));
    item->name = xstrdup(name);
    if (list->head == NULL)
	list->head = item;
    if (list->tail != NULL)
	list->tail->next = item;
    list->tail = item;
    list->count += 1;
    if (verbose > 1000)
	printf("%s:  h %p (%u)  t %p  w %p\n",
	       list->name,
	       (void *)list->head, list->count, (void *)list->tail, name);
    return;
}

void filelist_free(flhead_t *list)
{
    flitem_t *item, *next;

    for (item = list->head; item != NULL; item = next) {
	next = item->next;
	xfree(item->name);
	xfree(item);
    }
    xfree(list->name);
    xfree(list);

    return;
}

mlhead_t *msglist_new(const char *label)
{
    mlhead_t *v = xcalloc(1, sizeof(mlhead_t));
    v->name = xstrdup(label);
    return v;
}

void msglist_add(mlhead_t *list, wordhash_t *wh)
{
    mlitem_t *item = xcalloc(1, sizeof(mlitem_t));
    if (list->head == NULL)
	list->head = item;
    if (list->tail != NULL)
	list->tail->next = item;
    list->tail = item;
    list->count += 1;
    item->wh = wh;
    if (verbose > 1000)
	printf("%s:  h %p (%u)  t %p  w %p %4lu\n", 
	       list->name, (void *)list->head, list->count,
	       (void *)list->tail, (void *)wh, (unsigned long)wh->count);
    return;
}

void msglist_print(mlhead_t *list)
{
    int count = 0;
    mlitem_t *item;

    if (verbose <= 2)
	return;

    printf("%s:\n", list->name);

    if (list->count == 0)
	printf("  (empty)\n");

    for (item = list->head; item != NULL; item = item->next) {
	wordhash_t *wh = item->wh;
	printf("  %4d  %p  %4lu\n", count++, (void *)item,
		(unsigned long)wh->count);
    }

    return;
}

void msglist_free(mlhead_t *list)
{
    mlitem_t *item, *next;

    for (item = list->head; item != NULL; item = next) {
	next = item->next;
	wordhash_free(item->wh);
	xfree(item);
    }
    xfree(list->name);
    xfree(list);

    return;
}

tunelist_t *tunelist_new(const char *label)
{
    tunelist_t *list = xcalloc( 1, sizeof(tunelist_t));

    list->name = label;
    list->msgs   = msglist_new("msgs");
    list->u.r.r0 = msglist_new("r0");
    list->u.r.r1 = msglist_new("r1");
    list->u.r.r2 = msglist_new("r2");

    return list;
}

void tunelist_print(tunelist_t *list)
{
    printf("%s (%u):\n", list->name, list->count);

    msglist_print(list->u.r.r0);	/* run sets */
    msglist_print(list->u.r.r1);
    msglist_print(list->u.r.r2);

    return;
}

void tunelist_free(tunelist_t *list)
{
    if (list == NULL)
	return;

    wordhash_free(list->train);		/* training */
    msglist_free(list->msgs);
    msglist_free(list->u.r.r0);		/* run sets */
    msglist_free(list->u.r.r1);
    msglist_free(list->u.r.r2);
    xfree(list);

    return;
}

/* Count all messages */

uint count_messages(tunelist_t *list)
{
    uint i;
    uint count = 0;

    for (i=0; i < COUNTOF(list->u.sets); i += 1) {
	count += list->u.sets[i]->count;
    }

    return count;
}
