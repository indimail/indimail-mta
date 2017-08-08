/*
** Copyright 2000-2009 Double Precision, Inc.
** See COPYING for distribution information.
*/


#include	"config.h"
#include	"imapd.h"
#include	"thread.h"
#include	"searchinfo.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	"imapwrite.h"
#include	"imaptoken.h"
#include	"imapscanclient.h"
#include	"rfc822/rfc822.h"
#include	"rfc822/rfc2047.h"
#include	"rfc822/imaprefs.h"
#include	"unicode/courier-unicode.h"

static void thread_os_callback(struct searchinfo *, struct searchinfo *, int,
	unsigned long, void *);
static void thread_ref_callback(struct searchinfo *, struct searchinfo *, int,
	unsigned long, void *);

extern struct imapscaninfo current_maildir_info;


struct os_threadinfo {
	struct os_threadinfo *next;
	char *subj;
	time_t sent_date;
	unsigned long n;
	} ;

struct os_threadinfo_list {
	struct os_threadinfo_list *next;
	size_t thread_start;
} ;

struct os_struct {
	struct os_threadinfo *list;
	unsigned nmsgs;
	struct os_threadinfo **msgs;
	} ;

static void os_init(struct os_struct *os)
{
	memset(os, 0, sizeof(*os));
}

static void os_add(struct os_struct *os, unsigned long n, const char *s,
		   time_t sent_date)
{
struct os_threadinfo *osi=(struct os_threadinfo *)
	malloc(sizeof(struct os_threadinfo));

	if (!osi)	write_error_exit(0);
	osi->subj=strdup(s);
			/* This decodes the MIME encoding */
	if (!osi->subj)	write_error_exit(0);
	osi->sent_date=sent_date;
	osi->n=n;
	osi->next=os->list;
	os->list=osi;
	++os->nmsgs;
}

static void os_free(struct os_struct *os)
{
struct os_threadinfo *p;

	while ((p=os->list) != 0)
	{
		os->list=p->next;
		free(p->subj);
		free(p);
	}
	if (os->msgs) free(os->msgs);
}

static int cmpsubjs(const void *a, const void *b)
{
	const struct os_threadinfo *ap=*(const struct os_threadinfo **)a;
	const struct os_threadinfo *bp=*(const struct os_threadinfo **)b;
	int rc=strcmp( ap->subj, bp->subj);

	if (rc)	return (rc);

	return (ap->sent_date < bp->sent_date ? -1:
		ap->sent_date > bp->sent_date ? 1:0);
}

/* Print the meat of the THREAD ORDEREDSUBJECT response */

static void printos(struct os_threadinfo **array, size_t cnt)
{
	size_t	i;
	struct os_threadinfo_list *thread_list=NULL, *threadptr, **tptr;

	/*
	** thread_list - indexes to start of each thread, sort indexes by
	** sent_date
	*/

	for (i=0; i<cnt; i++)
	{
		/* Find start of next thread */

		if (i > 0 && strcmp(array[i-1]->subj, array[i]->subj) == 0)
			continue;

		threadptr=malloc(sizeof(struct os_threadinfo_list));
		if (!threadptr)
			write_error_exit(0);
		threadptr->thread_start=i;

		/* Insert into the list, sorted by sent date */

		for (tptr= &thread_list; *tptr; tptr=&(*tptr)->next)
			if ( array[(*tptr)->thread_start]->sent_date
			     > array[i]->sent_date)
				break;

		threadptr->next= *tptr;
		*tptr=threadptr;
	}

	while ( (threadptr=thread_list) != NULL)
	{
		size_t	i, j;
		const char *p;

		thread_list=threadptr->next;

		i=threadptr->thread_start;
		free(threadptr);

		for (j=i+1; j<cnt; j++)
		{
			if (strcmp(array[i]->subj, array[j]->subj))
				break;
		}

		p="(";
		while (i < j)
		{
			writes(p);
			p=" ";
			writen(array[i]->n);
			++i;
		}
		writes(")");
	}
}

void dothreadorderedsubj(struct searchinfo *si, struct searchinfo *sihead,
			 const char *charset, int isuid)
{
struct	os_struct	os;

	os_init(&os);
	search_internal(si, sihead, charset, isuid, thread_os_callback, &os);

	if (os.nmsgs > 0)	/* Found some messages */
	{
	size_t	i;
	struct os_threadinfo *o;

		/* Convert it to an array */

		os.msgs= (struct os_threadinfo **)
			malloc(os.nmsgs * sizeof(struct os_threadinfo *));
		if (!os.msgs)	write_error_exit(0);
		for (o=os.list, i=0; o; o=o->next, i++)
			os.msgs[i]=o;

		/* Sort the array */

		qsort(os.msgs, os.nmsgs, sizeof(*os.msgs), cmpsubjs);

		/* Print the array */

		printos(os.msgs, os.nmsgs);
	}
	os_free(&os);
}

