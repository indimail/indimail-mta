/*
** Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/
#if    HAVE_CONFIG_H
#include       "rfc2045_config.h"
#endif
#include       <stdlib.h>
#include       <stdio.h>
#include       <string.h>
#if    HAVE_STRINGS_H
#include       <strings.h>
#endif
#include	<ctype.h>
#include	"rfc2045.h"
#include	"rfc822/rfc822.h"
#include	"rfc2045charset.h"

static char	*rfc2045_defcharset=0;

int rfc2045_in_reformime=0;

extern void rfc2045_enomem();

#define	MAXLEVELS	20
#define	MAXPARTS	300

/*
	New RFC2045 structure.
*/

struct rfc2045 *rfc2045_alloc()
{
struct rfc2045 *p=(struct rfc2045 *)malloc(sizeof(struct rfc2045));

	if (!p)
	{
		rfc2045_enomem();
		return (0);
	}

	/* Initialize everything to nulls, except for one thing */

	memset(p, '\0', sizeof(*p));

	p->pindex=1;	/* Start with part #1 */
	p->workinheader=1;
	/* Most of the time, we're about to read a header */

	return (p);
}

const char *rfc2045_getattr(const struct rfc2045attr *p, const char *name)
{
	while (p)
	{
		if (p->name && strcmp(p->name, name) == 0)
			return (p->value);
		p=p->next;
	}
	return (0);
}

int rfc2045_attrset(struct rfc2045attr **p, const char *name, const char *val)
{
char	*v;

	while (*p)
	{
		if (strcmp( (*p)->name, name) == 0)	break;
		p=&(*p)->next;
	}
	if (val == 0)
	{
	struct rfc2045attr *q= *p;

		if (q)
		{
			*p=q->next;
			if (q->name)	free(q->name);
			if (q->value)	free(q->value);
			free(q);
		}
		return 0;
	}

	v=strdup(val);
	if (!v)
		return -1;

	if (!*p)
	{
		if (((*p)=(struct rfc2045attr *)malloc(sizeof(**p))) == 0)
		{
			free(v);
			return -1;
		}
		memset( (*p), 0, sizeof(**p));
		if ( ((*p)->name=strdup(name)) == 0)
		{
			free( *p );
			*p=0;
			free(v);
			return -1;
		}
	}
	if ( (*p)->value )	free ( (*p)->value );
	(*p)->value=v;
	return 0;
}

/* static const char cb_name[]="boundary"; */

/* #define	ContentBoundary(p)	(rfc2045_getattr( (p)->content_type_attr, cb_name)) */

#define	ContentBoundary(p)	( (p)->boundary )

/*
	Unallocate the RFC2045 structure.  Recursively unallocate
	all sub-structures.  Unallocate all associated buffers.
*/

static void rfc2045_freeattr(struct rfc2045attr *p)
{
	while (p)
	{
	struct rfc2045attr *q=p->next;

		if (p->name)	free(p->name);
		if (p->value)	free(p->value);
		free(p);
		p=q;
	}
}

void rfc2045_free(struct rfc2045 *p)
{
struct rfc2045 *q, *r;

	for (q=p->firstpart; q; )
	{
		r=q->next;
		rfc2045_free(q);
		q=r;
	}
	rfc2045_freeattr(p->content_type_attr);
	rfc2045_freeattr(p->content_disposition_attr);

	if (p->header)		free(p->header);
	if (p->content_md5)	free(p->content_md5);
	if (p->content_base)	free(p->content_base);
	if (p->content_location)	free(p->content_location);
	if (p->content_language)	free(p->content_language);
	if (p->content_id)	free(p->content_id);
	if (p->content_description)	free(p->content_description);
	if (p->content_transfer_encoding) free(p->content_transfer_encoding);
	if (p->boundary) free(p->boundary);
	if (p->content_type)	free(p->content_type);
	if (p->mime_version)	free(p->mime_version);
	if (p->workbuf)		free(p->workbuf);
	if (p->content_disposition) free(p->content_disposition);
	if (p->rw_transfer_encoding) free(p->rw_transfer_encoding);
	free(p);
}

/*
	Generic dynamic buffer append.
*/

