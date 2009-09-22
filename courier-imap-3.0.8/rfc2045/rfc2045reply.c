/*
** Copyright 2000-2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	"rfc2646.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045charset.h"
#include	"rfc822/rfc822.h"
#include	"unicode/unicode.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

static const char rcsid[]="$Id: rfc2045reply.c,v 1.15 2006/05/25 10:53:11 mrsam Exp $";

extern void rfc2045_enomem();

static int mkreply(struct rfc2045_mkreplyinfo *);
static int mkforward(struct rfc2045_mkreplyinfo *);
static struct rfc2045 *do_mixed_fwd(struct rfc2045 *, off_t *);

int rfc2045_makereply_do(struct rfc2045_mkreplyinfo *ri)
{
	if (strcmp(ri->replymode, "forward") == 0
	    || strcmp(ri->replymode, "forwardatt") == 0)
		return (mkforward(ri));

	return (mkreply(ri));
}

static int quotereply(const char *, size_t, void *);

static int flowreply(const char *, size_t, void *);

static void	replybody(struct rfc2045_mkreplyinfo *ri, struct rfc2045 *rfc)
{
	rfc=rfc2045_searchcontenttype(rfc, "text/plain");

	if (!rfc)
		return;

	if (rfc2045_isflowed(rfc))
	{
		struct rfc2646parser *parser;
		struct rfc2646reply *reply;

		reply=rfc2646reply_alloc(&flowreply, ri);
		if (!reply)
			rfc2045_enomem();
		parser=RFC2646REPLY_PARSEALLOC(reply);

		if (!parser)
		{
			rfc2646reply_free(reply);
			rfc2045_enomem();
		}

		(*ri->decodesectionfunc)(ri->fd, rfc, ri->charset,
					 &rfc2646_parse_cb, parser);

		rfc2646_free(parser);
		rfc2646reply_free(reply);
		return;
	}

	ri->start_line=1;
	(*ri->decodesectionfunc)(ri->fd, rfc, ri->charset,
				 &quotereply, ri);
}

static void writes(struct rfc2045_mkreplyinfo *ri, const char *c)
{
	(*ri->write_func)(c, strlen(c), ri->voidarg);
}

static int write_wrap_func(const char *c, size_t n, void *vp)
{
	struct rfc2045_mkreplyinfo *ri=(struct rfc2045_mkreplyinfo *)vp;

	(*ri->write_func)(c, n, ri->voidarg);
	return (0);
}

static int write_flowed_func(const char *c, size_t n, void *vp)
{
	struct rfc2646parser *p=(struct rfc2646parser *)vp;

	return (rfc2646_parse(p, c, n));
}

static int flowreply(const char *c, size_t l, void *voidptr)
{
	struct rfc2045_mkreplyinfo *ri=(struct rfc2045_mkreplyinfo *)voidptr;

	(*ri->write_func)(c, l, ri->voidarg);
	return (0);
}

static int quotereply(const char *p, size_t l, void *voidptr)
{
	struct rfc2045_mkreplyinfo *ri=(struct rfc2045_mkreplyinfo *)voidptr;
	size_t	i, j;

	for (i=j=0; i<l; i++)
	{
		if (p[i] == '\n')
		{
			if (ri->start_line)
				writes(ri, p[j] == '>' ? ">": "> ");
			ri->start_line=0;
			(*ri->write_func)(p+j, i-j, ri->voidarg);
			writes(ri, "\n");
			ri->start_line=1;
			j=i+1;
		}
	}

	if (j < i)
	{
		if (ri->start_line)
			writes(ri, p[j] == '>' ? ">": "> ");
		ri->start_line=0;
		(*ri->write_func)(p+j, i-j, ri->voidarg);
	}
	return (0);
}

static void forwardbody(struct rfc2045_mkreplyinfo *ri, long nbytes)
{
	char	buf[BUFSIZ];
	int	i;

	while ((i=nbytes > sizeof(buf) ? sizeof(buf):nbytes) > 0 &&
	       (i=read(ri->fd, buf, i)) > 0)
	{
		nbytes -= i;
		(*ri->write_func)(buf, i, ri->voidarg);
	}
}

static int mkforward(struct rfc2045_mkreplyinfo *ri)
{
	off_t	start_pos, end_pos, start_body;
	off_t	dummy;

	char	*header, *value;
	char	*subject=0;

	char	*boundary=0;
	struct	rfc2045 *mixed_fwd;
	off_t	orig_startpos;
	int is_flowed_fwd=0;

	struct rfc2045headerinfo *hi;
#if	HAVE_UNICODE
	const struct unicode_info *uiptr=NULL;

	if (ri->charset && *(ri->charset))
		uiptr = unicode_find(ri->charset);
#endif

	rfc2045_mimepos(ri->rfc2045partp, &start_pos, &end_pos, &start_body,
		&dummy, &dummy);

	orig_startpos=start_pos;

	hi=rfc2045header_start(ri->fd, ri->rfc2045partp);

	if (!hi)
		return (-1);

	for (;;)
	{
		if (rfc2045header_get(hi, &header, &value, 0))
		{
			rfc2045header_end(hi);
			return (-1);
		}

		if (!header)
			break;
		if (strcmp(header, "subject") == 0)
		{
			if (subject)	free(subject);
			subject=malloc(strlen(value)+10);
			if (!subject)
				return (-1);

			strcpy(subject, value);
			if (strlen(subject) > 255)
				subject[255]='\0';
		}
	}

	rfc2045header_end(hi);

	writes(ri, "Subject: ");
	if (subject)
	{
		char	*s=rfc822_coresubj_keepblobs(subject);

		if (!s)
			return (-1);

		writes(ri, s);
		free(s);
	}
	writes(ri, " (fwd)\nMime-Version: 1.0\n");

	if ((mixed_fwd=strcmp(ri->replymode, "forwardatt") == 0 ? 0:
	     do_mixed_fwd(ri->rfc2045partp, &start_pos)) != 0)
	{
		/* Borrow boundary from the message */

		boundary=strdup(rfc2045_boundary(ri->rfc2045partp));
		if (!boundary)
			return (-1);
 
		writes(ri, "Content-Type: multipart/mixed; boundary=\"");
		writes(ri, boundary);
		writes(ri, "\"\nContent-Transfer-Encoding: 8bit\n\n");
		writes(ri, RFC2045MIMEMSG);
		writes(ri, "--");
		writes(ri, boundary);
		writes(ri, "\n");

		if (rfc2045_isflowed(mixed_fwd))
			is_flowed_fwd=1;

	}
	else
		if (strcmp(ri->replymode, "forwardatt") == 0)
		{
			const char *content_type, *content_transfer_encoding,
				*charset;

			boundary=rfc2045_mk_boundary(ri->rfc2045partp,
						     ri->fd);
			if (!boundary)
				return (-1);

			rfc2045_mimeinfo(ri->rfc2045partp, &content_type,
					 &content_transfer_encoding, &charset);
			writes(ri,
			       "Content-Type: multipart/mixed; boundary=\"");
			writes(ri, boundary);
			writes(ri, "\"\nContent-Transfer-Encoding: ");
			writes(ri, content_transfer_encoding);
			writes(ri, "\n\n");
			writes(ri, RFC2045MIMEMSG);
			writes(ri, "--");
			writes(ri, boundary);
			writes(ri, "\n");
		}
		else if (rfc2045_isflowed(ri->rfc2045partp))
			is_flowed_fwd=1;

	writes(ri, "Content-Type: text/plain; charset=\"");
	writes(ri, ri->charset);
	writes(ri, "\"\n\n");
	(*ri->writesig_func)(ri->voidarg);
	writes(ri, "\n");

	if (!boundary)	/* Not forwarding as attachment */
	{
		if (ri->forwardsep)
		{
			writes(ri, ri->forwardsep);
			writes(ri, "\n");
		}

		/* Copy original headers. */
		
		hi=rfc2045header_start(ri->fd, ri->rfc2045partp);
		for (;;)
		{
			if (rfc2045header_get(hi, &header, &value,
					      RFC2045H_NOLC|RFC2045H_KEEPNL))
			{
				rfc2045header_end(hi);
				break;
			}
			if (!header)
				break;
			if (strcasecmp(header, "subject") == 0 ||
			    strcasecmp(header, "from") == 0 ||
			    strcasecmp(header, "to") == 0 ||
			    strcasecmp(header, "cc") == 0 ||
			    strcasecmp(header, "date") == 0 ||
			    strcasecmp(header, "message-id") == 0 ||
			    strcasecmp(header, "resent-from") == 0 ||
			    strcasecmp(header, "resent-to") == 0 ||
			    strcasecmp(header, "resent-cc") == 0 ||
			    strcasecmp(header, "resent-date") == 0 ||
			    strcasecmp(header, "resent-message-id") == 0)
			{
				if (subject) free(subject);
#if	HAVE_UNICODE
				if (uiptr)
					subject=rfc2047_decode_unicode(value,
						uiptr, RFC2047_DECODE_REPLACE);
				else
#endif
				subject=rfc2047_decode_enhanced(value,
								ri->charset);
				if (subject)
				{
					(*ri->write_func)(header,
							  strlen(header),
							  ri->voidarg);
					(*ri->write_func)(": ", 2, ri->voidarg);
					(*ri->write_func)(subject,
							  strlen(subject),
							  ri->voidarg);
					(*ri->write_func)("\n", 1, ri->voidarg);
				}
			}
		}
		rfc2045header_end(hi);
		(*ri->write_func)("\n", 1, ri->voidarg);

		if (lseek(ri->fd, start_body, SEEK_SET) == -1)
			return (-1);

		start_pos=start_body;

	}
	else if (mixed_fwd)
	{
		if (ri->forwardsep)
		{
			writes(ri, ri->forwardsep);
			writes(ri, "\n");
		}

		/* Copy original headers. */

		hi=rfc2045header_start(ri->fd, ri->rfc2045partp);
		for (;;)
		{
			if (rfc2045header_get(hi, &header, &value,
					      RFC2045H_NOLC|RFC2045H_KEEPNL))
			{
				rfc2045header_end(hi);
				break;
			}
			if (!header)
				break;
			if (strcasecmp(header, "subject") == 0 ||
			    strcasecmp(header, "from") == 0 ||
			    strcasecmp(header, "to") == 0 ||
			    strcasecmp(header, "cc") == 0 ||
			    strcasecmp(header, "date") == 0 ||
			    strcasecmp(header, "message-id") == 0 ||
			    strcasecmp(header, "resent-from") == 0 ||
			    strcasecmp(header, "resent-to") == 0 ||
			    strcasecmp(header, "resent-cc") == 0 ||
			    strcasecmp(header, "resent-date") == 0 ||
			    strcasecmp(header, "resent-message-id") == 0)
			{
				if (subject) free(subject);
#if	HAVE_UNICODE
				if (uiptr)
					subject=rfc2047_decode_unicode(value,
						uiptr, RFC2047_DECODE_REPLACE);
				else
#endif
				subject=rfc2047_decode_enhanced(value,
								ri->charset);
				if (subject)
				{
					(*ri->write_func)(header,
							  strlen(header),
							  ri->voidarg);
					(*ri->write_func)(": ", 2, ri->voidarg);
					(*ri->write_func)(subject,
							  strlen(subject),
							  ri->voidarg);
					(*ri->write_func)("\n", 1, ri->voidarg);
				}
			}
		}
		rfc2045header_end(hi);
		(*ri->write_func)("\n", 1, ri->voidarg);

		if (lseek(ri->fd, start_body, SEEK_SET) == -1)
			return (-1);

		/* Decode the body of the MIME section */

		if (is_flowed_fwd)
		{
			struct rfc2646fwd *p=rfc2646fwd_alloc(&write_wrap_func,
							      ri);
			struct rfc2646parser *parser;

			if (!p)
				return (-1);

			parser=RFC2646FWD_PARSEALLOC(p);

			if (!parser)
			{
				rfc2646fwd_free(p);
				return (-1);
			}

			(*ri->decodesectionfunc)(ri->fd, mixed_fwd,
						 ri->charset,
						 &write_flowed_func, parser);
			rfc2646_free(parser);
			rfc2646fwd_free(p);
			is_flowed_fwd=0;
		}
		else
			(*ri->decodesectionfunc)(ri->fd, mixed_fwd,
						 ri->charset,
						 &write_wrap_func, ri);

		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "\n");
	}
	else
	{
		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "\nContent-Type: message/rfc822\n");
		if (ri->forwarddescr)
		{
			char *p=rfc2047_encode_str(ri->forwarddescr,
						   ri->charset ?
						   ri->charset : "iso-8859-1",
						   rfc2047_qp_allow_any);

			writes(ri, "Content-Description: ");
			writes(ri, p ? p:"");
			free(p);
			writes(ri, "\n");
		}
		writes(ri, "\n");
	}

	{
		off_t save_start_body=ri->rfc2045partp->startbody;
		off_t save_end_pos=ri->rfc2045partp->endpos;
#if 0
		char *save_te=
			ri->rfc2045partp->content_transfer_encoding;
#endif

		ri->rfc2045partp->startbody=start_pos;
		ri->rfc2045partp->endpos=end_pos;
#if 0
		ri->rfc2045partp->content_transfer_encoding="8bit";
#endif
		if (is_flowed_fwd)
		{
			struct rfc2646fwd *p=rfc2646fwd_alloc(&write_wrap_func,
							      ri);
			struct rfc2646parser *parser;

			if (!p)
				return (-1);

#if 0
			ri->rfc2045partp->content_transfer_encoding=save_te;
#endif
			parser=RFC2646FWD_PARSEALLOC(p);

			if (!parser)
			{
				rfc2646fwd_free(p);
				return (-1);
			}

			(*ri->decodesectionfunc)(ri->fd, ri->rfc2045partp,
						 ri->charset,
						 &write_flowed_func, parser);
			rfc2646_free(parser);
			rfc2646fwd_free(p);
		}
		else if (strcmp(ri->replymode, "forwardatt") == 0)
		{
			if (lseek(ri->fd, start_pos, SEEK_SET) == 0)
				forwardbody(ri, end_pos - start_pos);
		}
		else	(*ri->decodesectionfunc)(ri->fd, ri->rfc2045partp,
						 ri->charset,
						 &write_wrap_func, ri);

		ri->rfc2045partp->startbody=save_start_body;
		ri->rfc2045partp->endpos=save_end_pos;
#if 0
		ri->rfc2045partp->content_transfer_encoding=save_te;
#endif
	}

	if (boundary)
	{
		if (!mixed_fwd)	/* Already copied */
		{
			writes(ri, "\n--");
			writes(ri, boundary);
			writes(ri, "--\n");
		}
		free(boundary);
	}
	return (0);
}


