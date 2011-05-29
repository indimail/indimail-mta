#ifndef	fetchinfo_h
#define	fetchinfo_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

static const char fetchinfo_h_rcsid[]="$Id: fetchinfo.h,v 1.3 2000/10/07 22:01:51 mrsam Exp $";

struct fetchinfo {
	struct fetchinfo *next;	/* Siblings */
	char *name;		/* Name */
	char *bodysection;	/* BODY section */
	int ispartial;
	unsigned long partialstart;
	unsigned long partialend;
	struct fetchinfo *bodysublist;	/* HEADER sublist */
	} ;

struct fetchinfo *fetchinfo_alloc(int);

void fetchinfo_free(struct fetchinfo *);

void fetch_free_cache();

void save_cached_offsets(off_t, off_t, off_t);

int get_cached_offsets(off_t, off_t *, off_t *);

#endif
