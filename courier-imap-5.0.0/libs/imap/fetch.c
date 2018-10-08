/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#include	<sys/stat.h>

#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"imapscanclient.h"
#include	"fetchinfo.h"
#include	"rfc822/rfc822.h"
#include	"rfc2045/rfc2045.h"
#include	"maildir/config.h"
#include	"maildir/maildirgetquota.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildiraclt.h"

#if SMAP
extern int smapflag;
#endif

static const char unavailable[]=
	"\
From: System Administrator <root@localhost>\n\
Subject: message unavailable\n\n\
This message is no longer available on the server.\n";

unsigned long header_count=0, body_count=0;	/* Total transferred */

extern int current_mailbox_ro;
extern char *current_mailbox_acl;
extern struct imapscaninfo current_maildir_info;
extern char *current_mailbox;
extern char *rfc2045id(struct rfc2045 *);

extern void snapshot_needed();

extern void msgenvelope(void (*)(const char *, size_t),
                FILE *, struct rfc2045 *);
extern void msgbodystructure( void (*)(const char *, size_t), int,
        FILE *, struct rfc2045 *);

extern int is_trash(const char *);
extern void get_message_flags(struct imapscanmessageinfo *,
	char *, struct imapflags *);
extern void append_flags(char *, struct imapflags *);

static int fetchitem(FILE **, int *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 **);

static void bodystructure(FILE *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 *);

static void body(FILE *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 *);

static void fetchmsgbody(FILE *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 *);

static void dofetchmsgbody(FILE *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 *);

static void envelope(FILE *, struct fetchinfo *,
	struct imapscaninfo *,  unsigned long,
	struct rfc2045 *);