static int writereferences(struct rfc2045_mkreplyinfo *ri,
			    const char *oldref, const char *oldmsgid)
{
char	*buf=malloc((oldref ? strlen(oldref):0)
		+ (oldmsgid ? strlen(oldmsgid):0)+2);
char	*p, *q;
struct	rfc822t *tp;
struct	rfc822a *ap;
int	i;

	if (!buf)
		return (-1);

	/* Create new references header */
	*buf=0;
	if (oldref)	strcat(buf, oldref);
	if (oldref && oldmsgid)	strcat(buf, " ");
	if (oldmsgid)	strcat(buf, oldmsgid);

	/* Do wrapping the RIGHT way, by
	** RFC822 parsing the References: header
	*/

	if ((tp=rfc822t_alloc_new(buf, NULL, NULL)) == 0 ||
		(ap=rfc822a_alloc(tp)) == 0)
	{
		free(buf);
		if (tp)
			rfc822t_free(tp);
		return (-1);
	}

	/* Keep only the last 20 message IDs */

	i=0;
	if (ap->naddrs > 20)	i=ap->naddrs-20;
	p="";
	while (i < ap->naddrs)
	{
		q=rfc822_gettok(ap->addrs[i].tokens);
		if (!q)
		{
			rfc822a_free(ap);
			rfc822t_free(tp);
			free(buf);
			return (-1);
		}

		writes(ri, p);
		writes(ri, "<");
		writes(ri, q);
		writes(ri, ">\n");
		p="            ";
		free(q);
		i++;
	}
	rfc822a_free(ap);
	rfc822t_free(tp);
	free(buf);
	return (0);
}

