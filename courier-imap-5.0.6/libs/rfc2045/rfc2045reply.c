/*
** Copyright 2000-2018 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	"rfc2045src.h"
#include	"rfc3676parser.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045charset.h"
#include	"rfc822/rfc822.h"
#include	"unicode/courier-unicode.h"
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>


extern void rfc2045_enomem();

static int mkreply(struct rfc2045_mkreplyinfo *);
static int mkforward(struct rfc2045_mkreplyinfo *);

static void mksalutation_datefmt(const char *fmt_start,
				 const char *fmt_end,
				 const char *date,
				 void (*callback_func)(const char *,
						       size_t, void *),
				 void *callback_arg)
{
	time_t t;

	if (!fmt_start)
	{
		fmt_start="%a, %d %b %Y %H:%M:%S %z";
		fmt_end=fmt_start + strlen(fmt_start);
	}

	if (rfc822_parsedate_chk(date, &t) == 0)
	{
		struct tm tmbuf;

		if (localtime_r(&t, &tmbuf))
		{
			char *fmtstr=malloc(fmt_end-fmt_start + 1);

			if (fmtstr)
			{
				char fmtbuf[1024];

				memcpy(fmtstr, fmt_start, fmt_end - fmt_start);
				fmtstr[fmt_end-fmt_start]=0;

				fmtbuf[strftime(fmtbuf,
						sizeof(fmtbuf)-1,
						fmtstr, &tmbuf)]=0;

				free(fmtstr);
				(*callback_func)(fmtbuf, strlen(fmtbuf),
						 callback_arg);
				return;
			}
		}
	}
	(*callback_func)(date, strlen(date), callback_arg);
}

static void mksalutation_cb(const char *salutation_template,
			    const char *newsgroup,
			    const char *message_id,
			    const char *newsgroups,
			    const char *sender_addr,
			    const char *sender_name,
			    const char *date,
			    const char *subject,

			    void (*callback_func)(const char *, size_t, void *),
			    void *callback_arg)
{
	const char *p;

	for (p=salutation_template; *p; p++)
	{
		const char *fmt_start=0, *fmt_end=0;

		if (*p != '%' || p[1] == '\0')
		{
			(*callback_func)( p, 1, callback_arg );
			continue;
		}

		++p;

		if (*p == '{')
		{
			fmt_start= ++p;

			while (*p)
			{
				if (*p == '}')
				{
					fmt_end=p;
					++p;
					break;
				}
			}

			if (!fmt_end)
				continue;
		}

#define CBSTR(s) s, strlen(s), callback_arg

		switch (*p)	{
		case 'n':
			(*callback_func)("\n", 1, callback_arg );
			continue;
		case 'C':
			(*callback_func)(CBSTR(newsgroup));
			break;
		case 'i':
			(*callback_func)(CBSTR(message_id));
			break;
		case 'N':
			(*callback_func)(CBSTR(newsgroups));
			break;
		case 'f':
			(*callback_func)(CBSTR(sender_addr));
			break;
		case 'F':
			(*callback_func)(CBSTR(sender_name));
			break;
		case 'd':
			if (date)
				mksalutation_datefmt(fmt_start,
						     fmt_end,
						     date,
						     callback_func,
						     callback_arg);
			break;
		case 's':
			(*callback_func)(CBSTR(subject));
			break;
		default:
			(*callback_func)(p, 1, callback_arg);
			break;
		}
#undef CBSTR

	}
}

static void mksal_count(const char *str,
			size_t cnt,
			void *arg)
{
	*(size_t *)arg += cnt;
}

static void mksal_save(const char *str,
		       size_t cnt,
		       void *arg)
{
	if (cnt)
		memcpy(*(char **)arg, str, cnt);

	*(char **)arg += cnt;
}

static char *mksalutation(const char *salutation_template,
			  const char *newsgroup,
			  const char *message_id,
			  const char *newsgroups,
			  const char *sender_addr,
			  const char *sender_name,
			  const char *date,
			  const char *subject,
			  const char *charset)
{
	size_t cnt;
	char *p, *q;

	char *subj_decoded=rfc822_display_hdrvalue_tobuf("subject", subject,
							 charset, NULL, NULL);

	if (!subj_decoded)
		return NULL;

	cnt=1;

	mksalutation_cb(salutation_template,
			newsgroup,
			message_id,
			newsgroups,
			sender_addr,
			sender_name,
			date,
			subj_decoded,
			mksal_count, &cnt);

	p=q=malloc(cnt);

	if (!p)
	{
		free(subj_decoded);
		return NULL;
	}

	mksalutation_cb(salutation_template,
			newsgroup,
			message_id,
			newsgroups,
			sender_addr,
			sender_name,
			date,
			subj_decoded,
			mksal_save, &q);
	*q=0;

	free(subj_decoded);
	return p;
}


int rfc2045_makereply(struct rfc2045_mkreplyinfo *ri)
{
	if (strcmp(ri->replymode, "forward") == 0
	    || strcmp(ri->replymode, "forwardatt") == 0)
		return (mkforward(ri));

	return (mkreply(ri));
}

struct replyinfostruct {

	struct rfc2045_mkreplyinfo *ri;
	rfc3676_parser_t parser;

	size_t quote_level_adjust;
	size_t quote_level;
	int start_line;
	int isflowed;
	size_t trailing_spaces;
	unicode_convert_handle_t u_handle;

};

/*
** Pass original content to the RFC 3676 parser
*/

