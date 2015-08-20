#ifndef	thread_h
#define	thread_h

#include	"searchinfo.h"
/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/


struct threadinfo;

extern int thread_orderedsubj(struct threadinfo *, struct threadinfo *);

struct unicode_info;

void dothreadorderedsubj(struct searchinfo *, struct searchinfo *,
			 const char *, int);
void dothreadreferences(struct searchinfo *, struct searchinfo *,
			const char *, int);

/* While we're at it, some support for SORT */

struct temp_sort_stack {	/* Temporary stack list of SORT criteria */
	struct temp_sort_stack *next;
	search_type type;
	} ;

void free_temp_sort_stack(struct temp_sort_stack *);
void dosortmsgs(struct searchinfo *, struct searchinfo *,
		const char *, int);

#endif