/*
This callback function is called once search finds a qualifying message.
We save its message number and subject in a link list.
*/

static void thread_os_callback(struct searchinfo *si,
			       struct searchinfo *sihead,
			       int isuid, unsigned long i,
			       void *voidarg)
{
	if (sihead->type == search_orderedsubj)
	{
		/* SHOULD BE ALWAYS TRUE */
		time_t t=0;

		if (sihead->bs)
			rfc822_parsedate_chk(sihead->bs, &t);

		os_add( (struct os_struct *)voidarg,
			isuid ? current_maildir_info.msgs[i].uid:i+1,
			sihead->as ? sihead->as:"",
			t);
	}
}

static void printthread(struct imap_refmsg *, int);

void dothreadreferences(struct searchinfo *si, struct searchinfo *sihead,
			const char *charset,
			int isuid)
{
	struct imap_refmsgtable *reftable;
	struct imap_refmsg *root;

	if (!(reftable=rfc822_threadalloc()))
	{
		write_error_exit(0);
		return;
	}

	search_internal(si, sihead, charset, 0,
			thread_ref_callback, reftable);

	root=rfc822_thread(reftable);
	printthread(root, isuid);
	rfc822_threadfree(reftable);
}

static void thread_ref_callback(struct searchinfo *si,
			       struct searchinfo *sihead,
			       int isuid, unsigned long i,
			       void *voidarg)
{
	if (sihead->type == search_references1 && sihead->a &&
	    sihead->a->type == search_references2 && sihead->a->a &&
	    sihead->a->a->type == search_references3 && sihead->a->a->a &&
	    sihead->a->a->a->type == search_references4)
	{
		const char *ref, *inreplyto, *subject, *date, *msgid;

		ref=sihead->as;
		inreplyto=sihead->bs;
		date=sihead->a->as;
		subject=sihead->a->a->as;
		msgid=sihead->a->a->a->as;

#if 0
		fprintf(stderr, "REFERENCES: ref=%s, inreplyto=%s, subject=%s, date=%s, msgid=%s\n",
			ref ? ref:"",
			inreplyto ? inreplyto:"",
			subject ? subject:"",
			date ? date:"",
			msgid ? msgid:"");
#endif

		if (!rfc822_threadmsg( (struct imap_refmsgtable *)voidarg,
				       msgid, ref && *ref ? ref:inreplyto,
				       subject, date, 0, i))
			write_error_exit(0);
	}
}

static void printthread(struct imap_refmsg *msg, int isuid)
{
	const char *pfix="";

	while (msg)
	{
		if (!msg->isdummy)
		{
			writes(pfix);
			writen(isuid ?
			       current_maildir_info.msgs[msg->seqnum].uid:
			       msg->seqnum+1);
			pfix=" ";
		}

		if (msg->firstchild && (msg->firstchild->nextsib
					|| msg->firstchild->isdummy
					|| msg->parent == NULL))
		{
			writes(pfix);
			for (msg=msg->firstchild; msg; msg=msg->nextsib)
			{
				struct imap_refmsg *msg2;

				msg2=msg;

				if (msg2->isdummy)
					msg2=msg2->firstchild;

				for (; msg2; msg2=msg2->firstchild)
				{
					if (!msg2->isdummy ||
					    msg2->nextsib)
						break;
				}

				if (msg2)
				{
					writes("(");
					printthread(msg, isuid);
					writes(")");
				}
			}
			break;
		}
		msg=msg->firstchild;
	}
}

void free_temp_sort_stack(struct temp_sort_stack *t)
{
	while (t)
	{
	struct temp_sort_stack *u=t->next;

		free(t);
		t=u;
	}
}

/* ---------------------------------- SORT ---------------------------- */

/* sortmsginfo holds the sorting information for a message. */

struct sortmsginfo {
	struct sortmsginfo *next;	/* next msg */
	unsigned long n;		/* msg number/uid */
	char **sortfields;		/* array of sorting fields */
	char *sortorder;		/* [x]=0 - normal, [x]=1 - reversed */
	size_t nfields;
	} ;