static int quotereply(const char *p, size_t l, void *voidptr)
{
	struct replyinfostruct *ris=(struct replyinfostruct *)voidptr;

	return rfc3676parser(ris->parser, p, l);
}

/*
** Push formatted reply downstream.
*/

static int output_reply(const char *ptr, size_t cnt, void *arg)
{
	struct replyinfostruct *s=(struct replyinfostruct *)arg;

	(*s->ri->write_func)(ptr, cnt, s->ri->voidarg);
	return 0;
}

/*
** RFC 3676 parser: Start of a new line in the reply.
*/
static int reply_begin(size_t quote_level,
		       void *arg)
{
	struct replyinfostruct *s=(struct replyinfostruct *)arg;
	char32_t quoteChar='>';

	/*
	** Save quote level, begin conversion from unicode to the native
	** charset.
	*/
	s->quote_level=quote_level+s->quote_level_adjust;

	s->u_handle=unicode_convert_init(unicode_u_ucs4_native,
					   s->ri->charset,
					   output_reply,
					   s);

	/*
	** Emit quoting indentation, if any.
	*/
	s->start_line=1;
	s->trailing_spaces=0;

	if (s->u_handle)
	{
		size_t i;

		for (i=0; i<s->quote_level; i++)
		{
			unicode_convert_uc(s->u_handle, &quoteChar, 1);
		}
	}
	return 0;
}

/*
** RFC 3676: (possibly partial) contents of a deflowed line, as unicode.
*/

static int reply_contents(const char32_t *txt,
			  size_t txt_size,
			  void *arg)
{
	char32_t spaceChar=' ';
	size_t nonspc_cnt;

	struct replyinfostruct *s=(struct replyinfostruct *)arg;

	if (!s->u_handle || txt_size == 0)
		return 0;

	/*
	** Space-stuff the initial character.
	*/

	if (s->start_line)
	{
		if (!s->isflowed)
		{
			/*
			** If the original content is not flowed, the rfc3676
			** parser does not parse the number of > quote
			** characters and does not set the quote level.
			*/

			if ((s->quote_level > 0 && *txt != '>') || *txt == ' ')
				unicode_convert_uc(s->u_handle,
						     &spaceChar, 1);

		}
		else
		{
			if (s->quote_level > 0 || *txt == ' ' || *txt == '>')
				unicode_convert_uc(s->u_handle,
						     &spaceChar, 1);
		}
		s->start_line=0;
	}

	/*
	** Trim any trailing spaces from the RFC 3676 parsed content.
	*/

	for (nonspc_cnt=txt_size; nonspc_cnt > 0; --nonspc_cnt)
		if (txt[nonspc_cnt-1] != ' ')
			break;

	/*
	** If the contents are not totally whitespace, it's ok now to emit
	** any accumulated whitespace from previous content.
	*/

	if (nonspc_cnt)
	{
		while (s->trailing_spaces)
		{
			unicode_convert_uc(s->u_handle, &spaceChar, 1);
			--s->trailing_spaces;
		}

		unicode_convert_uc(s->u_handle, txt, nonspc_cnt);
	}

	s->trailing_spaces += txt_size - nonspc_cnt;
	return 0;
}

