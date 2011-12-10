/*
** Copyright 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#include	<ctype.h>

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"maildirkeywords.h"


const char *libmail_kwVerbotten=NULL;
int libmail_kwCaseSensitive=1;

void libmail_kwhInit(struct libmail_kwHashtable *h)
{
	size_t i;

	memset(h, 0, sizeof(*h));

	for (i=0; i<sizeof(h->heads)/sizeof(h->heads[0]); i++)
	{
		h->heads[i].u.userPtr=h;
		h->tails[i].u.userPtr=h;

		h->heads[i].next=&h->tails[i];
		h->tails[i].prev=&h->heads[i];
	}
}

/* The hash table SHOULD be empty now */

int libmail_kwhCheck(struct libmail_kwHashtable *h)
{
	size_t i;

	for (i=0; i<sizeof(h->heads)/sizeof(h->heads[0]); i++)
		if (h->heads[i].next->next)
		{
			errno=EIO;
			return -1; /* Should not happen */
		}

	return 0;
}

struct libmail_keywordEntry *libmail_kweFind(struct libmail_kwHashtable *ht,
				      const char *name, int createIfNew)
{
	size_t hashBucket=0;
	const char *p;
	char *fixed_p=NULL;

	struct libmail_keywordEntry *e, *eNew;

	if (libmail_kwVerbotten)
	{
		for (p=name; *p; p++)
			if (strchr( libmail_kwVerbotten, *p))
				break;

		if (*p) /* Verbotten char, fix it */
		{
			char *q;

			fixed_p=strdup(name);

			if (!fixed_p)
				return NULL;

			for (q=fixed_p; *q; q++)
				if (strchr(libmail_kwVerbotten, *q))
					*q='_';

			name=fixed_p;
		}
	}

	p=name;
	while (*p)
	{
		hashBucket=(hashBucket << 1) ^ (hashBucket & 0x8000 ? 0x1301:0)

			^ (libmail_kwCaseSensitive ?
			   (unsigned char)*p:tolower((unsigned char)*p));
		++p;
	}
	hashBucket=hashBucket & 0xFFFF;
	hashBucket %= sizeof(ht->heads)/sizeof(ht->heads[0]);

	for (e= ht->heads[hashBucket].next; e->next; e=e->next)
	{
		const char *kn=keywordName(e);
		int n=libmail_kwCaseSensitive ? strcmp(kn, name) :
			strcasecmp(kn, name);

		if (n == 0)
		{
			if (fixed_p)
				free(fixed_p);
			return e;
		}

		if (n > 0)
			break;
	}

	if (!createIfNew)
	{
		if (fixed_p)
			free(fixed_p);
		return NULL;
	}

	if ((eNew=malloc(sizeof(*e)+1+strlen(name))) == NULL)
	{
		if (fixed_p)
			free(fixed_p);
		return NULL;
	}

	memset(eNew, 0, sizeof(*eNew));
	strcpy(keywordName(eNew), name);
	eNew->next=e;
	eNew->prev=e->prev;
	eNew->next->prev=eNew;
	eNew->prev->next=eNew;
	eNew->firstMsg=NULL;
	eNew->lastMsg=NULL;
	ht->keywordAddedRemoved=1;

	if (fixed_p)
		free(fixed_p);
	return eNew;
}

struct libmail_kwMessage *libmail_kwmCreate()
{
	struct libmail_kwMessage *kw=malloc(sizeof(struct libmail_kwMessage));

	if (kw == NULL)
		return NULL;

	memset(kw, 0, sizeof(*kw));

	return kw;
}

void libmail_kwmDestroy(struct libmail_kwMessage *kw)
{
	while (kw->firstEntry)
		libmail_kwmClearEntry(kw->firstEntry);

	free(kw);
}

int libmail_kwmSetName(struct libmail_kwHashtable *h,
		   struct libmail_kwMessage *km, const char *n)
{
	struct libmail_keywordEntry *ke=libmail_kweFind(h, n, 1);

	if (!ke)
		return -1;

	return libmail_kwmSet(km, ke);
}

