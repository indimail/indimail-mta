#ifndef	storeinfo_h
#define	storeinfo_h

/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"imaptoken.h"
#include	"numlib/numlib.h"


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
	int64_t nbytes;
	int nfiles;

	const char *acls;

	} ;

#endif