void rfc2045_add_buf(
	char **bufptr,	/* Buffer */
	size_t *bufsize,	/* Buffer's maximum size */
	size_t *buflen,		/* Buffer's current size */

	const char *p, size_t len)	/* Append this data */
{
	if (len + *buflen > *bufsize)
	{
	size_t	newsize=len+*buflen+256;
	char	*p= *bufptr ? (char *)realloc(*bufptr, newsize):
				(char *)malloc(newsize);

		if (!p)
		{
			rfc2045_enomem();
			return;
		}
		*bufptr=p;
		*bufsize=newsize;
	}

	memcpy(*bufptr + *buflen, p, len);
	*buflen += len;
}

/* Append to the work buffer */

void rfc2045_add_workbuf(struct rfc2045 *h, const char *p, size_t len)
{
	rfc2045_add_buf( &h->workbuf, &h->workbufsize, &h->workbuflen, p, len);
}

/* Append one character to the work buffer */

void rfc2045_add_workbufch(struct rfc2045 *h, int c)
{
char cc= (char)c;

	rfc2045_add_workbuf(h, &cc, 1);
}

/*
	Generic function to duplicate contents of a string.
	The destination string may already be previously allocated,
	so unallocate it.
*/

static void set_string(char **p,
	const char *q)
{
	if (*p)	free(*p);

	*p=0;
	if (!q)	return;

	if ((*p=(char *)malloc(strlen(q)+1)) == 0)
	{
		rfc2045_enomem();
		return;
	}

	strcpy(*p, q);
}

/* Update byte counts for this structure, and all the superstructures */

static void update_counts(struct rfc2045 *p, size_t newcnt, size_t newendcnt,
	unsigned nlines)
{
	while (p)
	{
		p->endpos = newcnt;
		p->endbody = newendcnt;
		p->nlines += nlines;
		if (!p->workinheader)
			p->nbodylines += nlines;
		p=p->parent;
	}
}

/*
	Main entry point for RFC2045 parsing.  External data is fed
	by repetitively calling rfc2045_parse().

	rfc2045_parse() breaks up input into lines, and calls doline()
	to process each line.
*/

static void doline(struct rfc2045 *);

void rfc2045_parse_partial(struct rfc2045 *h);

void rfc2045_parse(struct rfc2045 *h, const char *buf, size_t s)
{
	size_t	l;

	while (s)
	{
		for (l=0; l<s; l++)
			if (buf[l] == '\n')	break;
		if (l < s && buf[l] == '\n')
		{
			++l;
			rfc2045_add_workbuf(h, buf, l);
			doline(h);
			h->workbuflen=0;
		}
		else
			rfc2045_add_workbuf(h, buf, l);
		buf += l;
		s -= l;
	}

	if (h->workbuflen > 1024)
		rfc2045_parse_partial(h);
}

void rfc2045_parse_partial(struct rfc2045 *h)
{
	/*
	** Our buffer's getting pretty big.  Let's see if we can
	** partially handle it.
	*/

	if (h->workbuflen > 0)
	{
	struct	rfc2045 *p;
	int	l, i;

		for (p=h; p->lastpart && !p->lastpart->workclosed;
				p=p->lastpart)
			;

		/* If p->workinheader, we've got a mother of all headers
		** here.  Well, that's just too bad, we'll end up garbling
		** it.
		*/

		l=h->workbuflen;

		/* We do need to make sure that the final \r\n gets
		** stripped off, so don't gobble up everything if
		** the last character we see is a \r
		*/

		if (h->workbuf[l-1] == '\r')
			--l;

		/* If we'll be rewriting, make sure rwprep knows about
		** stuff that was skipped just now. */

		if (h->rfc2045acptr && !p->workinheader &&
			(!p->lastpart || !p->lastpart->workclosed))
			(*h->rfc2045acptr->section_contents)(h->workbuf, l);

		update_counts(p, p->endpos+l, p->endpos+l, 0);
		p->informdata=1;
		for (i=0; l<h->workbuflen; l++)
			h->workbuf[i++]=h->workbuf[l];
		h->workbuflen=i;
	}
}

/*
	Append a new RFC2045 subpart.  Adds new RFC2045 structure to the
	end of the list of existing RFC2045 substructures.
*/

static struct rfc2045 *append_part_noinherit(struct rfc2045 *p, size_t startpos){
struct rfc2045 *newp;

	newp=rfc2045_alloc();
	if (p->lastpart)
	{
		p->lastpart->next=newp;
		newp->pindex=p->lastpart->pindex+1;
	}
	else
	{
		p->firstpart=newp;
		newp->pindex=0;
	}
	p->lastpart=newp;
	newp->parent=p;