struct sortmsgs {
	struct sortmsginfo *list;	/* The actual list */
	struct sortmsginfo **array;	/* In array form */
	size_t	nmsgs;			/* Array size/count of msgs */
	size_t	nfields;		/* From the SORT() arg */
	} ;

static void free_sortmsgs(struct sortmsgs *p)
{
struct sortmsginfo *q;

	if (p->array)	free(p->array);
	while ((q=p->list) != 0)
	{
	size_t	i;

		p->list=q->next;
		for (i=0; i<p->nfields; i++)
			if (q->sortfields[i])
				free(q->sortfields[i]);
		if (q->sortfields)	free(q->sortfields);
		if (q->sortorder)	free(q->sortorder);
		free(q);
	}
}

static void sort_callback(struct searchinfo *, struct searchinfo *, int,
	unsigned long, void *);

static int cmpsort(const void *a, const void *b)
{
const struct sortmsginfo *ap=*(const struct sortmsginfo **)a;
const struct sortmsginfo *bp=*(const struct sortmsginfo **)b;
size_t	i;

	for (i=0; i<ap->nfields; i++)
	{
	int	n=strcmp(ap->sortfields[i], bp->sortfields[i]);

		if (n < 0)
			return (ap->sortorder[i] ? 1:-1);
		if (n > 0)
			return (ap->sortorder[i] ? -1:1);
	}
	return (0);
}

void dosortmsgs(struct searchinfo *si, struct searchinfo *sihead,
		const char *charset, int isuid)
{
struct	sortmsgs sm;
struct	searchinfo *p;

	memset(&sm, 0, sizeof(sm));
	for (p=sihead; p; p=p->a)
		switch (p->type)	{
		case search_orderedsubj:
		case search_arrival:
		case search_cc:
		case search_date:
		case search_from:
		case search_size:
		case search_to:
			++sm.nfields;
			break;
		default:
			break;
		}
	search_internal(si, sihead, charset, isuid, sort_callback, &sm);
	if (sm.nmsgs > 0)
	{
	size_t	i;
	struct sortmsginfo *o;

		/* Convert it to an array */

		sm.array= (struct sortmsginfo **)
			malloc(sm.nmsgs * sizeof(struct sortmsginfo *));
		if (!sm.array)	write_error_exit(0);
		for (o=sm.list, i=0; o; o=o->next, i++)
			sm.array[i]=o;

		/* Sort the array */

		qsort(sm.array, sm.nmsgs, sizeof(*sm.array), cmpsort);

		/* Print the array */

		for (i=0; i<sm.nmsgs; i++)
		{
			writes(" ");
			writen(sm.array[i]->n);
		}
	}
	free_sortmsgs(&sm);
}

static void sort_callback(struct searchinfo *si, struct searchinfo *sihead,
	int isuid, unsigned long n, void *voidarg)
{
struct sortmsgs *sm=(struct sortmsgs *)voidarg;
struct sortmsginfo *msg=(struct sortmsginfo *)
		malloc(sizeof(struct sortmsginfo));
struct searchinfo *ss;
int rev;
size_t	i;

	if (msg)	memset(msg, 0, sizeof(*msg));
	if (!msg || (sm->nfields && ((msg->sortfields=(char **)
				malloc(sizeof(char *)*sm->nfields)) == 0 ||
					(msg->sortorder=(char *)
				malloc(sm->nfields)) == 0)))
		write_error_exit(0);

	if (sm->nfields)
	{
		memset(msg->sortfields, 0, sizeof(char *)*sm->nfields);
		memset(msg->sortorder, 0, sm->nfields);
	}

	rev=0;
	i=0;

/* fprintf(stderr, "--\n"); */

	for (ss=sihead; ss; ss=ss->a)
	{
	char	*p;

		if (i >= sm->nfields)
			break;	/* Something's fucked up, better handle it
				** gracefully, instead of dumping core.
				*/
		switch (ss->type)	{
		case search_reverse:
			rev=1-rev;
			continue;
		case search_orderedsubj:
		case search_arrival:
		case search_cc:
		case search_date:
		case search_from:
		case search_size:
		case search_to:
			p=ss->as;
			if (!p)	p="";
			msg->sortfields[i]=my_strdup(p);
			msg->sortorder[i]=rev;
			/* fprintf(stderr, "%d %s\n", msg->sortorder[i], msg->sortfields[i]); */
			++i;
			rev=0;
			continue;
		default:
			break;
		}
		break;
	}

	msg->nfields=sm->nfields;
	msg->n=isuid ? current_maildir_info.msgs[n].uid:n+1;
	msg->next=sm->list;
	sm->list=msg;
	++sm->nmsgs;
}
