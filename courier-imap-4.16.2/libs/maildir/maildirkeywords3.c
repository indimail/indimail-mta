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

#include	"maildirkeywords.h"

void libmail_kwgInit(struct libmail_kwGeneric *g)
{
	memset(g, 0, sizeof(*g));
	libmail_kwhInit(&g->kwHashTable);
}

int libmail_kwgDestroy(struct libmail_kwGeneric *g)
{
	struct libmail_kwGenericEntry *p;
	size_t n;

	for (n=0; n<sizeof(g->messageHashTable)/sizeof(g->messageHashTable[0]);
	     n++)
		while ((p=g->messageHashTable[n]) != NULL)
		{
			g->messageHashTable[n]=p->next;
			if (p->filename)
				free(p->filename);
			if (p->keywords)
				libmail_kwmDestroy(p->keywords);
			free(p);
		}

	if (g->messages)
		free(g->messages);
	g->messages=NULL;
	g->nMessages=0;
	g->messagesValid=0;
	return libmail_kwhCheck(&g->kwHashTable);
}

static unsigned filenameHash(const char *fn)
{
	unsigned long hashBucket=0;

	while (*fn && *fn != MDIRSEP[0])
	{
		hashBucket=(hashBucket << 1) ^ (hashBucket & 0x8000 ? 0x1301:0)
			^ (unsigned char)*fn++;
	}

	return hashBucket & 0xFFFF;
}

static int filenameCmp(const char *a, const char *b)
{
	char ca, cb;

	while ( (ca= *a == MDIRSEP[0] ? 0:*a)
		== (cb= *b == MDIRSEP[0] ? 0:*b) && ca)
	{
		++a;
		++b;
	}

	return (ca < cb ? -1:ca > cb ? 1:0);
}

static int srch(struct libmail_kwGeneric *g, const char *filename,
		struct libmail_kwGenericEntry ***ret)
{
	struct libmail_kwGenericEntry **p;

	for (p= &g->messageHashTable[filenameHash(filename) %
				     (sizeof(g->messageHashTable)
				      /sizeof(g->messageHashTable[0]))];
	     *p; p= &(*p)->next)
	{
		int cmp=filenameCmp( (*p)->filename, filename);

		if (cmp == 0)
		{
			*ret=p;
			return 0;
		}

		if (cmp > 0)
			break;
	}

	*ret=p;
	return -1;
}

struct libmail_kwGenericEntry *
libmail_kwgFindByName(struct libmail_kwGeneric *g, const char *filename)
{
	const char *p=strrchr(filename, '/');
	struct libmail_kwGenericEntry **ret;

	if (p)
		filename=p+1;

	if (srch(g, filename, &ret) == 0)
		return *ret;

	return NULL;
}

struct libmail_kwGenericEntry *
libmail_kwgFindByIndex(struct libmail_kwGeneric *g, size_t n)
{
	if (!g->messagesValid || n >= g->nMessages)
		return NULL;

	return g->messages[n];
}

/***************************************************************************/

static struct libmail_kwMessage **g_findMessageByFilename(const char
							  *filename,
							  int autocreate,
							  size_t *indexNum,
							  void *voidarg);
static size_t g_getMessageCount(void *voidarg);

static struct libmail_kwMessage **g_findMessageByIndex(size_t indexNum,
						       int autocreate,
						       void *voidarg);

static const char *g_getMessageFilename(size_t n, void *voidarg);

static struct libmail_kwHashtable *g_getKeywordHashtable(void *voidarg);

static void g_updateKeywords(size_t n, struct libmail_kwMessage *kw,
			     void *voidarg);
static int g_validIndex(struct libmail_kwGeneric *g);