static char *mlcheck(struct rfc2045_mkreplyinfo *ri, const char *);

static int mkreply(struct rfc2045_mkreplyinfo *ri)
{
	char	*oldtocc, *oldfrom, *oldreplyto, *oldtolist;
	char	*subject;
	char	*oldmsgid;
	char	*oldreferences;
	char	*header, *value;

	char	*whowrote;
	off_t	start_pos, end_pos, start_body, dummy;
	int errflag=0;
	struct rfc2045headerinfo *hi;
#if	HAVE_UNICODE
	const struct unicode_info *uiptr=NULL;

	if (ri->charset && *(ri->charset))
		uiptr = unicode_find(ri->charset);
#endif

	oldtocc=0;
	oldtolist=0;
	oldfrom=0;
	oldreplyto=0;
	subject=0;
	oldmsgid=0;
	oldreferences=0;
	whowrote=0;

	rfc2045_mimepos(ri->rfc2045partp, &start_pos, &end_pos, &start_body,
		&dummy, &dummy);

	hi=rfc2045header_start(ri->fd, ri->rfc2045partp);

	if (!hi)
		return (-1);

#define BLOWUP { \
		if (whowrote) free(whowrote); \
		if (subject) free(subject); \
		if (oldmsgid) free(oldmsgid); \
		if (oldreferences)	free(oldreferences); \
		if (oldtocc) free(oldtocc); \
		if (oldtolist) free(oldtolist); \
		if (oldfrom) free(oldfrom); \
		if (oldreplyto)	free(oldreplyto); \
		rfc2045header_end(hi); \
		return (-1); \
	}

	for (;;)
	{
		if (rfc2045header_get(hi, &header, &value, 0))
		{
			BLOWUP;
			return (-1);
		}

		if (!header)
			break;

		if (strcmp(header, "subject") == 0)
		{
			if (subject)	free(subject);
			subject=strdup(value);
			if (!subject)
				BLOWUP;
		}
		else if (strcmp(header, "reply-to") == 0)
		{
			if (oldreplyto)	free(oldreplyto);
			oldreplyto=strdup(value);
			if (!oldreplyto)
				BLOWUP;
		}
		else if (strcmp(header, "from") == 0)
		{
			if (oldfrom)	free(oldfrom);
			oldfrom=strdup(value);
			if (!oldfrom)
				BLOWUP;
		}
		else if (strcmp(header, "message-id") == 0)
		{
			if (oldmsgid)	free(oldmsgid);
			oldmsgid=strdup(value);
			if (!oldmsgid)
				BLOWUP;
		}
		else if (strcmp(header, "references") == 0)
		{
			if (oldreferences)	free(oldreferences);
			oldreferences=strdup(value);
			if (!oldreferences)
				BLOWUP;
		}
		else if ((strcmp(ri->replymode, "replyall") == 0
			  || strcmp(ri->replymode, "replylist") == 0) &&
			 (
			  strcmp(header, "to") == 0 ||
			  strcmp(header, "cc") == 0
			  )
			 )
		{
			char	*newh=malloc( (oldtocc ?
					       strlen(oldtocc):0)
					      + strlen(value)+2);
			char	*p;

				if (!newh)
					BLOWUP;

				*newh=0;
				if (oldtocc)
					strcat(strcpy(newh, oldtocc),
								",");
				strcat(newh, value);
				if (oldtocc)	free(oldtocc);
				oldtocc=newh;

				p=mlcheck(ri, value);
				if (!p || (newh=malloc((oldtolist ?
						       strlen(oldtolist):0)
					   + strlen(p)+2)) == NULL)
				{
					if (p)
						free(p);
					BLOWUP;
				}

				if (*p)
				{
					*newh=0;
					if (oldtolist)
						strcat(strcpy(newh, oldtolist),
						       ",");
					strcat(newh, p);
					if (oldtolist)
						free(oldtolist);
					oldtolist=newh;
				}
				free(p);
		}
	}

	rfc2045header_end(hi);

	/* Write:  "%s writes:" */

	if (oldfrom)
	{
		struct	rfc822t *rfcp=rfc822t_alloc_new(oldfrom, NULL, NULL);
		struct	rfc822a *rfcpa;
		char	*p, *q;

		if (!rfcp)
			BLOWUP;

		rfcpa=rfc822a_alloc(rfcp);
		if (!rfcpa)
		{
			rfc822t_free(rfcp);
			BLOWUP;
		}

		p= rfcpa->naddrs > 0 ?
			rfc822_getname(rfcpa, 0):0;
		rfc822a_free(rfcpa);
		rfc822t_free(rfcp);
		if (!p)	p=strdup(oldfrom);
		if (!p)
			BLOWUP;

#if	HAVE_UNICODE
		if (uiptr)
			q=rfc2047_decode_unicode(p, uiptr,
				RFC2047_DECODE_REPLACE);
		else
#endif
		q=rfc2047_decode_enhanced(p, ri->charset);
		if (!q)
		{
			free(p);
			BLOWUP;
		}

		free(p);
		p=q;
		whowrote=malloc(strlen(p)+strlen(ri->replysalut)+1);
		if (!whowrote)
		{
			free(p);
			BLOWUP;
		}

		sprintf(whowrote, ri->replysalut, p);
		free(p);
	}

	if (oldreplyto)
	{
		if (oldfrom)	free(oldfrom);
		oldfrom=oldreplyto;
		oldreplyto=0;
	}

	/*
	** Replytolist: if we found mailing list addresses, drop
	** oldtocc, we'll use oldtolist.
	** Otherwise, drop oldtolist.
	*/

	if (strcmp(ri->replymode, "replylist") == 0)
	{
		if (oldtolist)
		{
			if (oldtocc)
			{
				free(oldtocc);
				oldtocc=0;
			}

			if (oldfrom)
			{
				free(oldfrom);
				oldfrom=0;
			}
		}
	}
	else
	{
		if (oldtolist)
		{
			free(oldtolist);
			oldtolist=NULL;
		}
	}

	/* Remove duplicate entries from new Cc header */

	if (oldtocc)
	{
		struct rfc822t	*rfccc, *rfcto;
		struct rfc822a	*arfccc, *arfcto;
		int	i, j;
		char	*new_addresses;

		rfccc=rfc822t_alloc_new(oldtocc, NULL, NULL);
		rfcto= oldfrom ? rfc822t_alloc_new(oldfrom, NULL,
						   NULL):NULL;
		arfccc=rfccc ? rfc822a_alloc(rfccc):NULL;
		arfcto=rfcto ? rfc822a_alloc(rfcto):NULL;

		for (i=0; arfccc && i <arfccc->naddrs; i++)
		{
			char	*addr=rfc822_getaddr(arfccc, i);

			if (!addr)	continue;

				/* Remove address from Cc if it is my address */

			if ( (ri->myaddr_func)(addr, ri->voidarg))
			{
				rfc822_deladdr(arfccc, i); --i;
				free(addr);
				continue;
			}

				/* Remove address from Cc if it appears in To: */

			for (j=0; arfcto && j < arfcto->naddrs; j++)
			{
				char *addr2=rfc822_getaddr(arfcto, j);

				if (!addr2)	continue;
				if (strcmp(addr, addr2) == 0)
				{
					free(addr2);
					break;
				}
				free(addr2);
			}
			if (arfcto && j < arfcto->naddrs)
			{
				rfc822_deladdr(arfccc, i); --i;
				free(addr);
				continue;
			}

				/* Remove outright duplicates in Cc */

			for (j=i+1; j<arfccc->naddrs; j++)
			{
				char *addr2=rfc822_getaddr(arfccc, j);

				if (!addr2)	continue;
				if (strcmp(addr, addr2) == 0)
				{
					rfc822_deladdr(arfccc, j);
					--j;
				}
				free(addr2);
			}
			free(addr);
		}
		new_addresses=rfc822_getaddrs(arfccc);
		free(oldtocc);
		oldtocc=new_addresses;
		if (arfccc)	rfc822a_free(arfccc);
		if (arfcto)	rfc822a_free(arfcto);
		rfc822t_free(rfccc);
		if (rfcto) rfc822t_free(rfcto);
	}

	if (oldtolist)
	{
		writes(ri, "To: ");
		writes(ri, oldtolist);
		writes(ri, "\n");
		free(oldtolist);
	}

	if (oldfrom)
	{
		writes(ri, "To: ");
		writes(ri, oldfrom);
		writes(ri, "\n");
		free(oldfrom);
	}

	if (oldtocc)
	{
		writes(ri, "Cc: ");
		writes(ri, oldtocc);
		writes(ri, "\n");
		free(oldtocc);
	}

	if (oldmsgid || oldreferences)
	{
		writes(ri, "References: ");
		if (writereferences(ri, oldreferences, oldmsgid))
			errflag= -1;
		if (oldreferences)	free(oldreferences);
	}
	if (oldmsgid)
	{
		writes(ri, "In-Reply-To: ");
		writes(ri, oldmsgid);
		writes(ri, "\n");
		free(oldmsgid);
	}
	writes(ri,"Subject: Re: ");
	if (subject)
	{
		char	*s=rfc822_coresubj_keepblobs(subject);

		writes(ri, s ? s:"");
		if (s)	free(s);
		free(subject);
	}

	writes(ri, "\nMime-Version: 1.0\n");
	writes(ri, "Content-Type: text/plain; charset=\"");
	writes(ri, ri->charset);
	writes(ri, "\"\n");
	writes(ri, "\n");
	if (whowrote)
	{
		writes(ri, whowrote);
		free(whowrote);
		writes(ri, "\n\n");
	}
	if (lseek(ri->fd, start_body, SEEK_SET) == -1)
		return (-1);

	replybody(ri, ri->rfc2045partp);
	writes(ri, "\n");	/* First blank line in the reply */
	(*ri->writesig_func)(ri->voidarg);
	return (errflag);
}