static int reply_end(void *arg)
{
	char32_t newLine='\n';
	struct replyinfostruct *s=(struct replyinfostruct *)arg;

	unicode_convert_uc(s->u_handle, &newLine, 1);

	unicode_convert_deinit(s->u_handle, NULL);
	return 0;
}

/*
** RFC 3676 parser: flowed line break. Replicate it.
*/
static int reply_wrap(void *arg)
{
	char32_t spaceChar=' ';
	struct replyinfostruct *s=(struct replyinfostruct *)arg;

	/*
	** It's safe to preserve trailing spaces on flowed lines.
	*/

	while (s->trailing_spaces)
	{
		unicode_convert_uc(s->u_handle, &spaceChar, 1);
		--s->trailing_spaces;
	}

	unicode_convert_uc(s->u_handle, &spaceChar, 1);
	reply_end(s);
	reply_begin(s->quote_level-s->quote_level_adjust, s);
	/* Undo the adjustment in reply_begin() */

	return 0;
}

static void reformat(struct rfc2045_mkreplyinfo *ri, struct rfc2045 *rfc,
		     size_t adjustLevel)
{
	struct replyinfostruct ris;

	struct rfc3676_parser_info info;
	int conv_err;

	ris.ri=ri;
	ris.quote_level_adjust=adjustLevel;

	memset(&info, 0, sizeof(info));

	info.charset=ri->charset;
	ris.isflowed=info.isflowed=rfc2045_isflowed(rfc);
	info.isdelsp=rfc2045_isdelsp(rfc);

	info.line_begin=reply_begin;
	info.line_contents=reply_contents;
	info.line_flowed_notify=reply_wrap;
	info.line_end=reply_end;
	info.arg=&ris;

	if ((ris.parser=rfc3676parser_init(&info)) != NULL)
	{
		rfc2045_decodetextmimesection(ri->src, rfc,
					      ri->charset,
					      &conv_err,
					      quotereply,
					      &ris);
		rfc3676parser_deinit(ris.parser, NULL);
	}
}

static void	replybody(struct rfc2045_mkreplyinfo *ri, struct rfc2045 *rfc)
{
	rfc=rfc2045_searchcontenttype(rfc, "text/plain");

	if (!rfc)
		return;

	reformat(ri, rfc, 1);
}

static void writes(struct rfc2045_mkreplyinfo *ri, const char *c)
{
	(*ri->write_func)(c, strlen(c), ri->voidarg);
}

static void forwardbody(struct rfc2045_mkreplyinfo *ri, long nbytes)
{
	char	buf[BUFSIZ];
	ssize_t i;

	while ((i=nbytes > sizeof(buf) ? sizeof(buf):nbytes) > 0 &&
	       (i=SRC_READ(ri->src, buf, i)) > 0)
	{
		nbytes -= i;
		(*ri->write_func)(buf, i, ri->voidarg);
	}
}