	/* Initialize source pointers */
	newp->startpos=newp->endpos=newp->startbody=newp->endbody=startpos;

	while (p->parent)
		p=p->parent;
	++p->numparts;

	return (newp);
}

static struct rfc2045 *append_part(struct rfc2045 *p, size_t startpos)
{
struct rfc2045 *newp=append_part_noinherit(p, startpos);

	/* Substructures inherit content transfer encoding and character set */

	set_string(&newp->content_transfer_encoding,
			p->content_transfer_encoding);

	if (rfc2045_attrset(&newp->content_type_attr, "charset",
			    rfc2045_getattr(p->content_type_attr, "charset"))
	    < 0)
		rfc2045_enomem();

	return (newp);
}

/*
	doline() processes next line in the RFC2045 message.

	Drills down the list of all the multipart messages currently open,
	and checks if the line is a boundary line for the given multipart.
	In theory the boundary line, if there is one, should be the boundary
	line only for the inner multipart only, but, this takes into account
	broken MIME messages.
*/

static void do_header(struct rfc2045 *);

static void doline(struct rfc2045 *p)
{
size_t	cnt=p->workbuflen;
char *c=p->workbuf;
size_t	n=cnt-1;	/* Strip \n (we always get at least a \n here) */
struct rfc2045 *newp;
struct rfc2045ac *rwp=p->rfc2045acptr;
unsigned num_levels=0;

size_t	k;
int	bit8=0;

	if (p->numparts > MAXPARTS)
	{
		p->rfcviolation |= RFC2045_ERR2COMPLEX;
		return;
	}

	for (k=0; k<cnt; k++)
	{
		if (c[k] == 0)
			c[k]=' ';
		if (c[k] & 0x80)	bit8=1;
	}

	if (n && c[n-1] == '\r')	/* Strip trailing \r */
		--n;

	/* Before the main drill down loop before, look ahead and see if we're
	** in a middle of a form-data section.  */

	for (newp=p; newp->lastpart &&
			!newp->lastpart->workclosed; newp=newp->lastpart,
			++num_levels)
	{
		if (ContentBoundary(newp) == 0 || newp->workinheader)
			continue;

		if (newp->lastpart->informdata)
		{
			p=newp->lastpart;
			p->informdata=0;
			break;
		}
	}

	/* Drill down until we match a boundary, or until we've reached
	the last RFC2045 section that has been opened.
	*/

	while (p->lastpart)
	{
	size_t l;
	const char *cb;

		if (p->lastpart->workclosed)
		{
			update_counts(p, p->endpos+cnt, p->endpos+n, 1);
			return;
		}
		/* Leftover trash -- workclosed is set when the final
		** terminating boundary has been seen */

		/* content_boundary may be set before the entire header
		** has been seen, so continue drilling down in that case
		*/

		cb=ContentBoundary(p);

		if (cb == 0 || p->workinheader)
		{
			p=p->lastpart;
			++num_levels;
			continue;
		}

		l=strlen(cb);

		if (c[0] == '-' && c[1] == '-' && n >= 2+l &&
			strncasecmp(cb, c+2, l) == 0)
		{

			if (rwp && (!p->lastpart || !p->lastpart->isdummy))
				(*rwp->end_section)();

		/* Ok, we've found a boundary */

			if (n >= 4+l && strncmp(c+2+l, "--", 2) == 0)
			{
			/* Last boundary */

				p->lastpart->workclosed=1;
				update_counts(p, p->endpos+cnt, p->endpos+cnt,
					1);
				return;
			}

		/* Create new RFC2045 section */

			newp=append_part(p, p->endpos+cnt);
			update_counts(p, p->endpos+cnt, p->endpos+n, 1);

			/* The new RFC2045 section is MIME compliant */

			if ((newp->mime_version=strdup(p->mime_version)) == 0)
				rfc2045_enomem();
			return;
		}
		p=p->lastpart;
		++num_levels;
	}

	/* Ok, we've found the RFC2045 section that we're working with.
	** No what?
	*/

	if (! p->workinheader)
	{
		/* Processing body, just update the counts. */

	size_t cnt_update=cnt;

		if (bit8 && !p->content_8bit &&
			(p->rfcviolation & RFC2045_ERR8BITCONTENT) == 0)
		{
		struct rfc2045 *q;

			for (q=p; q; q=q->parent)
				q->rfcviolation |= RFC2045_ERR8BITCONTENT;
		}

		/*
		** In multiparts, the final newline in a part belongs to the
		** boundary, otherwise, include it in the text.
		*/
		if (p->parent && p->parent->content_type &&
				strncasecmp(p->parent->content_type,
						"multipart/", 10) == 0)
			cnt_update=n;

		if (!p->lastpart || !p->lastpart->workclosed)
		{
			if (rwp && !p->isdummy)
				(*rwp->section_contents)(c, cnt);

			update_counts(p, p->endpos+cnt, p->endpos+cnt_update,
				1);
		}
		return;
	}

	if (bit8 && (p->rfcviolation & RFC2045_ERR8BITHEADER) == 0)
	{
	struct rfc2045 *q;

		for (q=p; q; q=q->parent)
			q->rfcviolation |= RFC2045_ERR8BITHEADER;
	}

	/* In the header */

	if ( n == 0 )	/* End of header, body begins.  Parse header. */
	{
		do_header(p);	/* Clean up any left over header line */
		p->workinheader=0;

		/* Message body starts right here */

		p->startbody=p->endpos+cnt;
		update_counts(p, p->startbody, p->startbody, 1);
		--p->nbodylines;	/* Don't count the blank line */

		/* Discard content type and boundary if I don't understand
		** this MIME flavor.
		*/

		if (!RFC2045_ISMIME1(p->mime_version))
		{
			set_string(&p->content_type, 0);

			rfc2045_freeattr(p->content_type_attr);
			p->content_type_attr=0;
			set_string(&p->content_disposition, 0);
			rfc2045_freeattr(p->content_disposition_attr);
			p->content_disposition_attr=0;
			if (p->boundary)
			{
				free(p->boundary);
				p->boundary=0;
			}
		}

		/* Normally, if we don't have a content_type, default it
		** to text/plain.  However, if the multipart type is
		** multipart/digest, it is message/rfc822.
		*/

		if (RFC2045_ISMIME1(p->mime_version) && !p->content_type)
		{
		char	*q="text/plain";

			if (p->parent && p->parent->content_type &&
				strcmp(p->parent->content_type,
					"multipart/digest") == 0)
				q="message/rfc822";
			set_string(&p->content_type, q);
		}

		/* If this is not a multipart section, we don't want to
		** hear about any boundaries
		*/

		if (!p->content_type ||
			strncmp(p->content_type, "multipart/", 10))
		{
			if (p->boundary)
				free(p->boundary);
			p->boundary=0;
		}

		/* If this section's a message, we will expect to see
		** more RFC2045 stuff, so create a nested RFC2045 structure,
		** and indicate that we expect to see headers.
		*/

		if (p->content_type &&
			strcmp(p->content_type, "message/rfc822") == 0)
		{
			newp=append_part_noinherit(p, p->startbody);
			newp->workinheader=1;
			return;
		}

		/*
		** If this is a multipart message (boundary defined),
		** create a RFC2045 structure for the pseudo-section
		** that precedes the first boundary line.
		*/

		if (ContentBoundary(p))
		{
			newp=append_part(p, p->startbody);
			newp->workinheader=0;
			newp->isdummy=1;
				/* It's easier just to create it. */
			return;
		}

		if (rwp)
			(*rwp->start_section)(p);
		return;
	}

	/* RFC822 header continues */

	update_counts(p, p->endpos + cnt, p->endpos+n, 1);

	/* If this header line starts with a space, append one space
	** to the saved contents of the previous line, and append this
	** line to it.
	*/

	if (isspace((int)(unsigned char)*c))
	{
		rfc2045_add_buf(&p->header, &p->headersize, &p->headerlen, " ", 1);
	}
	else
	{
	/* Otherwise the previous header line is complete, so process it */

		do_header(p);
		p->headerlen=0;
	}

	/* Save this line in the header buffer, because the next line
	** could be a continuation.
	*/

	rfc2045_add_buf( &p->header, &p->headersize, &p->headerlen, c, n);
}