void doflags(FILE *, struct fetchinfo *,
	     struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void internaldate(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void uid(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void all(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void fast(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void full(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

static void rfc822size(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long, struct rfc2045 *);

#if 0
static void do_envelope(FILE *, struct fetchinfo *,
	struct imapscanmessageinfo *, struct rfc2045 *);
#endif

static void dofetchheadersbuf(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long,
	struct rfc2045 *,
	int (*)(struct fetchinfo *fi, const char *));
static void dofetchheadersfile(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long,
	struct rfc2045 *,
	int (*)(struct fetchinfo *fi, const char *));

static void print_bodysection_partial(struct fetchinfo *,
		void (*)(const char *));
static void print_bodysection_output(const char *);

static int dofetchheaderfields(struct fetchinfo *, const char *);
static int dofetchheadernotfields(struct fetchinfo *, const char *);
static int dofetchheadermime(struct fetchinfo *, const char *);

static void rfc822(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long,
	struct rfc2045 *);

static void rfc822header(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long,
	struct rfc2045 *);

static void rfc822text(FILE *, struct fetchinfo *,
	struct imapscaninfo *, unsigned long,
	struct rfc2045 *);

struct rfc2045 *fetch_alloc_rfc2045(unsigned long, FILE *);
FILE *open_cached_fp(unsigned long);

void fetchflags(unsigned long);

static void fetcherrorprt(const char *p)
{
	fprintf(stderr, "%s", p);
}

static void fetcherror(const char *errmsg,
		struct fetchinfo *fi,
		struct imapscaninfo *info, unsigned long j)
{
struct imapscanmessageinfo *mi=info->msgs+j;

	fprintf(stderr, "IMAP FETCH ERROR: %s, uid=%u, filename=%s: %s",
		errmsg, (unsigned)getuid(), mi->filename, fi->name);
	if (fi->bodysection)
		print_bodysection_partial(fi, &fetcherrorprt);
	fprintf(stderr, "\n");
}

char *get_reflagged_filename(const char *fn, struct imapflags *newflags)
{
	char *p=malloc(strlen(fn)+20);
	char *q;

	if (!p)	write_error_exit(0);
	strcpy(p, fn);
	if ((q=strrchr(p, MDIRSEP[0])) != 0)	*q=0;
	strcat(p, MDIRSEP "2,");
	append_flags(p, newflags);
	return p;
}

int reflag_filename(struct imapscanmessageinfo *mi, struct imapflags *flags,
	int fd)
{
char    *p, *q, *r;
int	rc=0;
struct	imapflags old_flags;
struct	stat	stat_buf;

	get_message_flags(mi, 0, &old_flags);

	p=get_reflagged_filename(mi->filename, flags);

	q=malloc(strlen(current_mailbox)+strlen(mi->filename)+sizeof("/cur/"));
	r=malloc(strlen(current_mailbox)+strlen(p)+sizeof("/cur/"));
	if (!q || !r)	write_error_exit(0);
	strcat(strcat(strcpy(q, current_mailbox), "/cur/"), mi->filename);
	strcat(strcat(strcpy(r, current_mailbox), "/cur/"), p);
	if (strcmp(q, r))
	{
		if (maildirquota_countfolder(current_mailbox)
			&& old_flags.deleted != flags->deleted
			&& fstat(fd, &stat_buf) == 0)
		{
			struct maildirsize quotainfo;
			int64_t	nbytes;
			unsigned long unbytes;
			int	nmsgs=1;

			if (maildir_parsequota(mi->filename, &unbytes) == 0)
				nbytes=unbytes;
			else
				nbytes=stat_buf.st_size;
			if ( flags->deleted )
			{
				nbytes= -nbytes;
				nmsgs= -nmsgs;
			}

			if ( maildir_quota_delundel_start(current_mailbox,
							  &quotainfo,
							  nbytes, nmsgs))
				rc= -1;
			else
				maildir_quota_delundel_end(&quotainfo,
							   nbytes, nmsgs);
		}

		if (rc == 0)
			rename(q, r);

#if SMAP
		snapshot_needed();
#endif
	}
	free(q);
	free(r);
	free(mi->filename);
	mi->filename=p;

#if 0
	if (is_sharedsubdir(current_mailbox))
		maildir_shared_updateflags(current_mailbox, p);
#endif

	return (rc);
}

int do_fetch(unsigned long n, int byuid, void *p)
{
	struct fetchinfo *fi=(struct fetchinfo *)p;
	FILE	*fp;
	struct	rfc2045 *rfc2045p;
	int	seen;
	int	open_err;

	fp=NULL;
	open_err=0;

	writes("* ");
	writen(n);
	writes(" FETCH (");

	if (byuid)
	{
	struct fetchinfo *fip;

		for (fip=fi; fip; fip=fip->next)
			if (strcmp(fip->name, "UID") == 0)
				break;

		if (fip == 0)
		{
			writes("UID ");
			writen(current_maildir_info.msgs[n-1].uid);
			writes(" ");
		}
	}
	seen=0;
	rfc2045p=0;
	while (fi)
	{
		if (fetchitem(&fp, &open_err, fi, &current_maildir_info, n-1,
			&rfc2045p))	seen=1;
		if ((fi=fi->next) != 0)	writes(" ");
	}
	writes(")\r\n");

	if (open_err)
	{
		writes("* NO Cannot open message ");
		writen(n);
		writes("\r\n");
		return (0);
	}


#if SMAP
	if (!smapflag)
#endif
		if (current_mailbox_acl &&
		    strchr(current_mailbox_acl, ACL_SEEN[0]) == NULL)
			seen=0; /* No permissions */

	if (seen && !current_mailbox_ro)
	{
	struct	imapflags	flags;

		get_message_flags(current_maildir_info.msgs+(n-1),
				0, &flags);
		if (!flags.seen)
		{
			flags.seen=1;
			reflag_filename(&current_maildir_info.msgs[n-1],&flags,
				fileno(fp));
			current_maildir_info.msgs[n-1].changedflags=1;
		}
	}

	if (current_maildir_info.msgs[n-1].changedflags)
		fetchflags(n-1);
	return (0);
}

static int fetchitem(FILE **fp, int *open_err, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 **mimep)
{
	void (*fetchfunc)(FILE *, struct fetchinfo *,
			  struct imapscaninfo *, unsigned long,
			  struct rfc2045 *);
	int	parsemime=0;
	int	rc=0;
	int	do_open=1;

	if (strcmp(fi->name, "ALL") == 0)
	{
		parsemime=1;
		fetchfunc= &all;
	}
	else if (strcmp(fi->name, "BODYSTRUCTURE") == 0)
	{
		parsemime=1;
		fetchfunc= &bodystructure;
	}
	else if (strcmp(fi->name, "BODY") == 0)
	{
		parsemime=1;
		fetchfunc= &body;
		if (fi->bodysection)
		{
			fetchfunc= &fetchmsgbody;
			rc=1;
		}
	}
	else if (strcmp(fi->name, "BODY.PEEK") == 0)
	{
		parsemime=1;
		fetchfunc= &body;
		if (fi->bodysection)
			fetchfunc= &fetchmsgbody;
	}
	else if (strcmp(fi->name, "ENVELOPE") == 0)
	{
		parsemime=1;
		fetchfunc= &envelope;
	}
	else if (strcmp(fi->name, "FAST") == 0)
	{
		parsemime=1;
		fetchfunc= &fast;
	}
	else if (strcmp(fi->name, "FULL") == 0)
	{
		parsemime=1;
		fetchfunc= &full;
	}
	else if (strcmp(fi->name, "FLAGS") == 0)
	{
		fetchfunc= &doflags;
		do_open=0;
	}
	else if (strcmp(fi->name, "INTERNALDATE") == 0)
	{
		fetchfunc= &internaldate;
	}
	else if (strcmp(fi->name, "RFC822") == 0)
	{
		fetchfunc= &rfc822;
		rc=1;
	}
	else if (strcmp(fi->name, "RFC822.HEADER") == 0)
	{
		fetchfunc= &rfc822header;
	}
	else if (strcmp(fi->name, "RFC822.SIZE") == 0)
	{
		parsemime=1;
		fetchfunc= &rfc822size;
	}
	else if (strcmp(fi->name, "RFC822.TEXT") == 0)
	{
		parsemime=1;
		fetchfunc= &rfc822text;
	}
	else if (strcmp(fi->name, "UID") == 0)
	{
		fetchfunc= &uid;
		do_open=0;
	}
	else	return (0);

	if (do_open && *fp == NULL)
	{
		*fp=open_cached_fp(msgnum);
		if (!*fp)
		{
			*open_err=1;
			return rc;
		}
	}

	if (parsemime && !*mimep)
	{
		*mimep=fetch_alloc_rfc2045(msgnum, *fp);
	}

	(*fetchfunc)(*fp, fi, i, msgnum, *mimep);
	return (rc);
}

static void bodystructure(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	writes("BODYSTRUCTURE ");
	msgbodystructure(writemem, 1, fp, mimep);
}

static void body(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	writes("BODY ");
	msgbodystructure(writemem, 0, fp, mimep);
}

static void envelope(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	writes("ENVELOPE ");
	msgenvelope( &writemem, fp, mimep);
}

void fetchflags(unsigned long n)
{
#if SMAP
	if (smapflag)
	{
		writes("* FETCH ");
		writen(n+1);
	}
	else
#endif
	{
		writes("* ");
		writen(n+1);
		writes(" FETCH (");
	}

	doflags(0, 0, &current_maildir_info, n, 0);

#if SMAP
	if (smapflag)
	{
		writes("\n");
	}
	else
#endif
		writes(")\r\n");
}

void fetchflags_byuid(unsigned long n)
{
	writes("* ");
	writen(n+1);
	writes(" FETCH (");
	uid(0, 0, &current_maildir_info, n, 0);
	writes(" ");
	doflags(0, 0, &current_maildir_info, n, 0);
	writes(")\r\n");
}

void doflags(FILE *fp, struct fetchinfo *fi,
	     struct imapscaninfo *i, unsigned long msgnum,
	     struct rfc2045 *mimep)
{
	struct libmail_kwMessageEntry *kme;

	char	buf[256];

#if SMAP
	if (smapflag)
	{
		writes(" FLAGS=");
		get_message_flags(i->msgs+msgnum, buf, 0);
		writes(buf);
	}
	else
#endif
	{
		struct libmail_kwMessage *km;

		writes("FLAGS ");

		get_message_flags(i->msgs+msgnum, buf, 0);

		writes("(");
		writes(buf);

		if (buf[0])
			strcpy(buf, " ");

		if ((km=i->msgs[msgnum].keywordMsg) != NULL)
			for (kme=km->firstEntry; kme; kme=kme->next)
			{
				writes(buf);
				strcpy(buf, " ");
				writes(keywordName(kme->libmail_keywordEntryPtr));
			}
		writes(")");
	}

	i->msgs[msgnum].changedflags=0;
}

static void internaldate(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
struct	stat	stat_buf;
char	buf[256];
char	*p, *q;

	writes("INTERNALDATE ");
	if (fstat(fileno(fp), &stat_buf) == 0)
	{
		rfc822_mkdate_buf(stat_buf.st_mtime, buf);

		/* Convert RFC822 date to imap date */

		p=strchr(buf, ',');
		if (p)	++p;
		else	p=buf;
		while (*p == ' ')	++p;
		if ((q=strchr(p, ' ')) != 0)	*q++='-';
		if ((q=strchr(p, ' ')) != 0)	*q++='-';
		writes("\"");
		writes(p);
		writes("\"");
	}
	else
		writes("NIL");
}

static void uid(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	writes("UID ");
	writen(i->msgs[msgnum].uid);
}

static void rfc822size(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;

	writes("RFC822.SIZE ");

	rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	writen(end_pos - start_pos + nlines);
}

static void all(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	doflags(fp, fi, i, msgnum, mimep);
	writes(" ");
	internaldate(fp, fi, i, msgnum, mimep);
	writes(" ");
	rfc822size(fp, fi, i, msgnum, mimep);
	writes(" ");
	envelope(fp, fi, i, msgnum, mimep);
}

static void fast(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	doflags(fp, fi, i, msgnum, mimep);
	writes(" ");
	internaldate(fp, fi, i, msgnum, mimep);
	writes(" ");
	rfc822size(fp, fi, i, msgnum, mimep);
}

static void full(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	doflags(fp, fi, i, msgnum, mimep);
	writes(" ");
	internaldate(fp, fi, i, msgnum, mimep);
	writes(" ");
	rfc822size(fp, fi, i, msgnum, mimep);
	writes(" ");
	envelope(fp, fi, i, msgnum, mimep);
	writes(" ");
	body(fp, fi, i, msgnum, mimep);
}

static void fetchmsgbody(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
	writes("BODY");
	print_bodysection_partial(fi, &print_bodysection_output);
	writes(" ");
	dofetchmsgbody(fp, fi, i, msgnum, mimep);
}

static void print_bodysection_output(const char *p)
{
	writes(p);
}

static void print_bodysection_partial(struct fetchinfo *fi,
	void (*func)(const char *))
{
	(*func)("[");
	if (fi->bodysection)
	{
	struct fetchinfo *subl;

		(*func)(fi->bodysection);
		if (fi->bodysublist)
		{
		char	*p=" (";

			for (subl=fi->bodysublist; subl; subl=subl->next)
			{
				(*func)(p);
				p=" ";
				(*func)("\"");
				(*func)(subl->name);
				(*func)("\"");
			}
			(*func)(")");
		}
	}
	(*func)("]");
	if (fi->ispartial)
	{
	char	buf[80];

		sprintf(buf, "<%lu>", (unsigned long)fi->partialstart);
		(*func)(buf);
	}
}

static void dofetchmsgbody(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *i, unsigned long msgnum,
	struct rfc2045 *mimep)
{
const char *p=fi->bodysection;
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;
unsigned long cnt;
char	buf[BUFSIZ];
char	rbuf[BUFSIZ];
char	*rbufptr;
int	rbufleft;
unsigned long bufptr;
unsigned long skipping;
int	ismsgrfc822=1;

off_t start_seek_pos;
 struct rfc2045 *headermimep;

/*
** To optimize consecutive FETCHes, we cache our virtual and physical
** position.  What we do is that on the first fetch we count off the
** characters we read, and keep track of both the physical and the CRLF-based
** offset into the message.  Then, on subsequent FETCHes, we attempt to
** use that information.
*/

off_t cnt_virtual_chars;
off_t cnt_phys_chars;

off_t cache_virtual_chars;
off_t cache_phys_chars;

	headermimep=mimep;
 
	while (p && isdigit((int)(unsigned char)*p))
	{
	unsigned long n=0;

		headermimep=mimep;

		do
		{
			n=n*10 + (*p++ - '0');
		} while (isdigit((int)(unsigned char)*p));

		if (mimep)
		{
			if (ismsgrfc822)
			{
				const char *ct, *dummy;

				if (mimep->firstpart == 0)
				{
					/* Not a multipart, n must be 1 */
					if (n != 1)
						mimep=0;
					if (*p == '.')
						++p;
					continue;
				}
				ismsgrfc822=0;

				rfc2045_mimeinfo(mimep, &ct,
						 &dummy,
						 &dummy);

				if (ct && strcasecmp(ct, "message/rfc822"
						     ) == 0)
					ismsgrfc822=1;
				/* The content is another message/rfc822 */
			}

			mimep=mimep->firstpart;
			while (mimep)
			{
				if (!mimep->isdummy && --n == 0)
					break;
				mimep=mimep->next;
			}
			headermimep=mimep;

			if (mimep && mimep->firstpart &&
				!mimep->firstpart->isdummy)
				/* This is a message/rfc822 part */
			{
				if (!*p)
					break;

				mimep=mimep->firstpart;
				ismsgrfc822=1;
			}
		}
		if (*p == '.')
			++p;
	}

	if (p && strcmp(p, "MIME") == 0)
		mimep=headermimep;

	if (mimep == 0)
	{
		writes("{0}\r\n");
		return;
	}

	rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);


	if (p && strcmp(p, "TEXT") == 0)
	{
		start_seek_pos=start_body;
		cnt=end_pos - start_body + nbodylines;
	}
	else if (p && strcmp(p, "HEADER") == 0)
	{
		start_seek_pos=start_pos;
		cnt= start_body - start_pos + (nlines - nbodylines);
	}
	else if (p && strcmp(p, "HEADER.FIELDS") == 0)
	{
		if (start_body - start_pos <= BUFSIZ)
			dofetchheadersbuf(fp, fi, i, msgnum, mimep,
				&dofetchheaderfields);
		else
			dofetchheadersfile(fp, fi, i, msgnum, mimep,
				&dofetchheaderfields);
		return;
	}
	else if (p && strcmp(p, "HEADER.FIELDS.NOT") == 0)
	{
		if (start_body - start_pos <= BUFSIZ)
			dofetchheadersbuf(fp, fi, i, msgnum, mimep,
				&dofetchheadernotfields);
		else
			dofetchheadersfile(fp, fi, i, msgnum, mimep,
				&dofetchheadernotfields);
		return;
	}
	else if (p && strcmp(p, "MIME") == 0)
	{
		if (start_body - start_pos <= BUFSIZ)
			dofetchheadersbuf(fp, fi, i, msgnum, mimep,
				&dofetchheadermime);
		else
			dofetchheadersfile(fp, fi, i, msgnum, mimep,
				&dofetchheadermime);
		return;
	}
	else if (*fi->bodysection == 0)
	{
		start_seek_pos=start_pos;

		cnt= end_pos - start_pos + nlines;
	}
	else	/* Last possibility: entire body */
	{
		start_seek_pos=start_body;

		cnt= end_pos - start_body + nbodylines;
	}

	skipping=0;
	if (fi->ispartial)
	{
		skipping=fi->partialstart;
		if (skipping > cnt)	skipping=cnt;
		cnt -= skipping;
		if (fi->ispartial > 1 && cnt > fi->partialend)
			cnt=fi->partialend;
	}

	if (get_cached_offsets(start_seek_pos, &cnt_virtual_chars,
			       &cnt_phys_chars) == 0 &&
	    cnt_virtual_chars <= skipping)	/* Yeah - cache it, baby! */
	{
		if (fseek(fp, start_seek_pos+cnt_phys_chars, SEEK_SET) == -1)
		{
			writes("{0}\r\n");
			fetcherror("fseek", fi, i, msgnum);
			return;
		}
		skipping -= cnt_virtual_chars;
	}
	else
	{
		if (fseek(fp, start_seek_pos, SEEK_SET) == -1)
		{
			writes("{0}\r\n");
			fetcherror("fseek", fi, i, msgnum);
			return;
		}

		cnt_virtual_chars=0;
		cnt_phys_chars=0;
	}

	cache_virtual_chars=cnt_virtual_chars;
	cache_phys_chars=cnt_phys_chars;

	writes("{");
	writen(cnt);
	writes("}\r\n");
	bufptr=0;
	writeflush();

	rbufptr=0;
	rbufleft=0;

	while (cnt)
	{
	int	c;

		if (!rbufleft)
		{
			rbufleft=fread(rbuf, 1, sizeof(rbuf), fp);
			if (rbufleft < 0)	rbufleft=0;
			rbufptr=rbuf;
		}

		if (!rbufleft)
		{
			fetcherror("unexpected EOF", fi, i, msgnum);
			_exit(1);
		}

		--rbufleft;
		c=(int)(unsigned char)*rbufptr++;
		++cnt_phys_chars;

		if (c == '\n')
		{
			++cnt_virtual_chars;

			if (skipping)
				--skipping;
			else
			{
				if (bufptr >= sizeof(buf))
				{
					writemem(buf, sizeof(buf));
					bufptr=0;
					/*writeflush();*/
				}
				buf[bufptr++]='\r';
				--cnt;

				if (cnt == 0)
					break;
			}
		}

		++cnt_virtual_chars;
		if (skipping)
			--skipping;
		else
		{
			++body_count;

			if (bufptr >= sizeof(buf))
			{
				writemem(buf, sizeof(buf));
				bufptr=0;
				/*writeflush();*/
			}
			buf[bufptr++]=c;
			--cnt;
		}
		cache_virtual_chars=cnt_virtual_chars;
		cache_phys_chars=cnt_phys_chars;
	}
	writemem(buf, bufptr);
	writeflush();
	save_cached_offsets(start_seek_pos, cache_virtual_chars,
			    cache_phys_chars);
}

static int dofetchheaderfields(struct fetchinfo *fi, const char *name)
{
	for (fi=fi->bodysublist; fi; fi=fi->next)
	{
	int	i, a, b;

		if (fi->name == 0)	continue;
		for (i=0; fi->name[i] && name[i]; i++)
		{
			a=(unsigned char)name[i];
			a=toupper(a);
			b=fi->name[i];
			b=toupper(b);
			if (a != b)	break;
		}
		if (fi->name[i] == 0 && name[i] == 0)	return (1);
	}

	return (0);
}

static int dofetchheadernotfields(struct fetchinfo *fi, const char *name)
{
	return (!dofetchheaderfields(fi, name));
}

static int dofetchheadermime(struct fetchinfo *fi, const char *name)
{
int	i, a;
static const char mv[]="MIME-VERSION";

	for (i=0; i<sizeof(mv)-1; i++)
	{
		a= (unsigned char)name[i];
		a=toupper(a);
		if (a != mv[i])	break;
	}
	if (mv[i] == 0 && name[i] == 0)	return (1);

	for (i=0; i<8; i++)
	{
		a= (unsigned char)name[i];
		a=toupper(a);
		if (a != "CONTENT-"[i])	return (0);
	}
	return (1);
}

static void dofetchheadersbuf(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *info, unsigned long msgnum,
	struct rfc2045 *mimep,
	int (*headerfunc)(struct fetchinfo *fi, const char *))
{
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;
size_t i,j,k,l;
char	buf[BUFSIZ+2];
int	goodheader;
unsigned long skipping;
unsigned long cnt;
char	*p;
int	ii;

	rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);
	if (fseek(fp, start_pos, SEEK_SET) == -1)
	{
		writes("{0}\r\n");
		fetcherror("fseek", fi, info, msgnum);
		return;
	}

	ii=fread(buf, 1, start_body - start_pos, fp);
	if (ii < 0 || (i=ii) != start_body - start_pos)
	{
		fetcherror("unexpected EOF", fi, info, msgnum);
		exit(1);
	}
	goodheader= (*headerfunc)(fi, "");

	l=0;
	for (j=0; j<i; )
	{
		if (buf[j] != '\n' && buf[j] != '\r' &&
			!isspace((int)(unsigned char)buf[j]))
		{
			goodheader= (*headerfunc)(fi, "");

			for (k=j; k<i; k++)
			{
				if (buf[k] == '\n' || buf[k] == ':')
					break;
			}

			if (k < i && buf[k] == ':')
			{
				buf[k]=0;
				goodheader=(*headerfunc)(fi, buf+j);
				buf[k]=':';
			}
		}
		else if (buf[j] == '\n')
			goodheader=0;

		for (k=j; k<i; k++)
			if (buf[k] == '\n')
			{
				++k;
				break;
			}

		if (goodheader)
		{
			while (j<k)
				buf[l++]=buf[j++];
		}
		j=k;
	}

	buf[l++]='\n';	/* Always append a blank line */

	cnt=l;
	for (i=0; i<l; i++)
		if (buf[i] == '\n')	++cnt;

	skipping=0;
	if (fi->ispartial)
	{
		skipping=fi->partialstart;
		if (skipping > cnt)	skipping=cnt;
		cnt -= skipping;
		if (fi->ispartial > 1 && cnt > fi->partialend)
			cnt=fi->partialend;
	}

	writes("{");
	writen(cnt);
	writes("}\r\n");
	p=buf;
	while (skipping)
	{
		if (*p == '\n')
		{
			--skipping;
			if (skipping == 0)
			{
				if (cnt)
				{
					writes("\n");
					--cnt;
				}
				break;
			}
		}
		--skipping;
		++p;
	}

	while (cnt)
	{
		if (*p == '\n')
		{
			writes("\r");
			if (--cnt == 0)	break;
			writes("\n");
			--cnt;
			++p;
			continue;
		}
		for (i=0; i<cnt; i++)
			if (p[i] == '\n')
				break;
		writemem(p, i);
		p += i;
		cnt -= i;
		header_count += i;
	}
}

struct fetchheaderinfo {
	unsigned long skipping;
	unsigned long cnt;
	} ;

static void countheader(struct fetchheaderinfo *, const char *, size_t);

static void printheader(struct fetchheaderinfo *, const char *, size_t);

static void dofetchheadersfile(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *info, unsigned long msgnum,
	struct rfc2045 *mimep,
	int (*headerfunc)(struct fetchinfo *fi, const char *))
{
off_t start_pos, end_pos, start_body, left;
off_t nlines, nbodylines;
size_t i;
int	c, pass;
char	buf1[256];
int	goodheader;
struct	fetchheaderinfo finfo;

	finfo.cnt=0;
	for (pass=0; pass<2; pass++)
	{
	void (*func)(struct fetchheaderinfo *, const char *, size_t)=
			pass ? printheader:countheader;

		rfc2045_mimepos(mimep, &start_pos, &end_pos, &start_body,
			&nlines, &nbodylines);
		if (fseek(fp, start_pos, SEEK_SET) == -1)
		{
			writes("{0}\r\n");
			fetcherror("fseek", fi, info, msgnum);
			return;
		}
		if (pass)
		{
			finfo.skipping=0;
			if (fi->ispartial)
			{
				finfo.skipping=fi->partialstart;
				if (finfo.skipping > finfo.cnt)
					finfo.skipping=finfo.cnt;
				finfo.cnt -= finfo.skipping;
				if (fi->ispartial > 1 &&
					finfo.cnt > fi->partialend)
					finfo.cnt=fi->partialend;
			}

			writes("{");
			writen(finfo.cnt+2);	/* BUG */
			writes("}\r\n");
		}
		left=start_body - start_pos;

		goodheader= (*headerfunc)(fi, "");
		while (left)
		{
			for (i=0; i<sizeof(buf1)-1 && i<left; i++)
			{
				c=getc(fp);
				if (c == EOF)
				{
					fetcherror("unexpected EOF", fi, info, msgnum);
					_exit(1);
				}

				if (c == '\n' || c == ':')
				{
					ungetc(c, fp);
					break;
				}
				buf1[i]=c;
			}
			buf1[i]=0;
			left -= i;

			if (buf1[0] != '\n' && buf1[0] != '\r' &&
				!isspace((int)(unsigned char)buf1[0]))
				goodheader= (*headerfunc)(fi, buf1);
			else if (buf1[0] == '\n')
				goodheader=0;

			if (!goodheader)
			{
				while (left)
				{
					c=getc(fp);
					--left;
					if (c == EOF)
					{
						fetcherror("unexpected EOF", fi, info, msgnum);
						_exit(1);
					}
					if (c == '\n')	break;
				}
				continue;
			}

			(*func)(&finfo, buf1, i);

			i=0;
			while (left)
			{
				c=getc(fp);
				if (c == EOF)
				{
					fetcherror("unexpected EOF", fi, info, msgnum);
					_exit(1);
				}
				--left;
				if (i >= sizeof(buf1))
				{
					(*func)(&finfo, buf1, i);
					i=0;
				}
				if (c == '\n')
				{
					(*func)(&finfo, buf1, i);
					buf1[0]='\r';
					i=1;
				}
				buf1[i++]=c;
				if (c == '\n')	break;
			}
			(*func)(&finfo, buf1, i);
			if (pass && finfo.cnt == 0)	break;
		}
	}
	writes("\r\n");	/* BUG */
}

static void countheader(struct fetchheaderinfo *fi, const char *p, size_t s)
{
	fi->cnt += s;
}

static void printheader(struct fetchheaderinfo *fi, const char *p, size_t s)
{
	size_t i;

	if (fi->skipping)
	{
		if (fi->skipping > s)
		{
			fi->skipping -= s;
			return;
		}
		p += fi->skipping;
		s -= fi->skipping;
		fi->skipping=0;
	}
	if (s > fi->cnt)	s=fi->cnt;
	for (i=0; i <= s; i++)
		if (p[i] != '\r')
			++header_count;
	writemem(p, s);
	fi->cnt -= s;
}

static void rfc822(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *info, unsigned long msgnum,
	struct rfc2045 *rfcp)
{
unsigned long n=0;
int	c;
char	buf[BUFSIZ];
unsigned long i;

	writes("RFC822 ");

	if (fseek(fp, 0L, SEEK_SET) == -1)
	{
		fetcherror("fseek", fi, info, msgnum);
		writes("{0}\r\n");
		return;
	}
	while ((c=getc(fp)) != EOF)
	{
		++n;
		if (c == '\n')	++n;
	}

	if (fseek(fp, 0L, SEEK_SET) == -1)
	{
		fetcherror("fseek", fi, info, msgnum);
		writes("{0}\r\n");
		return;
	}
	writes("{");
	writen(n);
	writes("}\r\n");

	i=0;
	while (n)
	{
		c=getc(fp);
		if (c == '\n')
		{
			if (i >= sizeof(buf))
			{
				writemem(buf, i);
				i=0;
			}
			buf[i++]='\r';
			if (--n == 0)	break;
		}

		if (i >= sizeof(buf))
		{
			writemem(buf, i);
			i=0;
		}
		buf[i++]=c;
		--n;
		++body_count;
	}
	writemem(buf, i);
}

static void rfc822header(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *info, unsigned long msgnum,
	struct rfc2045 *rfcp)
{
unsigned long n=0;
int	c;
char	buf[BUFSIZ];
unsigned long i;
int	eol;

	writes("RFC822.HEADER ");

	if (fseek(fp, 0L, SEEK_SET) == -1)
	{
		fetcherror("fseek", fi, info, msgnum);
		writes("{0}\r\n");
		return;
	}

	eol=0;
	while ((c=getc(fp)) != EOF)
	{
		++n;
		if (c != '\n')
		{
			eol=0;
			continue;
		}
		++n;
		if (eol)	break;
		eol=1;
	}

	if (fseek(fp, 0L, SEEK_SET) == -1)
	{
		fetcherror("fseek", fi, info, msgnum);
		writes("{0}\r\n");
		return;
	}
	writes("{");
	writen(n);
	writes("}\r\n");

	i=0;
	while (n)
	{
		c=getc(fp);
		if (c == '\n')
		{
			if (i >= sizeof(buf))
			{
				writemem(buf, i);
				i=0;
			}
			buf[i++]='\r';
			if (--n == 0)	break;
		}

		if (i >= sizeof(buf))
		{
			writemem(buf, i);
			i=0;
		}
		buf[i++]=c;
		--n;
		++header_count;
	}
	writemem(buf, i);
}

static void rfc822text(FILE *fp, struct fetchinfo *fi,
	struct imapscaninfo *info, unsigned long msgnum,
	struct rfc2045 *rfcp)
{
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;
unsigned long i;
int	c;
char	buf[BUFSIZ];
unsigned long l;

	writes("RFC822.TEXT {");

	rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	if (fseek(fp, start_body, SEEK_SET) == -1)
	{
		fetcherror("fseek", fi, info, msgnum);
		writes("0}\r\n");
		return;
	}

	i=end_pos - start_body + nbodylines;

	writen(i);
	writes("}\r\n");

	l=0;
	while (i)
	{
		c=getc(fp);
		if (c == EOF)
		{
			fetcherror("unexpected EOF", fi, info, msgnum);
			_exit(1);
		}
		--i;
		if (l >= sizeof(BUFSIZ))
		{
			writemem(buf, l);
			l=0;
		}
		if (c == '\n' && i)
		{
			--i;
			buf[l++]='\r';
			if (l >= sizeof(BUFSIZ))
			{
				writemem(buf, l);
				l=0;
			}
		}
		buf[l++]=c;
		++body_count;
	}
	writemem(buf, l);
}

/*
** Poorly written IMAP clients (read: Netscape Messenger) like to issue
** consecutive partial fetches for downloading large messages.
**
** To save the time of reparsing the MIME structure, we cache it.
*/

static struct rfc2045 *cached_rfc2045p;
static char *cached_filename;

void fetch_free_cached()
{
	if (cached_rfc2045p)
	{
		rfc2045_free(cached_rfc2045p);
		cached_rfc2045p=0;
		free(cached_filename);
		cached_filename=0;
	}
}

struct rfc2045 *fetch_alloc_rfc2045(unsigned long msgnum, FILE *fp)
{
	if (cached_rfc2045p &&
	    strcmp(cached_filename,
		   current_maildir_info.msgs[msgnum].filename) == 0)
		return (cached_rfc2045p);

	fetch_free_cached();

	if ((cached_filename=strdup(current_maildir_info.
				    msgs[msgnum].filename))
	    == 0) write_error_exit(0);

	if (fseek(fp, 0L, SEEK_SET) == -1)
	{
		write_error_exit(0);
		return (0);
	}
	cached_rfc2045p=rfc2045_fromfp(fp);
	if (!cached_rfc2045p)
	{
		free(cached_filename);
		cached_filename=0;
		write_error_exit(0);
	}
	return (cached_rfc2045p);
}

static FILE *cached_fp=0;
static char *cached_fp_filename=0;
static off_t cached_base_offset;
static off_t cached_virtual_offset;
static off_t cached_phys_offset;

FILE *open_cached_fp(unsigned long msgnum)
{
	int	fd;

	if (cached_fp && strcmp(cached_fp_filename,
				current_maildir_info.msgs[msgnum].filename)
	    == 0)
		return (cached_fp);

	if (cached_fp)
	{
		fclose(cached_fp);
		free(cached_fp_filename);
		cached_fp_filename=0;
		cached_fp=0;
	}

	fd=imapscan_openfile(current_mailbox, &current_maildir_info, msgnum);
	if (fd < 0 || (cached_fp=fdopen(fd, "r")) == 0)
	{
		if (fd >= 0)	close(fd);

		if ((cached_fp=tmpfile()) != 0)
		{
			fprintf(cached_fp, unavailable);
			if (fseek(cached_fp, 0L, SEEK_SET) < 0 ||
			    ferror(cached_fp))
			{
				fclose(cached_fp);
				cached_fp=0;
			}
		}

		if (cached_fp == 0)
		{
			fprintf(stderr, "ERR: %s: %s\n",
				getenv("AUTHENTICATED"),
#if	HAVE_STRERROR
				strerror(errno)
#else
				"error"
#endif

				);
			fflush(stderr);
			_exit(1);
		}
	}

	if ((cached_fp_filename=strdup(current_maildir_info.
				       msgs[msgnum].filename))
	    == 0)
	{
		fclose(cached_fp);
		cached_fp=0;
		write_error_exit(0);
	}
	cached_base_offset=0;
	cached_virtual_offset=0;
	cached_phys_offset=0;
	return (cached_fp);
}

void fetch_free_cache()
{
	if (cached_fp)
	{
		fclose(cached_fp);
		cached_fp=0;
		free(cached_fp_filename);
		cached_fp_filename=0;
	}
}

void save_cached_offsets(off_t base, off_t virt, off_t phys)
{
	cached_base_offset=base;
	cached_virtual_offset=virt;
	cached_phys_offset=phys;
}

int get_cached_offsets(off_t base, off_t *virt, off_t *phys)
{
	if (!cached_fp)
		return (-1);
	if (base != cached_base_offset)
		return (-1);

	*virt=cached_virtual_offset;
	*phys=cached_phys_offset;
	return (0);
}
