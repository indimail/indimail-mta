#ifndef	thread_h
#define	thread_h

#include	"searchinfo.h"
/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

static const char thread_h_rcsid[]="$Id: thread.h,v 1.3 2000/09/29 04:38:47 mrsam Exp $";

struct threadinfo;

extern int thread_orderedsubj(struct threadinfo *, struct threadinfo *);

struct unicode_info;

void dothreadorderedsubj(struct searchinfo *, struct searchinfo *,
			 const struct unicode_info *, int);
void dothreadreferences(struct searchinfo *, struct searchinfo *,
			const struct unicode_info *, int);

/* While we're at it, some support for SORT */

struct temp_sort_stack {	/* Temporary stack list of SORT criteria */
	struct temp_sort_stack *next;
	search_type type;
	} ;

void free_temp_sort_stack(struct temp_sort_stack *);
void dosortmsgs(struct searchinfo *, struct searchinfo *,
		const struct unicode_info *, int);

#endif