/***********************************************************************/

/*
** paste_tokens() - recombine an array of RFC822 tokens back as a string.
** (Comments) are ignored.
*/

static char *paste_tokens(struct rfc822t *h, int start, int cnt)
{
int	l;
int	i;
char	*p;

	/* Calculate string size */

	l=1;
	for (i=0; i<cnt; i++)
	{
		if (h->tokens[start+i].token == '(')
			continue;

		if (rfc822_is_atom(h->tokens[start+i].token))
			l += h->tokens[start+i].len;
		else
			l++;
	}

	/* Do it */

	p=( char *)malloc(l);
	if (!p)
	{
		rfc2045_enomem();
		return (0);
	}
	l=0;

	for (i=0; i<cnt; i++)
	{
		if (h->tokens[start+i].token == '(')
			continue;

		if (rfc822_is_atom(h->tokens[start+i].token))
		{
		int l2=h->tokens[start+i].len;

			memcpy(p+l, h->tokens[start+i].ptr, l2);
			l += l2;
		}
		else	p[l++]=h->tokens[start+i].token;
	}
	p[l]=0;
	return (p);
}

/* Various permutations of the above, including forcing the string to
** lowercase
*/

static char *lower_paste_tokens(struct rfc822t *h, int start, int cnt)
{
char	*p=paste_tokens(h, start, cnt);
char	*q;

	for (q=p; q && *q; q++)
		*q=tolower(*q);
	return (p);
}