int libmail_kwgReadMaildir(struct libmail_kwGeneric *g,
			   const char *maildir)
{
	struct maildir_kwReadInfo ri;

	memset(&ri, 0, sizeof(ri));

	ri.findMessageByFilename= &g_findMessageByFilename;
	ri.getMessageCount= &g_getMessageCount;
	ri.findMessageByIndex= &g_findMessageByIndex;
	ri.getMessageFilename= &g_getMessageFilename;
	ri.getKeywordHashtable= &g_getKeywordHashtable;
	ri.updateKeywords=&g_updateKeywords;
	ri.voidarg=g;

	if (maildir_kwRead(maildir, &ri) < 0)
	{
		libmail_kwgDestroy(g);
		return -1;
	}

	if (ri.tryagain)
	{
		libmail_kwgDestroy(g); /* Free memory */
		return 1;
	}

	if (g_validIndex(g) < 0)
	{
		libmail_kwgDestroy(g); /* Free memory */
		return -1;
	}
	return 0;
}

static struct libmail_kwGenericEntry *g_create(const char *filename)
{
	struct libmail_kwGenericEntry *e=
		malloc(sizeof (struct libmail_kwGenericEntry));
	char *p;

	if (e && (e->filename=strdup(filename)) == NULL)
	{
		free(e);
		e=NULL;
	}

	if (!e)
		return NULL;

	e->keywords=NULL;

	if ((p=strrchr(e->filename, MDIRSEP[0])) != 0)
		*p=0;
	return e;
}

static struct libmail_kwMessage **g_findMessageByFilename(const char
							  *filename,
							  int autocreate,
							  size_t *indexNum,
							  void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;
	const char *p=strrchr(filename, '/');
	struct libmail_kwGenericEntry **ret;

	if (p)
		filename=p+1;

	if (srch(g, filename, &ret))
	{
		struct libmail_kwGenericEntry *n;

		n=g_create(filename);
		if (!n)
			return NULL;

		g->messagesValid=0;
		n->messageNum= g->nMessages++;

		n->next= *ret;
		*ret=n;
	}

	if (indexNum)
		*indexNum=(*ret)->messageNum;

	if ( (*ret)->keywords == NULL && autocreate)
		(*ret)->keywords=libmail_kwmCreate();

	return & (*ret)->keywords;
}

static size_t g_getMessageCount(void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;

	return g->nMessages;
}

static int g_validIndex(struct libmail_kwGeneric *g)
{
	struct libmail_kwGenericEntry *p;
	size_t n;

	if (g->messagesValid)
		return 0;

	if (g->messages)
		free(g->messages);
	g->messages=NULL;

	if (g->nMessages == 0)
	{
		g->messagesValid=1;
		return 0;
	}

	if ((g->messages=malloc(g->nMessages * sizeof(*g->messages)))
	    == NULL)
		return -1;

	for (n=0; n<sizeof(g->messageHashTable)/sizeof(g->messageHashTable[0]);
	     n++)
		for (p=g->messageHashTable[n]; p; p=p->next)
			g->messages[p->messageNum]=p;

	g->messagesValid=1;

	return 0;
}

static struct libmail_kwMessage **g_findMessageByIndex(size_t indexNum,
						       int autocreate,
						       void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;

	if (g_validIndex(g))
		return NULL;

	if (indexNum >= g->nMessages)
		return NULL;

	if ( g->messages[indexNum]->keywords == NULL && autocreate)
		g->messages[indexNum]->keywords=libmail_kwmCreate();

	return &g->messages[indexNum]->keywords;
}

static const char *g_getMessageFilename(size_t indexNum, void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;

	if (g_validIndex(g))
		return NULL;

	if (indexNum >= g->nMessages)
		return NULL;

	return g->messages[indexNum]->filename;
}

static struct libmail_kwHashtable *g_getKeywordHashtable(void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;

	return &g->kwHashTable;
}

static void g_updateKeywords(size_t indexNum, struct libmail_kwMessage *kw,
			     void *voidarg)
{
	struct libmail_kwGeneric *g=(struct libmail_kwGeneric *)voidarg;

	if (g_validIndex(g))
		return;

	if (indexNum >= g->nMessages)
		return;

	if ( g->messages[indexNum]->keywords)
		libmail_kwmDestroy(g->messages[indexNum]->keywords);

	g->messages[indexNum]->keywords=kw;
}