/*
** Format a forward
*/
static int mkforward(struct rfc2045_mkreplyinfo *ri)
{
	off_t	start_pos, end_pos, start_body;
	off_t	dummy;

	char	*header, *value;
	char	*subject=0;

	char	*boundary=0;

	struct rfc2045headerinfo *hi;

	struct	rfc2045 *textplain_content;
	struct	rfc2045 *first_attachment;
	int attachment_is_message_rfc822;

	/*
	** Use the original message's subject to set the subject of the
	** forward message.
	*/

	hi=rfc2045header_start(ri->src, ri->rfc2045partp);

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

			subject=strdup(value);
			if (!subject)
			{
				rfc2045header_end(hi);
				return (-1);
			}
		}
	}

	rfc2045header_end(hi);

	writes(ri, "Subject: ");

	if (ri->subject)
	{
		/*
		** ... unless the caller overrides it.
		*/

		writes(ri, ri->subject);
	}
	else if (subject)
	{
		char	*s=rfc822_coresubj_keepblobs(subject);

		if (!s)
			return (-1);

		writes(ri, s);
		free(s);
		writes(ri, " (fwd)");
	}
	writes(ri, "\nMime-Version: 1.0\n");

	/*
	** To assemble a forward template, two things are needed:
	**
	** 1. The original message, as text/plain.
	**
	** 2. Any attachments in the original message.
	**    A. The attachments get either copied to the forward message, or
	**    B. The original message is attached as a single message/rfc822
	**       entity.
	**
	** 2b is always produced by "forwardatt". If a suitable text/plain
	** part of the original message could not be found, 2b is also
	** produced even by "forward".
	*/

	textplain_content=NULL;

	attachment_is_message_rfc822=0;
	first_attachment=NULL;

	{
		const char *content_type, *dummy;

		struct rfc2045 *top_part=ri->rfc2045partp;

		rfc2045_mimeinfo(top_part,
				 &content_type, &dummy, &dummy);

		if (strcmp(content_type, "multipart/signed") == 0)
		{
			struct rfc2045 *p=top_part->firstpart;

			if (p && p->isdummy)
				p=p->next;

			if (p)
			{
				top_part=p;
				rfc2045_mimeinfo(top_part,
						 &content_type, &dummy, &dummy);
			}
		}
		else if (strcmp(content_type, "multipart/x-mimegpg") == 0)
		{
			struct rfc2045 *p=top_part->firstpart;

			if (p && p->isdummy)
				p=p->next;

			if (p)
			{
				const char *part_ct;

				rfc2045_mimeinfo(p,
						 &part_ct, &dummy, &dummy);

				if (strcmp(part_ct, "text/x-gpg-output") == 0
				    && p->next)
				{
					top_part=p->next;
					rfc2045_mimeinfo(top_part,
							 &content_type,
							 &dummy, &dummy);
				}
			}
		}

		if (strcmp(content_type, "text/plain") == 0)
		{
			textplain_content=top_part;
		}
		else if (strcmp(content_type, "multipart/alternative") == 0)
		{
			textplain_content=
				rfc2045_searchcontenttype(top_part,
							  "text/plain");
		}
		else if (strcmp(content_type, "multipart/mixed") == 0)
		{
			struct rfc2045 *p=top_part->firstpart;

			if (p->isdummy)
				p=p->next;

			textplain_content=
				rfc2045_searchcontenttype(p, "text/plain");

			/*
			** If the first part contained a suitable text/plain,
			** any remaining MIME parts become attachments that
			** get copied to the forward message.
			*/
			if (textplain_content)
				first_attachment=p->next;
		}

		if (strcmp(ri->replymode, "forwardatt") == 0 ||
		    textplain_content == NULL)
		{
			/*
			** Copy the entire message as the sole message/rfc822
			** attachment in the forward message.
			*/
			textplain_content=NULL;
			first_attachment=top_part;
			attachment_is_message_rfc822=1;
		}
	}

	boundary=strdup(rfc2045_mk_boundary(ri->rfc2045partp, ri->src));
	if (!boundary)
	{
		if (subject)	free(subject);
		return (-1);
	}

	if (first_attachment)
	{
		writes(ri, "Content-Type: multipart/mixed; boundary=\"");
		writes(ri, boundary);
		writes(ri, "\"\n\n");
		writes(ri, RFC2045MIMEMSG);
		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "\n");
	}

	if (ri->content_set_charset)
	{
		(*ri->content_set_charset)(ri->voidarg);
	}
	else
	{
		writes(ri, "Content-Type: text/plain; format=flowed; delsp=yes; charset=\"");
		writes(ri, ri->charset);
		writes(ri, "\"\n");
	}

	writes(ri, "Content-Transfer-Encoding: 8bit\n\n");
	if (ri->content_specify)
		(*ri->content_specify)(ri->voidarg);

	writes(ri, "\n");
	if (ri->writesig_func)
		(*ri->writesig_func)(ri->voidarg);
	writes(ri, "\n");

	if (ri->forwardsep)
	{
		writes(ri, ri->forwardsep);
		writes(ri, "\n");
	}

	if (textplain_content)
	{
		/* Copy original headers. */

		hi=rfc2045header_start(ri->src, ri->rfc2045partp);
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

				subject=rfc822_display_hdrvalue_tobuf(header,
								      value,
								      ri->charset,
								      NULL,
								      NULL);

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

		reformat(ri, textplain_content, 0);
	}

	if (first_attachment)
	{
		/*
		** There are attachments to copy
		*/

		if (attachment_is_message_rfc822)
		{
			/* Copy everything as a message/rfc822 */

			writes(ri, "\n--");
			writes(ri, boundary);
			writes(ri, "\nContent-Type: ");

			writes(ri, first_attachment->rfcviolation &
			       RFC2045_ERR8BITHEADER
			       ? RFC2045_MIME_MESSAGE_GLOBAL
			       : RFC2045_MIME_MESSAGE_RFC822);
			writes(ri, "\n");

			if (ri->forwarddescr)
			{
				char *p=rfc2047_encode_str(ri->forwarddescr,
							   ri->charset ?
							   ri->charset
							   : "utf-8",
							   rfc2047_qp_allow_any
							   );

				writes(ri, "Content-Description: ");
				writes(ri, p ? p:"");
				free(p);
				writes(ri, "\n");
			}

			writes(ri, "\n");

			rfc2045_mimepos(first_attachment, &start_pos, &end_pos,
					&start_body,
					&dummy, &dummy);

			if (SRC_SEEK(ri->src, start_pos) == (off_t)-1)
			{
				if (subject) free(subject);
				free(boundary);
				return -1;
			}
			forwardbody(ri, end_pos - start_pos);
		}
		else
		{
			/* Copy over individual attachments, one by one */

			for (; first_attachment;
			     first_attachment=first_attachment->next)
			{
				writes(ri, "\n--");
				writes(ri, boundary);
				writes(ri, "\n");

				rfc2045_mimepos(first_attachment, &start_pos,
						&end_pos,
						&start_body,
						&dummy, &dummy);

				if (SRC_SEEK(ri->src, start_pos) == (off_t)-1)
				{
					if (subject) free(subject);
					free(boundary);
					return -1;
				}

				forwardbody(ri, end_pos - start_pos);
			}
		}

		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "--\n");
	}

	if (subject) free(subject);
	free(boundary);
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