static char *paste_token(struct rfc822t *h, int i)
{
	if (i >= h->ntokens)	return (0);
	return (paste_tokens(h, i, 1));
}

static char *lower_paste_token(struct rfc822t *h, int i)
{
char *p=paste_token(h, i);
char *q;

	for (q=p; q && *q; q++)
		*q=tolower(*q);
	return (p);
}

/*
	do_header() - process completed RFC822 header.
*/

static void mime_version(struct rfc2045 *, struct rfc822t *);
static void content_type(struct rfc2045 *, struct rfc822t *);
static void content_transfer_encoding(struct rfc2045 *, struct rfc822t *);
static void content_disposition(struct rfc2045 *, struct rfc822t *);
static void content_id(struct rfc2045 *, struct rfc822t *);
static void content_description(struct rfc2045 *, const char *);
static void content_language(struct rfc2045 *, const char *);
static void content_md5(struct rfc2045 *, const char *);
static void content_base(struct rfc2045 *, struct rfc822t *);
static void content_location(struct rfc2045 *, struct rfc822t *);

static void do_header(struct rfc2045 *p)
{
struct rfc822t *header;
char	*t;

	if (p->headerlen == 0)	return;
	rfc2045_add_buf( &p->header, &p->headersize, &p->headerlen, "", 1);
				/* 0 terminate */

	/* Parse the header line according to RFC822 */

	header=rfc822t_alloc_new(p->header, NULL, NULL);

	if (!header)	return;	/* Broken header */

	if (header->ntokens < 2 ||
		header->tokens[0].token ||
		header->tokens[1].token != ':')
	{
		rfc822t_free(header);
		return;	/* Broken header */
	}

	t=lower_paste_token(header, 0);

	if (t == 0)
		;
	else if (strcmp(t, "mime-version") == 0)
	{
		free(t);
		mime_version(p, header);
	}
	else if (strcmp(t, "content-type") == 0)
	{
		free(t);
		content_type(p, header);
	} else if (strcmp(t, "content-transfer-encoding") == 0)
	{
		free(t);
		content_transfer_encoding(p, header);
	} else if (strcmp(t, "content-disposition") == 0)
	{
		free(t);
		content_disposition(p, header);
	} else if (strcmp(t, "content-id") == 0)
	{
		free(t);
		content_id(p, header);
	} else if (strcmp(t, "content-description") == 0)
	{
		free(t);
		t=strchr(p->header, ':');
		if (t)	++t;
		while (t && isspace((int)(unsigned char)*t))
			++t;
		content_description(p, t);
	} else if (strcmp(t, "content-language") == 0)
	{
		free(t);
		t=strchr(p->header, ':');
		if (t)	++t;
		while (t && isspace((int)(unsigned char)*t))
			++t;
		content_language(p, t);
	} else if (strcmp(t, "content-base") == 0)
	{
		free(t);
		content_base(p, header);
	} else if (strcmp(t, "content-location") == 0)
	{
		free(t);
		content_location(p, header);
	} else if (strcmp(t, "content-md5") == 0)
	{
		free(t);
		t=strchr(p->header, ':');
		if (t)	++t;
		while (t && isspace((int)(unsigned char)*t))
			++t;
		content_md5(p, t);
	}
	else	free(t);
	rfc822t_free(header);
}

