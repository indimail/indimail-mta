#ifndef	storeinfo_h
#define	storeinfo_h

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"imaptoken.h"

static const char storeinfo_h_rcsid[]="$Id: storeinfo.h,v 1.6 2004/01/11 02:47:33 mrsam Exp $";

struct storeinfo {
	int plusminus;
	int silent;
	struct imapflags flags;
	struct libmail_kwMessage *keywords;
	} ;

int storeinfo_init(struct storeinfo *);
int do_store(unsigned long, int, void *);

int do_copy_message(unsigned long, int, void *);
int do_copy_quota_calc(unsigned long, int, void *);

struct uidplus_info;

struct do_copy_info {
	const char *mailbox;
	const char *acls;

	struct uidplus_info *uidplus_list;
	struct uidplus_info **uidplus_tail;
};

/*
** maildir quota calculation for copying messages.
*/

struct copyquotainfo {
	char *destmailbox;
	long nbytes;
	int nfiles;

	const char *acls;

	} ;

#endif