static int replydsn(struct rfc2045_mkreplyinfo *);
static int replyfeedback(struct rfc2045_mkreplyinfo *);
static int replydraft(struct rfc2045_mkreplyinfo *);
static void copyheaders(struct rfc2045_mkreplyinfo *);

static int mkreply(struct rfc2045_mkreplyinfo *ri)
{
	char	*oldtocc, *oldfrom, *oldreplyto, *oldtolist;
	char	*subject;
	char	*oldmsgid;
	char	*oldreferences;
	char	*oldenvelope;
	char	*header, *value;
	char	*date;
	char	*newsgroup;
	char	*newsgroups;

	char	*whowrote;
	off_t	start_pos, end_pos, start_body, dummy;
	int errflag=0;
	char	*boundary;
	const char *dsn_report_type;
	int	(*dsn_report_gen)(struct rfc2045_mkreplyinfo *);

	struct rfc2045headerinfo *hi;

	oldtocc=0;
	oldtolist=0;
	oldfrom=0;
	oldreplyto=0;
	subject=0;
	oldmsgid=0;
	oldreferences=0;
	oldenvelope=0;
	whowrote=0;
	newsgroup=0;
	newsgroups=0;
	date=0;

	rfc2045_mimepos(ri->rfc2045partp, &start_pos, &end_pos, &start_body,
		&dummy, &dummy);

	hi=rfc2045header_start(ri->src, ri->rfc2045partp);

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
		if (oldenvelope) free(oldenvelope); \
		if (newsgroup) free(newsgroup); \
		if (newsgroups) free(newsgroups); \
		if (date) free(date); \
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
		else if ((strcmp(header, "return-path") == 0 ||
			  strcmp(header, "errors-to") == 0) &&
			 ri->replytoenvelope)
		{
			if (oldenvelope)	free(oldenvelope);
			oldenvelope=strdup(value);
			if (!oldenvelope)
				BLOWUP;
		}
		else if (strcmp(header, "newsgroups") == 0)
		{
			if (newsgroups)	free(newsgroups);
			newsgroups=strdup(value);
			if (!newsgroups)
				BLOWUP;
		}
		else if (strcmp(header, "x-newsgroup") == 0)
		{
			if (newsgroup)	free(newsgroup);
			newsgroup=strdup(value);
			if (!newsgroup)
				BLOWUP;
		}
		else if (strcmp(header, "date") == 0)
		{
			if (date)	free(date);
			date=strdup(value);
			if (!date)
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

	{
		struct	rfc822t *rfcp=rfc822t_alloc_new(oldfrom ? oldfrom:"",
							NULL, NULL);
		struct	rfc822a *rfcpa;
		char	*sender_name=NULL;
		char	*sender_addr=NULL;
		int	n;

		if (!rfcp)
			BLOWUP;

		rfcpa=rfc822a_alloc(rfcp);
		if (!rfcpa)
		{
			rfc822t_free(rfcp);
			BLOWUP;
		}

		for (n=0; n<rfcpa->naddrs; ++n)
		{
			if (rfcpa->addrs[n].tokens == NULL)
				continue;

			sender_name=rfc822_display_name_tobuf(rfcpa, n,
							      ri->charset);
			sender_addr=rfc822_display_addr_tobuf(rfcpa, n,
							      ri->charset);
			break;
		}

		rfc822a_free(rfcpa);
		rfc822t_free(rfcp);

		whowrote=mksalutation(ri->replysalut,
				      newsgroup ? newsgroup:"",
				      oldmsgid ? oldmsgid:"",
				      newsgroups ? newsgroups:"",

				      sender_addr ? sender_addr:"(no address given)",
				      sender_name ? sender_name:
				      sender_addr ? sender_addr:"@",
				      date,
				      subject,
				      ri->charset);

		if (sender_name)
			free(sender_name);
		if (sender_addr)
			free(sender_addr);

		if (!whowrote)
		{
			BLOWUP;
		}
	}

	if (newsgroups)
		free(newsgroups);
	if (newsgroup)
		free(newsgroup);
	if (date)
		free(date);
	if (oldreplyto)
	{
		if (oldfrom)	free(oldfrom);
		oldfrom=oldreplyto;
		oldreplyto=0;
	}

	if (oldenvelope)
	{
		if (oldfrom)	free(oldfrom);
		oldfrom=oldenvelope;
		oldenvelope=0;
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

	if (strcmp(ri->replymode, "feedback") == 0)
	{
		if (oldtolist)
		{
			free(oldtolist);
			oldtolist=NULL;
		}

		if (oldfrom)
		{
			free(oldfrom);
			oldfrom=NULL;
		}

		if (oldtocc)
		{
			free(oldtocc);
			oldtocc=NULL;
		}
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
	writes(ri,"Subject: ");

	if (ri->subject)
	{
		writes(ri, ri->subject);
	}
	else if (subject)
	{
		if (strcmp(ri->replymode, "feedback") == 0 ||
		    strcmp(ri->replymode, "replyfeedback") == 0)
		{
			writes(ri, subject);
		}
		else
		{
			char	*s=rfc822_coresubj_keepblobs(subject);

			writes(ri, "Re: ");
			writes(ri, s ? s:"");
			if (s)	free(s);
		}
		free(subject);
	}

	if (strcmp(ri->replymode, "replydraft") == 0)
	{
		writes(ri, "\n");
		if (whowrote)
			free(whowrote);
		whowrote=0;
		return replydraft(ri);
	}

	writes(ri, "\nMime-Version: 1.0\n");

	boundary=NULL;
	dsn_report_type=NULL;

	if (strcmp(ri->replymode, "replydsn") == 0 && ri->dsnfrom)
	{
		dsn_report_type="delivery-status";
		dsn_report_gen=&replydsn;
	}
	else if (strcmp(ri->replymode, "replyfeedback") == 0 ||
		 strcmp(ri->replymode, "feedback") == 0)
	{
		dsn_report_type="feedback-report";
		dsn_report_gen=&replyfeedback;
	}

	if (dsn_report_type)
	{
		boundary=rfc2045_mk_boundary(ri->rfc2045partp, ri->src);
		if (!boundary)
			return (-1);

		writes(ri, "Content-Type: multipart/report;"
		       " report-type=");

		writes(ri, dsn_report_type);
		writes(ri, ";\n    boundary=\"");

		writes(ri, boundary);

		writes(ri,"\"\n"
			"\n"
			RFC2045MIMEMSG
			"\n"
		       "--");
		writes(ri, boundary);
		writes(ri, "\n");
	}

	if (ri->content_set_charset)
	{
		(*ri->content_set_charset)(ri->voidarg);
	}
	else
	{
		writes(ri, "Content-Type: text/plain; format=flowed; delsp=yes; charset=\"");
		writes(ri, ri->charset);
		writes(ri, "\"\n");
	}
	writes(ri, "Content-Transfer-Encoding: 8bit\n\n");

	if (!ri->donotquote)
	{
		if (whowrote)
		{
			writes(ri, whowrote);
			free(whowrote);
			whowrote=0;
			writes(ri, "\n\n");
		}
		if (SRC_SEEK(ri->src, start_body) == (off_t)-1)
			return (-1);

		replybody(ri, ri->rfc2045partp);
		writes(ri, "\n");	/* First blank line in the reply */
	}

	if (ri->content_specify)
		(*ri->content_specify)(ri->voidarg);

	writes(ri, "\n");
	if (ri->writesig_func)
		(*ri->writesig_func)(ri->voidarg);
	writes(ri, "\n");

	if (boundary)
	{
		/* replydsn or replyfeedback */

		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "\n");

		if (errflag == 0)
			errflag=(*dsn_report_gen)(ri);

		writes(ri, "\n--");
		writes(ri, boundary);

		if (ri->fullmsg)
		{
			off_t cnt=end_pos - start_pos;
			char buf[BUFSIZ];

			writes(ri, "\nContent-Type: ");

			writes(ri, ri->rfc2045partp->rfcviolation &
			       RFC2045_ERR8BITHEADER
			       ? RFC2045_MIME_MESSAGE_GLOBAL
			       : RFC2045_MIME_MESSAGE_RFC822);

			writes(ri, "\n"
			       "Content-Disposition: attachment\n\n");

			if (errflag == 0)
				errflag=SRC_SEEK(ri->src, start_pos);

			while (errflag == 0 && cnt > 0)
			{
				int n=cnt > sizeof(BUFSIZ) ? BUFSIZ:(int)cnt;

				n=SRC_READ(ri->src, buf, n);

				if (n <= 0)
				{
					errflag= -1;
					break;
				}
				(*ri->write_func)(buf, n, ri->voidarg);
				cnt -= n;
			}
		}
		else
		{
			copyheaders(ri);
		}
		writes(ri, "\n--");
		writes(ri, boundary);
		writes(ri, "--\n");
		free(boundary);
	}
	return (errflag);
}

static void dsn_arrival_date(struct rfc2045_mkreplyinfo *ri)
{
	writes(ri, "Arrival-Date: ");

	time_t now;

	time(&now);

	writes(ri, rfc822_mkdate(now));
	writes(ri, "\n");
}

static void copyheaders(struct rfc2045_mkreplyinfo *ri)
{
	struct rfc2045headerinfo *hi;
	char	*header, *value;


	writes(ri, "\nContent-Type: ");

	if (ri->rfc2045partp->rfcviolation & RFC2045_ERR8BITHEADER)
		writes(ri, RFC2045_MIME_MESSAGE_GLOBAL_HEADERS);
	else
		writes(ri, RFC2045_MIME_MESSAGE_HEADERS);

	writes(ri, "; charset=\"utf-8\"\n"
	       "Content-Disposition: attachment\n"
	       "Content-Transfer-Encoding: 8bit\n\n"
	       );

	hi=rfc2045header_start(ri->src, ri->rfc2045partp);

	while (hi)
	{
		if (rfc2045header_get(hi, &header, &value,
				      RFC2045H_NOLC) || !header)
		{
			rfc2045header_end(hi);
			break;
		}

		writes(ri, header);
		writes(ri, ": ");
		writes(ri, value);
		writes(ri, "\n");
	}
}

static int replydraft(struct rfc2045_mkreplyinfo *ri)
{
	char *boundary=rfc2045_mk_boundary(ri->rfc2045partp, ri->src);
	if (!boundary)
		return (-1);

	writes(ri, "Content-Type: multipart/mixed; boundary=\"");
	writes(ri, boundary);
	writes(ri, "\"\n\n");
	writes(ri, RFC2045MIMEMSG);
	writes(ri, "\n--");
	writes(ri, boundary);
	writes(ri, "\n");

	if (ri->content_specify)
		(*ri->content_specify)(ri->voidarg);

	writes(ri, "\n--");
	writes(ri, boundary);
	copyheaders(ri);
	writes(ri, "\n--");
	writes(ri, boundary);
	writes(ri, "--\n");
	free(boundary);
	return 0;
}

static int replydsn(struct rfc2045_mkreplyinfo *ri)
{
	const char *p;
	int is8bit=0;

	for (p=ri->dsnfrom; *p; ++p)
		if (*p & 0x80)
		{
			is8bit=1;
			break;
		}

	if (is8bit)
	{
		writes(ri, "Content-Type: "
		       RFC2045_MIME_MESSAGE_GLOBAL_DELIVERY_STATUS
		       "; charset=\"utf-8\"\n");
		writes(ri, "Content-Transfer-Encoding: 8bit\n\n");
	}
	else
	{
		writes(ri, "Content-Type: "
		       RFC2045_MIME_MESSAGE_DELIVERY_STATUS
		       "\n");
		writes(ri, "Content-Transfer-Encoding: 7bit\n\n");
	}

	dsn_arrival_date(ri);

	writes (ri, "\n"
		"Final-Recipient: rfc822; ");

	writes(ri, ri->dsnfrom);

	writes(ri, "\n"
	       "Action: delivered\n"
	       "Status: 2.0.0\n");
	return 0;
}

static int replyfeedback(struct rfc2045_mkreplyinfo *ri)
{
	size_t i;

	writes(ri, "Content-Type: message/feedback-report; charset=\"utf-8\"\n");
	writes(ri, "Content-Transfer-Encoding: 8bit\n\n");

	dsn_arrival_date(ri);
	writes(ri, "User-Agent: librfc2045 "
	       RFC2045PKG "/" RFC2045VER
	       "\n"
	       "Version: 1\n");

	for (i=0; ri->feedbackheaders &&
		     ri->feedbackheaders[i] && ri->feedbackheaders[i+1];
	     i += 2)
	{
		char *p=strdup(ri->feedbackheaders[i]), *q;
		char lastch;

		if (!p)
			return -1;

		lastch='-';
		for (q=p; *q; q++)
		{
			if (*q >= 'A' && *q <= 'Z')
				*q += 'a'-'A';

			if (lastch == '-' && *q >= 'a' && *q <= 'z')
				*q += 'A' - 'a';
			lastch=*q;
		}

		writes(ri, p);
		free(p);
		writes(ri, ": ");
		writes(ri, ri->feedbackheaders[i+1]);
		writes(ri, "\n");
	}

	return 0;
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