/* Mime-Version: and Content-Transfer-Encoding: headers are easy */

static void mime_version(struct rfc2045 *p, struct rfc822t *header)
{
char	*vers=paste_tokens(header, 2, header->ntokens-2);

	if (!vers)	return;

	if (p->mime_version)	free(p->mime_version);
	p->mime_version=vers;
}

static void content_transfer_encoding(struct rfc2045 *r,
				struct rfc822t *header)
{
char	*p;

	p=lower_paste_tokens(header, 2, header->ntokens-2);
	if (!p)	return;

	if (r->content_transfer_encoding)
		free(r->content_transfer_encoding);
	r->content_transfer_encoding=p;

	if (strcmp(p, "8bit") == 0)
		r->content_8bit=1;
}

/* Dig into the content_type header */

static void parse_content_header(struct rfc822t *header,
				 int init_start,
				 void (*init_token)(char *, void *),
				 void (*init_parameter)(const char *,
							struct rfc822t *,
							int, int,
							void *),
				 void *void_arg)
{
int	start;
int	i, j;
char	*p;

	/* Look for the 1st ; */

	for (start=init_start; start < header->ntokens; start++)
		if (header->tokens[start].token == ';')
			break;

	/* Everything up to the 1st ; is the content type */

	p=lower_paste_tokens(header, init_start, start-init_start);
	if (!p)	return;

	(*init_token)(p, void_arg);
	if (start < header->ntokens) start++;

	/* Handle the remainder of the Content-Type: header */

	while (start < header->ntokens)
	{
		/* Look for next ; */

		for (i=start; i<header->ntokens; i++)
			if (header->tokens[i].token == ';')
				break;
		j=start;
		if (j < i)
		{
			++j;

			/* We only understand <atom>= */

			while (j < i && header->tokens[j].token == '(')
				++j;
			if (j < i && header->tokens[j].token == '=')
			{
				++j;

				/*
				** reformime: loose parsing due to loose
				** parsing in MSOE, leading to viruses slipping
				** through virus scanners if we strictly
				** parsed the content-type header.
				*/
				if (rfc2045_in_reformime && j < i
				    && header->tokens[j].token == '"')
					i=j+1;

				p=lower_paste_token(header, start);
				if (!p)	return;
				(*init_parameter)(p, header, j, i-j, void_arg);
				free(p);
			}
		}
		if ( i<header->ntokens ) ++i;	/* Skip over ; */
		start=i;
	}
}

/* Dig into the content_type header */

static void save_content_type(char *, void *);
static void save_content_type_parameter( const char *,
					 struct rfc822t *, int, int, void *);

static void content_type(struct rfc2045 *r, struct rfc822t *header)
{
	parse_content_header(header, 2, &save_content_type,
			     &save_content_type_parameter, r);
}

static void save_content_type(char *content_type, void *void_arg)
{
	struct rfc2045 *r=(struct rfc2045 *)void_arg;

	if (r->content_type)	free(r->content_type);
	r->content_type=content_type;
}

static void save_content_type_parameter(const char *name,
					struct rfc822t *header, int start,
					int len, void *void_arg)
{
	struct rfc2045 *r=(struct rfc2045 *)void_arg;
	char	*p;

	p=strcmp(name, "charset") == 0 ?
			lower_paste_tokens(header, start, len):
			paste_tokens(header, start, len);
	if (!p)	return;

	if (rfc2045_attrset(&r->content_type_attr, name, p) < 0)
	{
		free(p);
		rfc2045_enomem();
	}

	free(p);

	if (strcmp(name, "boundary") == 0)
	{
		struct rfc2045 *q;

		if (r->boundary)
			free(r->boundary);
		p=lower_paste_tokens(header, start, len);
		r->boundary=p;

		/*
		** Check all the outer MIME boundaries.  If this is a
		** substring of an outer MIME boundary, or the outer
		** boundary is a substring of the inner boundary, we
		** have an ambiguity - see "IMPLEMENTOR'S NOTE" in
		** section 5.1.1 of RFC 2046.
		*/

		for (q=r->parent; q; q=q->parent)
		{
			const char *a, *b;

			if (!q->boundary)
				continue;

			for (a=q->boundary, b=p; *a && *b; a++, b++)
				if (*a != *b)
					break;

			if (!*a || !*b)
			{
				while (q->parent)
					q=q->parent;
				q->rfcviolation |= RFC2045_ERRBADBOUNDARY;
				break;
			}
		}
	}
}