/*****************************************************************************
**
** do_mixed_fwd determines if what we should create for forwarding should be
** a mixture of a portion of the original message, plus attachments.  This
** should happen when the original message has attachments.  Normally, we
** either forward the entire message as text/plain, or as a message/rfc822
** attachment.  When the original message has attachments, it's preferrable
** to keep the original attachments as attachments of the forward message,
** while putting the text portion of the original message in the forwarded
** body.
**
** So, what we check here is this:
**
** A) This is a multipart message with at least two parts.
** B) The first part is text/plain (if it's text/html we'll just quote the
**    whole mess too).  If the first part is multipart/alternative, look
**    inside the multipart/alternative for the text/plain section.
**
** If this is not the case, we return NULL.  Otherwise we return the
** rfc2045 pointer to the text/plain section, and the starting position
** of the first attachment (it's used to reset the starting position of
** the portion of the original message that's copied into the forwarding
** message).
*/

static struct rfc2045 *do_mixed_fwd(struct rfc2045 *p, off_t *f)
{
const char *content_type, *dummy;
off_t	dummyp;

	if (!p->firstpart || !p->firstpart->isdummy ||
		!p->firstpart->next || !p->firstpart->next->next)
		return (0);

	rfc2045_mimepos(p->firstpart->next->next, f, &dummyp, &dummyp,
		&dummyp, &dummyp);