int libmail_kwmSet(struct libmail_kwMessage *km, struct libmail_keywordEntry *ke)
{
	struct libmail_kwMessageEntry *keLast, *kePtr;

	const char *name=keywordName(ke);

	for (keLast=km->firstEntry; keLast; keLast=keLast->next)
	{
		int rc=strcmp(keywordName(keLast->libmail_keywordEntryPtr), name);

		if (rc == 0)
			return 1; /* Keyword already set */

		if (rc > 0)
			break;
	}

	kePtr=malloc(sizeof(*kePtr));

	if (!kePtr)
		return -1;

	if (keLast)
	{
		kePtr->next=keLast;
		kePtr->prev=keLast->prev;

		keLast->prev=kePtr;

		if (kePtr->prev)
			kePtr->prev->next=kePtr;
		else
			km->firstEntry=kePtr;
	}
	else
	{
		kePtr->next=NULL;

		if ((kePtr->prev=km->lastEntry) != NULL)
			kePtr->prev->next=kePtr;
		else
			km->firstEntry=kePtr;
		km->lastEntry=kePtr;
	}
	kePtr->libmail_kwMessagePtr=km;

	kePtr->keywordNext=NULL;
	if ((kePtr->keywordPrev=ke->lastMsg) != NULL)
		kePtr->keywordPrev->keywordNext=kePtr;
	else
		ke->firstMsg=kePtr;

	ke->lastMsg=kePtr;
	kePtr->libmail_keywordEntryPtr=ke;
	return 0;
}

/*
** Because keywords are linked in a sorted order, comparing for equality
** is trivial.
*/

int libmail_kwmCmp(struct libmail_kwMessage *km1,
	       struct libmail_kwMessage *km2)
{
	struct libmail_kwMessageEntry *e1, *e2;

	for (e1=km1->firstEntry, e2=km2->firstEntry; e1 && e2;
	     e1=e1->next, e2=e2->next)
		if (strcmp(keywordName(e1->libmail_keywordEntryPtr),
			   keywordName(e2->libmail_keywordEntryPtr)))
			break;

	return e1 || e2 ? -1:0;
}


int libmail_kwmClear(struct libmail_kwMessage *km,
		     struct libmail_keywordEntry *ke)
{
	return libmail_kwmClearName(km, keywordName(ke));
}

int libmail_kwmClearName(struct libmail_kwMessage *km, const char *n)
{
	struct libmail_kwMessageEntry *kEntry;

	for (kEntry=km->firstEntry; kEntry; kEntry=kEntry->next)
		if (strcmp(keywordName(kEntry->libmail_keywordEntryPtr), n) == 0)
		{
			return libmail_kwmClearEntry(kEntry);
		}

	return 1;
}

int libmail_kwmClearEntry(struct libmail_kwMessageEntry *e)
{
	struct libmail_keywordEntry *kw=e->libmail_keywordEntryPtr;

	if (e->next)
		e->next->prev=e->prev;
	else e->libmail_kwMessagePtr->lastEntry=e->prev;

	if (e->prev)
		e->prev->next=e->next;
	else e->libmail_kwMessagePtr->firstEntry=e->next;

	if (e->keywordNext)
		e->keywordNext->keywordPrev=e->keywordPrev;
	else
		kw->lastMsg=e->keywordPrev;

	if (e->keywordPrev)
		e->keywordPrev->keywordNext=e->keywordNext;
	else
		kw->firstMsg=e->keywordNext;

	libmail_kweClear(kw);
	free(e);
	return 0;
}


void libmail_kweClear(struct libmail_keywordEntry *kw)
{
	if (kw->firstMsg == NULL && kw->lastMsg == NULL)
	{
		struct libmail_keywordEntry *k;

		for (k=kw; k->next; k=k->next)
			;

		((struct libmail_kwHashtable *)k->u.userPtr)
			->keywordAddedRemoved=1;

		kw->prev->next=kw->next;
		kw->next->prev=kw->prev; /* There are always dummy head/tail */

		free(kw);
	}
}

int libmail_kwEnumerate(struct libmail_kwHashtable *h,
		     int (*callback_func)(struct libmail_keywordEntry *,
					  void *),
		     void *callback_arg)
{
	size_t i;

	for (i=0; i<sizeof(h->heads)/sizeof(h->heads[0]); i++)
	{
		struct libmail_keywordEntry *ke=h->heads[i].next;

		while (ke->next)
		{
			struct libmail_keywordEntry *ke2=ke;
			int rc;

			ke=ke->next;

			if ((rc=(*callback_func)(ke2, callback_arg)) != 0)
				return rc;
		}
	}
	return 0;
}
