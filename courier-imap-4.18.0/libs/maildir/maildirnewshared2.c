/*
** Copyright 2004-2007 Double Precision, Inc.
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
#include	<fcntl.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_UTIME_H
#include	<utime.h>
#endif
#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"config.h"
#include	"maildir/maildirnewshared.h"


static struct maildir_shindex_cache *shared_cache=NULL;

static void shared_cache_free(struct maildir_shindex_cache *c)
{
	while (c)
	{
		struct maildir_shindex_cache *p=c;
		size_t i;

		c=c->next;

		free(p->hierarchy);
		if (p->records)
		{
			for (i=0; i<p->nrecords; i++)
				free(p->records[i].name);
			free(p->records);
		}
		maildir_newshared_close(&p->indexfile);
		free(p);
	}
}

static struct maildir_shindex_cache *do_shared_cache_read(const char *indexfile,
						       const char *subhier);

struct maildir_shindex_cache *
maildir_shared_cache_read(struct maildir_shindex_cache *parent,
			  const char *indexfile,
			  const char *subhierarchy)
{
	struct maildir_shindex_cache *p;

	if (parent && parent->next && subhierarchy &&
	    strcmp(parent->next->hierarchy, subhierarchy) == 0)
	{
		return parent->next; /* That was easy */
	}

	if (!parent && shared_cache)
	{
		return shared_cache;
	}

	if (!indexfile)
	{
		indexfile=maildir_shared_index_file();
		if (!indexfile)
			return NULL;
		subhierarchy="";
	}


	if (!subhierarchy)
		return NULL;
	/* Should not happen, bad usage. subhierarchy allowed to be NULL only
	** when indexfile is also NULL */

	p=do_shared_cache_read(indexfile, subhierarchy);

	if (!p)
		return NULL;

	if (!parent)
	{
		shared_cache_free(shared_cache);
		shared_cache=p;
	}
	else
	{
		shared_cache_free(parent->next);
		parent->next=p;
	}
	return p;
}

struct maildir_shindex_temp_record {
	struct maildir_shindex_temp_record *next;
	struct maildir_shindex_record rec;
};

static int shared_cache_read_cb(struct maildir_newshared_enum_cb *ptr)
{
	struct maildir_shindex_temp_record *r;
	struct maildir_shindex_temp_record **list=
		(struct maildir_shindex_temp_record **)ptr->cb_arg;

	if ((r=malloc(sizeof(struct maildir_shindex_temp_record))) == NULL ||
	    (r->rec.name=strdup(ptr->name)) == NULL)
	{
		if (r)
			free(r);
		perror("malloc");
		return -1;
	}

	r->rec.offset=ptr->startingpos;
	r->next= *list;
	*list=r;
	return 0;
}

static struct maildir_shindex_cache *do_shared_cache_read(const char *indexfile,
						       const char *subhier)
{
	struct maildir_shindex_temp_record *rec=NULL;
	size_t n;
	struct maildir_shindex_cache *c;
	int eof;
	int rc;

	if ((c=malloc(sizeof(struct maildir_shindex_cache))) == NULL ||
	    (c->hierarchy=strdup(subhier)) == NULL)
	{
		if (c)
			free(c);
		perror("malloc");
		return NULL;
	}

	if (maildir_newshared_open(indexfile, &c->indexfile) < 0)
	{
		free(c->hierarchy);
		free(c);
		return NULL;
	}

	n=0;

	while ((rc=maildir_newshared_next(&c->indexfile, &eof,
					  shared_cache_read_cb, &rec)) == 0)
	{
		if (eof)
			break;
		++n;
	}

	if (rc)
	{
		free(c->hierarchy);
		free(c);
		while (rec)
		{
			struct maildir_shindex_temp_record *r=rec;

			rec=rec->next;

			free(r->rec.name);
			free(r);
		}
		return NULL;
	}

	/* Now, convert from list to array */

	c->nrecords=n;
	c->records=NULL;
	c->next=NULL;

	if (n)
	{
		if ((c->records=malloc(sizeof(*c->records)*n)) == NULL)
		{

			free(c->hierarchy);
			free(c);
			while (rec)
			{
				struct maildir_shindex_temp_record *r=rec;

				rec=rec->next;

				free(r->rec.name);
				free(r);
			}
			return NULL;
		}

		n=0;
		while (rec)
		{
			struct maildir_shindex_temp_record *r=rec;

			rec=rec->next;
			c->records[n]= r->rec;

			free(r);
			++n;
		}
	}

	return c;
}