	p=p->firstpart->next;

	rfc2045_mimeinfo(p, &content_type, &dummy, &dummy);

	if (strcmp(content_type, "text/plain") &&
		strcmp(content_type, "text/html") /* GRUMBLE */)
	{
		if (strcmp(content_type, "multipart/alternative"))
			return (0);

		for (p=p->firstpart; p; p=p->next)
		{
			if (p->isdummy)	continue;

			rfc2045_mimeinfo(p, &content_type, &dummy, &dummy);
			if (strcmp(content_type, "text/plain") == 0)
				break;
		}
		if (!p)	return (0);
	}
	return (p);
}

/*
** Accept a list of recipients, and return a list that contains only those
** recipients that are defined as mailing lists.
*/

static char *do_checkmailinglists(struct rfc822a *a,
				  struct rfc2045_mkreplyinfo *,
				  char *);

static char *mlcheck(struct rfc2045_mkreplyinfo *ri, const char *hdr)
{
	struct rfc822t *t;
	struct rfc822a *a;

	char *mlcopy;
	char *p;

	t=rfc822t_alloc_new(hdr, NULL, NULL);

	if (!t)
		return (0);

	a=rfc822a_alloc(t);

	if (!a)
	{
		rfc822t_free(t);
		return (0);
	}

	mlcopy=strdup(ri->mailinglists ? ri->mailinglists:"");
	if (!mlcopy)
		p=0;
	else
	{
		p=do_checkmailinglists(a, ri, mlcopy);
		free(mlcopy);
	}

	rfc822a_free(a);
	rfc822t_free(t);
	return (p);
}

static char *do_checkmailinglists(struct rfc822a *a,
				  struct rfc2045_mkreplyinfo *ri,
				  char *mlbuffer)
{
	int i;

	for (i=0; i<a->naddrs; i++)
	{
		char *p=rfc822_getaddr(a, i);
		char *q;

		if (!p)
			return (NULL);

		strcpy(mlbuffer, ri->mailinglists ? ri->mailinglists:"");

		for (q=mlbuffer; (q=strtok(q, "\n \t")) != NULL; q=NULL)
		{
			if (strcasecmp(p, q) == 0)
				break;
		}

		free(p);
		if (q == NULL)
		{
			rfc822_deladdr(a, i);
			--i;
		}
	}

	if (a->naddrs == 0)
		return (strdup(""));
	return (rfc822_getaddrs(a));
}