/* Dig into content-disposition */

static void save_content_disposition(char *, void *);
static void save_content_disposition_parameter( const char *,
						struct rfc822t *, int, int,
						void *);

static void content_disposition(struct rfc2045 *r, struct rfc822t *header)
{
	parse_content_header(header, 2, &save_content_disposition,
			     &save_content_disposition_parameter, r);
}

static void save_content_disposition(char *content_disposition, void *void_arg)
{
	struct rfc2045 *r=(struct rfc2045 *)void_arg;

	if (r->content_disposition)	free(r->content_disposition);
	r->content_disposition=content_disposition;
}

static void save_content_disposition_parameter(const char *name,
					       struct rfc822t *header,
					       int start, int len,
					       void *void_arg)
{
	struct rfc2045 *r=(struct rfc2045 *)void_arg;
	char	*p;

	p=paste_tokens(header, start, len);
	if (!p)	return;

	if (rfc2045_attrset(&r->content_disposition_attr, name, p) < 0)
	{
		free(p);
		rfc2045_enomem();
	}
	free(p);
}

char *rfc2045_related_start(const struct rfc2045 *p)
{
const char *cb=rfc2045_getattr( p->content_type_attr, "start");
struct	rfc822t *t;
struct	rfc822a	*a;
int	i;

	if (!cb || !*cb)	return (0);

	t=rfc822t_alloc_new(cb, 0, NULL);
	if (!t)
	{
		rfc2045_enomem();
		return(0);
	}

	a=rfc822a_alloc(t);
	if (!a)
	{
		rfc822t_free(t);
		rfc2045_enomem();
		return (0);
	}
	for (i=0; i<a->naddrs; i++)
		if (a->addrs[i].tokens)
		{
		char	*s=rfc822_getaddr(a, i);

			rfc822a_free(a);
			rfc822t_free(t);
			if (!s)
				rfc2045_enomem();
			return (s);
		}

	rfc822a_free(a);
	rfc822t_free(t);
	return (0);
}

static void content_id(struct rfc2045 *p, struct rfc822t *t)
{
struct	rfc822a	*a=rfc822a_alloc(t);
int	i;

	if (!a)
	{
		rfc2045_enomem();
		return;
	}

	for (i=0; i<a->naddrs; i++)
		if (a->addrs[i].tokens)
		{
		char	*s=rfc822_getaddr(a, i);

			if (!s)
			{
				rfc822a_free(a);
				rfc2045_enomem();
				return;
			}
			if (p->content_id)
				free(p->content_id);
			p->content_id=s;
			break;
		}

	rfc822a_free(a);
}

static void content_description(struct rfc2045 *p, const char *s)
{
	if (s && *s)
		set_string(&p->content_description, s);
}

static void content_language(struct rfc2045 *p, const char *s)
{
	if (s && *s)
		set_string(&p->content_language, s);
}

static void content_md5(struct rfc2045 *p, const char *s)
{
	if (s && *s)
		set_string(&p->content_md5, s);
}

static void content_base(struct rfc2045 *p, struct rfc822t *t)
{
char	*s;
int	i;

	for (i=0; i<t->ntokens; i++)
		if (t->tokens[i].token == '"')
			t->tokens[i].token=0;

	s=paste_tokens(t, 2, t->ntokens-2);
	set_string(&p->content_base, s);
}

static void content_location(struct rfc2045 *p, struct rfc822t *t)
{
char	*s;
int	i;

	for (i=0; i<t->ntokens; i++)
		if (t->tokens[i].token == '"')
			t->tokens[i].token=0;

	s=paste_tokens(t, 2, t->ntokens-2);
	set_string(&p->content_location, s);
	free(s);
}

/* -------------------- */

#define	GETINFO(s, def) ( (s) && (*s) ? (s):def)

void rfc2045_mimeinfo(const struct rfc2045 *p,
	const char **content_type_s,
	const char **content_transfer_encoding_s,
	const char **charset_s)
{
const char *c;

	*content_type_s=GETINFO(p->content_type, "text/plain");
	*content_transfer_encoding_s=GETINFO(p->content_transfer_encoding,
						"8bit");

	c=rfc2045_getattr(p->content_type_attr, "charset");
	if (!c)	c=rfc2045_getdefaultcharset();

	*charset_s=c;
}

const char *rfc2045_getdefaultcharset()
{
const char *p=rfc2045_defcharset;

	if (!p)	p=RFC2045CHARSET;
	return (p);
}

void rfc2045_setdefaultcharset(const char *charset)
{
char	*p=strdup(charset);

	if (!p)
	{
		rfc2045_enomem();
		return;
	}

	if (rfc2045_defcharset)	free(rfc2045_defcharset);
	rfc2045_defcharset=p;
}

const char *rfc2045_boundary(const struct rfc2045 *p)
{
const char *cb=rfc2045_getattr( p->content_type_attr, "boundary");

	if (!cb)	cb="";
	return (cb);
}

int rfc2045_isflowed(const struct rfc2045 *p)
{
	const char *cb=rfc2045_getattr(p->content_type_attr, "format");

	return (cb && strcmp(cb, "flowed") == 0);
}

int rfc2045_isdelsp(const struct rfc2045 *p)
{
	const char *cb=rfc2045_getattr(p->content_type_attr, "delsp");

	return (cb && strcmp(cb, "yes") == 0);
}

const char *rfc2045_content_id(const struct rfc2045 *p)
{
	return (p->content_id ? p->content_id:"");
}

const char *rfc2045_content_description(const struct rfc2045 *p)
{
	return (p->content_description ? p->content_description:"");
}

const char *rfc2045_content_language(const struct rfc2045 *p)
{
	return (p->content_language ? p->content_language:"");
}

const char *rfc2045_content_md5(const struct rfc2045 *p)
{
	return (p->content_md5 ? p->content_md5:"");
}

void rfc2045_mimepos(const struct rfc2045 *p,
	off_t *start_pos, off_t *end_pos, off_t *start_body,
	off_t *nlines, off_t *nbodylines)
{
	*start_pos=p->startpos;
	*end_pos=p->endpos;

	*nlines=p->nlines;
	*nbodylines=p->nbodylines;
	if (p->parent)	/* MIME parts do not have the trailing CRLF */
	{
		*end_pos=p->endbody;
		if (*nlines)	--*nlines;
		if (*nbodylines) --*nbodylines;
	}
	*start_body=p->startbody;

	if (*start_body == *start_pos)	/* No header */
	{
		*start_body= *end_pos;
	}
}

unsigned rfc2045_mimepartcount(const struct rfc2045 *p)
{
const struct rfc2045 *q;
unsigned n=0;

	for (q=p->firstpart; q; q=q->next)	++n;
	return (n);
}

/*
** Generic interface into parse_content_header
*/

struct rfc2045_parse_mime_info {
	void (*header_type_cb)(const char *, void *);
	void (*header_param_cb)(const char *, const char *, void *);
	void *void_arg;
};

static void parse_mime_cb(char *, void *);
static void parse_param_cb(const char *, struct rfc822t *,
			   int, int, void *);

int rfc2045_parse_mime_header(const char *header,
			      void (*header_type_cb)(const char *, void *),
			      void (*header_param_cb)(const char *,
						      const char *,
						      void *),
			      void *void_arg)
{
	struct rfc2045_parse_mime_info mi;
	struct rfc822t *h=rfc822t_alloc_new(header, NULL, NULL);

	mi.header_type_cb=header_type_cb;
	mi.header_param_cb=header_param_cb;
	mi.void_arg=void_arg;

	if (!h)
		return -1;

	parse_content_header(h, 0, parse_mime_cb, parse_param_cb, &mi);
	rfc822t_free(h);
	return 0;
}

static void parse_mime_cb(char *t, void *void_arg)
{
	struct rfc2045_parse_mime_info *mi=
		(struct rfc2045_parse_mime_info *)void_arg;

	(*mi->header_type_cb)(t, mi->void_arg);
	free(t);
}


static void parse_param_cb(const char *name,
			   struct rfc822t *header, int start,
			   int len, void *void_arg)
{
	struct rfc2045_parse_mime_info *mi=
		(struct rfc2045_parse_mime_info *)void_arg;
	char *p=paste_tokens(header, start, len);

	if (!p)
		return;

	(*mi->header_param_cb)(name, p, mi->void_arg);
	free(p);
}
