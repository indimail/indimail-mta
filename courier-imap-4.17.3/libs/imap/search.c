/*
** Copyright 1998 - 2009 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	<time.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"rfc822/rfc822.h"
#include	"rfc822/rfc822hdr.h"
#include	"rfc822/rfc2047.h"
#include	"rfc2045/rfc2045.h"
#include	"unicode/courier-unicode.h"
#include	"numlib/numlib.h"
#include	"searchinfo.h"
#include	"imapwrite.h"
#include	"imaptoken.h"
#include	"imapscanclient.h"


extern time_t rfc822_parsedt(const char *);
extern struct imapscaninfo current_maildir_info;
extern char *current_mailbox;

extern int get_flagname(const char *, struct imapflags *);
extern void get_message_flags( struct imapscanmessageinfo *,
	char *, struct imapflags *);
extern int valid_keyword(const char *kw);

static void fill_search_preparse(struct searchinfo *);

static void fill_search_veryquick(struct searchinfo *,
	unsigned long, struct imapflags *);

static void fill_search_quick(struct searchinfo *,
	unsigned long, struct stat *);

static void fill_search_header(struct searchinfo *,
			       const char *,
			       struct rfc2045 *, FILE *,
			       struct imapscanmessageinfo *);

static void fill_search_body(struct searchinfo *,
			     struct rfc2045 *, FILE *,
			     struct imapscanmessageinfo *);

static int search_evaluate(struct searchinfo *);

static void search_callback(struct searchinfo *, struct searchinfo *, int,
	unsigned long, void *);

/*
**	search_internal() does the main heavylifting of searching the
**	maildir for qualifying messages.  It calls a callback function
**	when a matching message is found.
**
**	For a plain SEARCH, the callback function merely prints the message
**	number.
*/

void dosearch(struct searchinfo *si, struct searchinfo *sihead,
	      const char *charset, int isuid)
{
	search_internal(si, sihead, charset, isuid, search_callback, 0);
}

static void search_callback(struct searchinfo *si, struct searchinfo *sihead,
	int isuid, unsigned long i, void *dummy)
{
	writes(" ");
	writen(isuid ? current_maildir_info.msgs[i].uid:i+1);
}

static void search_oneatatime(struct searchinfo *si,
			      unsigned long i,
			      struct searchinfo *sihead,
			      const char *charset, int isuid,
			      void (*callback_func)(struct searchinfo *,
						    struct searchinfo *, int,
						    unsigned long, void *),
			      void *voidarg);

static void search_byKeyword(struct searchinfo *tree,
			   struct searchinfo *keyword,
			   struct searchinfo *sihead,
			   const char *charset, int isuid,
			   void (*callback_func)(struct searchinfo *,
						 struct searchinfo *, int,
						 unsigned long, void *),
			   void *voidarg);

void search_internal(struct searchinfo *si, struct searchinfo *sihead,
		     const char *charset, int isuid,
		     void (*callback_func)(struct searchinfo *,
					   struct searchinfo *, int,
					   unsigned long, void *),
		     void *voidarg)
{
	unsigned long i;
	struct	searchinfo *p;

	for (p=sihead; p; p=p->next)
		fill_search_preparse(p);

	/* Shortcuts for keyword-based searches */

	if (si->type == search_msgkeyword && si->bs == NULL && si->ke)
		search_byKeyword(NULL, si, sihead, charset, isuid,
			       callback_func, voidarg);
	else if (si->type == search_and &&
		 si->a->type == search_msgkeyword && si->a->bs == NULL
		 && si->a->ke)
		search_byKeyword(si->b, si->a, sihead, charset, isuid,
			       callback_func, voidarg);
	else for (i=0; i<current_maildir_info.nmessages; i++)
		search_oneatatime(si, i, sihead, charset, isuid,
				  callback_func, voidarg);
}

static void search_byKeyword(struct searchinfo *tree,
			     struct searchinfo *keyword,
			     struct searchinfo *sihead,
			     const char *charset, int isuid,
			     void (*callback_func)(struct searchinfo *,
						   struct searchinfo *, int,
						   unsigned long, void *),
			     void *voidarg)
{
	struct libmail_kwMessageEntry *kme;

	for (kme=keyword->ke->firstMsg; kme; kme=kme->keywordNext)
	{
		unsigned long n=kme->libmail_kwMessagePtr->u.userNum;
		if (!tree)
		{
			(*callback_func)(keyword, keyword, isuid, n, voidarg);
			continue;
		}

		search_oneatatime(tree, n, sihead, charset, isuid,
				  callback_func, voidarg);
	}
}

/*
** Evaluate the search tree for a given message.
*/

static void search_oneatatime(struct searchinfo *si,
			      unsigned long i,
			      struct searchinfo *sihead,
			      const char *charset, int isuid,
			      void (*callback_func)(struct searchinfo *,
						    struct searchinfo *, int,
						    unsigned long, void *),
			      void *voidarg)
{
struct	searchinfo *p;
struct	imapflags	flags;
int	fd;
FILE	*fp;
struct	stat	stat_buf;
int	rc;

	{
		for (p=sihead; p; p=p->next)
			p->value= -1;	/* Search result unknown */

		/* First, see if non-content search will be sufficient */

		get_message_flags(current_maildir_info.msgs+i, 0, &flags);

		for (p=sihead; p; p=p->next)
			fill_search_veryquick(p, i, &flags);

		if ((rc=search_evaluate(si)) >= 0)
		{
			if (rc > 0)
				(*callback_func)(si, sihead, isuid, i,
						 voidarg);
			return;
		}

		fd=imapscan_openfile(current_mailbox,
			&current_maildir_info, i);
		if (fd < 0)	return;

		if ((fp=fdopen(fd, "r")) == 0)
			write_error_exit(0);

		if (fstat(fileno(fp), &stat_buf))
		{
			fclose(fp);
			return;
		}

		/* First, see if non-content search will be sufficient */

		for (p=sihead; p; p=p->next)
			fill_search_quick(p, i, &stat_buf);

		if ((rc=search_evaluate(si)) < 0)
		{
			/* No, search the headers then */
                        /* struct        rfc2045 *rfcp=rfc2045_fromfp(fp); */
                        struct        rfc2045 *rfcp=rfc2045header_fromfp(fp);

			fill_search_header(sihead, charset, rfcp, fp,
					   current_maildir_info.msgs+i);
			rc=search_evaluate(si);
                        rfc2045_free(rfcp);

			if (rc < 0)
			{
				/* Ok, search message contents */
                                struct        rfc2045 *rfcp=rfc2045_fromfp(fp);

				fill_search_body(sihead, rfcp, fp,
						 current_maildir_info.msgs+i);

				/*
				** If there are still UNKNOWN nodes, change
				** them to fail.
				*/

				for (p=sihead; p; p=p->next)
					if (p->value < 0)
						p->value=0;

				rc=search_evaluate(si);
                                rfc2045_free(rfcp);
			}
                        /* rfc2045_free(rfcp); */
		}

		if (rc > 0)
		{
			(*callback_func)(si, sihead, isuid, i, voidarg);
		}
		fclose(fp);
		close(fd);
	}
}

/* Check if the given index is included in the specified message set */

static int is_in_set(const char *msgset, unsigned long n)
{
unsigned long i, j;

	while (isdigit((int)(unsigned char)*msgset))
	{
		i=0;
		while (isdigit((int)(unsigned char)*msgset))
		{
			i=i*10 + (*msgset++-'0');
		}
		if (*msgset != ':')
			j=i;
		else
		{
			j=0;
			++msgset;
			if (*msgset == '*')
			{
				++msgset;
				/*
				** Ok, we don't really need to know the upper
				** end, just hack it.
				*/
				j=i;
				if (j < n)
					j=n;
			}
			else
				while (isdigit((int)(unsigned char)*msgset))
				{
					j=j*10 + (*msgset++-'0');
				}
		}
		if (n >= i && n <= j)	return (1);
		if (*msgset == 0 || *msgset++ != ',')	break;
	}
	return (0);
}

/*
** Search date comparisons compare the dates only, not the time.
** We convert all timestamps to midnight GMT on their respective dates.
** Use convenient RFC822 functions for that purpose.
*/

static int decode_date(char *p, time_t *tret)
{
	char	*s=malloc(strlen(p)+sizeof(" 00:00:00"));
	unsigned        i;
	int ret;

	if (!s)	write_error_exit(0);

        /* Convert to format rfc822_parsedt likes */

        for (i=1; p[i] != ' '; i++)
        {
                if (!p[i])	break;
        }
	memcpy(s, p, i);
	strcpy(s+i, " 00:00:00");
	while (i)
	{
		if (s[--i] == '-')
			s[i]=' ';
	}

	ret=rfc822_parsedate_chk(s, tret);
	free(s);
	return (ret);
}

/* Given a time_t that falls on, say, 3-Aug-1999 9:50:43 local time,
** calculate the time_t for midnight 3-Aug-1999 UTC.  Search date comparisons
** are done against midnight UTCs */

static time_t timestamp_to_day(time_t t)
{
char	buf1[60], buf2[80];

	rfc822_mkdate_buf(t, buf1);	/* Converts to local time */
	(void)strtok(buf1, " ");	/* Skip weekday */
	strcpy(buf2, strtok(0, " "));
	strcat(buf2, " ");
	strcat(buf2, strtok(0, " "));
	strcat(buf2, " ");
	strcat(buf2, strtok(0, " "));
	strcat(buf2, " 00:00:00");

	rfc822_parsedate_chk(buf2, &t);

	return t;
}

static char *timestamp_for_sorting(time_t t)
{
struct tm *tm=localtime(&t);
char	buf[200];

	buf[0]=0;
	if ( strftime(buf, sizeof(buf), "%Y.%m.%d.%H.%M.%S", tm) == 0)
		buf[0]=0;
	return (my_strdup(buf));
}

static void fill_search_preparse(struct searchinfo *p)
{
	switch (p->type) {
	case search_msgflag:
		{
			struct imapflags flags;

			memset(&flags, 0, sizeof(flags));
			p->ke=NULL;

			if (get_flagname(p->as, &flags) == 0)
			{
				p->bs=malloc(sizeof(flags));

				if (!p->bs)
					write_error_exit(0);

				memcpy(p->bs, &flags, sizeof(flags));
			}
		}
		break;

	case search_msgkeyword:
		p->ke=NULL;
		if (valid_keyword(p->as))
			p->ke=libmail_kweFind(current_maildir_info
					       .keywordList,
					       p->as, 0);
		break;
	default:
		break;
	}
}

/* Evaluate non-content search nodes */

static void fill_search_veryquick(struct searchinfo *p,
	unsigned long msgnum, struct imapflags *flags)
{
	switch (p->type) {
	case search_msgflag:
		{
			struct imapflags *f=(struct imapflags *)p->bs;

			p->value=0;
			if (strcmp(p->as, "\\RECENT") == 0 &&
				current_maildir_info.msgs[msgnum].recentflag)
				p->value=1;

			if (f)
			{
				if (f->seen && flags->seen)
					p->value=1;
				if (f->answered && flags->answered)
					p->value=1;
				if (f->deleted && flags->deleted)
					p->value=1;
				if (f->flagged && flags->flagged)
					p->value=1;
				if (f->drafts && flags->drafts)
					p->value=1;
			}
			break;
		}

	case search_msgkeyword:
		p->value=0;
		if (p->ke)
		{
			struct libmail_kwMessage *km=
				current_maildir_info.msgs[msgnum]
				.keywordMsg;
			struct libmail_kwMessageEntry *kme;

			for (kme=km ? km->firstEntry:NULL;
			     kme; kme=kme->next)
				if (strcasecmp(keywordName(kme->
							   libmail_keywordEntryPtr),
					       keywordName(p->ke))==0)
				{
					p->value=1;
					break;
				}
		}
		break;
	case search_messageset:
		if (is_in_set(p->as, msgnum+1))
			p->value=1;
		else
			p->value=0;
		break;
	case search_all:
		p->value=1;
		break;
	case search_uid:
		if (is_in_set(p->as, current_maildir_info.msgs[msgnum].uid))
			p->value=1;
		else
			p->value=0;
		break;
	case search_reverse:
		p->value=1;
		break;
	default:
		break;
	}
}

static void fill_search_quick(struct searchinfo *p,
	unsigned long msgnum, struct stat *stat_buf)
{
	switch (p->type)	{
	case search_before:
		p->value=0;
		{
			time_t t;

			if (decode_date(p->as, &t) == 0 &&
			    timestamp_to_day(stat_buf->st_mtime) < t)
				p->value=1;
		}
		break;
	case search_since:
		p->value=0;
		{
			time_t t;

			if (decode_date(p->as, &t) == 0 &&
			    timestamp_to_day(stat_buf->st_mtime) >= t)
				p->value=1;
		}
		break;
	case search_on:
		p->value=0;
		{
			time_t t;

			if (decode_date(p->as, &t) == 0 &&
			    timestamp_to_day(stat_buf->st_mtime) == t)
				p->value=1;
		}
		break;
	case search_smaller:
		p->value=0;
		{
		unsigned long n;

			if (sscanf(p->as, "%lu", &n) > 0 &&
				stat_buf->st_size < n)
				p->value=1;
		}
		break;
	case search_larger:
		p->value=0;
		{
		unsigned long n;

			if (sscanf(p->as, "%lu", &n) > 0 &&
				stat_buf->st_size > n)
				p->value=1;
		}
		break;
	case search_orderedsubj:
	case search_references1:
	case search_references2:
	case search_references3:
	case search_references4:
	case search_arrival:
	case search_cc:
	case search_date:
	case search_from:
	case search_reverse:
	case search_size:
	case search_to:

		/* DUMMY nodes for SORT/THREAD.  Make sure that the
		** dummy node is CLEARed */

		if (p->as)
		{
			free(p->as);
			p->as=0;
		}

		if (p->bs)
		{
			free(p->bs);
			p->bs=0;
		}

		switch (p->type)	{
		case search_arrival:
			p->as=timestamp_for_sorting(stat_buf->st_mtime);
			p->value=1;
			break;
		case search_size:
			{
			char	buf[NUMBUFSIZE], buf2[NUMBUFSIZE];
			char *q;

				libmail_str_size_t(stat_buf->st_size, buf);
				sprintf(buf2, "%*s", (int)(sizeof(buf2)-1), buf);
				for (q=buf2; *q == ' '; *q++='0')
					;
				p->as=my_strdup(buf2);
				p->value=1;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

/* Evaluate search results.  Returns: 0 - false, 1 - true, -1 - unknown
** (partial search on message metadata, like size or flags, in hopes of
** preventing a search
** of message contents).
*/

static int search_evaluate(struct searchinfo *si)
{
int	rc, rc2;

	switch (si->type)	{
	case search_orderedsubj:	/* DUMMIES for THREAD and SORT */
	case search_references1:
	case search_references2:
	case search_references3:
	case search_references4:
        case search_arrival:
        case search_cc:
        case search_date:
        case search_from:
        case search_reverse:
        case search_size:
        case search_to:
		rc = search_evaluate(si->a);
		if (rc == 0) return 0;
		if (si->value < 0)  return (-1);
		break;
	case search_not:
		rc=search_evaluate(si->a);
		if (rc >= 0)	rc= 1-rc;
		break;
	case search_and:
		rc=search_evaluate(si->a);
		rc2=search_evaluate(si->b);

		rc=  rc > 0 && rc2 > 0 ? 1:
			rc == 0 || rc2 == 0 ? 0:-1;
		break;
	case search_or:
		rc=search_evaluate(si->a);
		rc2=search_evaluate(si->b);

		rc=  rc > 0 || rc2 > 0 ? 1:
			rc == 0 && rc2 == 0 ? 0:-1;
		break;
	default:
		rc=si->value;
		break;
	}
	return (rc);
}

/* ------- header search -------- */

struct fill_search_header_info {

	struct searchinfo *si;

	char *utf8buf;
	size_t utf8buflen;
	size_t utf8bufsize;
};

static int headerfilter_func(const char *name, const char *value, void *arg);
static int fill_search_header_utf8(const char *, size_t, void *);
static int fill_search_header_done(const char *, void *);

static void fill_search_header(struct searchinfo *si,
			       const char *charset,
			       struct rfc2045 *rfcp, FILE *fp,
			       struct imapscanmessageinfo *mi)
{
	struct searchinfo *sip;
	struct rfc2045src *src;
	struct rfc2045_decodemsgtoutf8_cb decodecb;
	struct fill_search_header_info decodeinfo;

	/* Consider the following dummy nodes as evaluated */

	for (sip=si; sip; sip=sip->next)
		switch (sip->type) {
		case search_orderedsubj:
		case search_references1:
		case search_references2:
		case search_references3:
		case search_references4:
		case search_cc:
		case search_date:
		case search_from:
		case search_to:
			sip->value=1;
			break;
		default:
			break;
		}

	search_set_charset_conv(si, charset);

	src=rfc2045src_init_fd(fileno(fp));

	if (!src)
		return;

	memset(&decodecb, 0, sizeof(decodecb));
	memset(&decodeinfo, 0, sizeof(decodeinfo));

	decodeinfo.si=si;

	decodecb.flags=RFC2045_DECODEMSG_NOBODY
		| RFC2045_DECODEMSG_NOHEADERNAME;
	decodecb.headerfilter_func=headerfilter_func;
	decodecb.output_func=fill_search_header_utf8;
	decodecb.headerdone_func=fill_search_header_done;
	decodecb.arg=&decodeinfo;

	rfc2045_decodemsgtoutf8(src, rfcp, &decodecb);
	rfc2045src_deinit(src);
	if (decodeinfo.utf8buf)
		free(decodeinfo.utf8buf);
}

static int headerfilter_func(const char *name, const char *value, void *arg)
{
	struct fill_search_header_info *decodeinfo=
		(struct fill_search_header_info *)arg;
	struct searchinfo *sip;
	const char *p;
	int isto=rfc822hdr_namecmp(name, "to");
	int iscc=rfc822hdr_namecmp(name, "cc");
	int isfrom=rfc822hdr_namecmp(name, "from");
	int isinreplyto=rfc822hdr_namecmp(name, "in-reply-to");
	int isdate=rfc822hdr_namecmp(name, "date");

	int isreferences=rfc822hdr_namecmp(name, "references");
	int ismessageid=rfc822hdr_namecmp(name, "message-id");

	for (sip=decodeinfo->si; sip; sip=sip->next)
	{
		if (sip->type == search_text && sip->value <= 0)
		{
			/*
			** Full message search. Reset the search engine,
			** feed it "Headername: "
			*/

			maildir_search_reset(&sip->sei);

			for (p=name; *p; p++)
			{
				maildir_search_step_unicode_lc(&sip->sei,
							       (unsigned char)
							       *p);
				if (maildir_search_found(&sip->sei))
					sip->value=1;
			}
			for (p=": "; *p; p++)
			{
				maildir_search_step_unicode_lc(&sip->sei,
							       (unsigned char)
							       *p);
				if (maildir_search_found(&sip->sei))
					sip->value=1;
			}
		}

		if ( (sip->type == search_cc && iscc == 0 && sip->as == 0)
		     ||
		     (sip->type == search_from && isfrom == 0 && sip->as == 0)
		     ||
		     (sip->type == search_to && isto == 0 && sip->as == 0)
		     ||
		     (sip->type == search_references1 && isinreplyto == 0
		      && sip->bs == 0))
		{
			struct rfc822t *t;
			struct rfc822a *a;
			char *s;

			t=rfc822t_alloc_new(value, NULL, NULL);
			if (!t) write_error_exit(0);
			a=rfc822a_alloc(t);
			if (!a) write_error_exit(0);
			s=a->naddrs > 0 ? rfc822_getaddr(a, 0):strdup("");
			rfc822a_free(a);
			rfc822t_free(t);
			if (!s) write_error_exit(0);

			if (sip->type == search_references1)
			{
				sip->bs=malloc(strlen(s)+3);
				if (!sip->bs)
					write_error_exit(0);
				strcat(strcat(strcpy(sip->bs, "<"), s), ">");
				free(s);
			}
			else
				sip->as=s;
		}

		switch (sip->type) {
		case search_orderedsubj:

			if (isdate == 0 && sip->bs == 0)
			{
				sip->bs=strdup(value);
				if (!sip->bs)
					write_error_exit(0);
			}
			break;

		case search_date:

			if (isdate == 0 && sip->as == 0)
			{
				time_t msg_time;

				rfc822_parsedate_chk(value, &msg_time);
				sip->as=timestamp_for_sorting(msg_time);
			}
			break;

		case search_sentbefore:
		case search_sentsince:
		case search_senton:

			if (sip->value > 0)
				break;

			if (isdate == 0)
			{
				time_t given_time;
				time_t msg_time;

				if (decode_date(sip->as, &given_time)
				    || rfc822_parsedate_chk(value, &msg_time))
					break;

				msg_time=timestamp_to_day(msg_time);
				sip->value=0;
				if ((sip->type == search_sentbefore &&
					msg_time < given_time) ||
					(sip->type == search_sentsince&&
						msg_time>=given_time)||
					(sip->type == search_senton &&
						msg_time == given_time))
					sip->value=1;
			}
			break;

		case search_references1:
			if (isreferences == 0 && sip->as == 0)
			{
				sip->as=strdup(value);
				if (!sip->as)
					write_error_exit(0);
			}
			break;
		case search_references2:
			if (isdate == 0 && sip->as == 0)
			{
				sip->as=strdup(value);
				if (!sip->as)
					write_error_exit(0);
			}
			break;
		case search_references4:
			if (ismessageid == 0 && sip->as == 0)
			{
				sip->as=strdup(value);
				if (!sip->as)
					write_error_exit(0);
			}
			break;
		default:
			break;
		}
	}
	decodeinfo->utf8buflen=0;
	return 1;
}

static int fill_search_header_utf8(const char *str, size_t cnt, void *arg)
{
	struct fill_search_header_info *decodeinfo=
		(struct fill_search_header_info *)arg;

	if (decodeinfo->utf8bufsize - decodeinfo->utf8buflen < cnt)
	{
		size_t newsize=decodeinfo->utf8buflen + cnt*2;
		char *p=decodeinfo->utf8buf
			? realloc(decodeinfo->utf8buf, newsize):
			malloc(newsize);

		if (!p)
			write_error_exit(0);
		decodeinfo->utf8buf=p;
		decodeinfo->utf8bufsize=newsize;
	}

	if (cnt)
		memcpy(decodeinfo->utf8buf+decodeinfo->utf8buflen, str, cnt);
	decodeinfo->utf8buflen += cnt;
	return 0;
}

static int fill_search_header_done(const char *name, void *arg)
{
	struct fill_search_header_info *decodeinfo=
		(struct fill_search_header_info *)arg;
	struct searchinfo *sip;
	int issubject=rfc822hdr_namecmp(name, "subject");
	size_t j;
	unicode_convert_handle_t conv;
	char32_t *ucptr;
	size_t ucsize;
	int rc;

	if (decodeinfo->utf8buflen &&
	    decodeinfo->utf8buf[decodeinfo->utf8buflen-1] == '\n')
		--decodeinfo->utf8buflen;

	fill_search_header_utf8("", 1, arg);

	for (sip=decodeinfo->si; sip; sip=sip->next)
		switch (sip->type) {
		case search_references3:
			if (issubject == 0 && sip->as == 0)
			{
				sip->as=strdup(decodeinfo->utf8buf);
				if (!sip->as)
					write_error_exit(0);
			}
			break;
		case search_orderedsubj:

			if (issubject == 0 && sip->as == 0)
			{
				int dummy;

				sip->as=rfc822_coresubj(decodeinfo->utf8buf,
							&dummy);
				if (!sip->as)
					write_error_exit(0);
			}
			break;
		case search_header:

			if (sip->cs == NULL || rfc822hdr_namecmp(sip->cs, name))
				break;

			/* FALLTHRU */

		case search_text:
			if (sip->value > 0)
				break;

			maildir_search_reset(&sip->sei);

			conv=unicode_convert_tou_init("utf-8", &ucptr,
							&ucsize, 0);

			if (!conv)
				break;

			rc=unicode_convert(conv, decodeinfo->utf8buf,
					     decodeinfo->utf8buflen-1);

			if (unicode_convert_deinit(conv, NULL))
				break;

			if (rc)
			{
				free(ucptr);
				break;
			}

			for (j=0; j<=ucsize; ++j)
			{
				maildir_search_step_unicode_lc(&sip->sei,
							       j == ucsize
							       ? ' ':
							       ucptr[j]);
				if (maildir_search_found(&sip->sei))
				{
					sip->value=1;
					break;
				}
			}
			free(ucptr);
			break;
		default:
			break;
		}


	return 0;
}

struct fill_search_body_info {

	struct searchinfo *si;
	unicode_convert_handle_t toucs4_handle;

};

static int fill_search_body_utf8(const char *str, size_t n, void *arg);
static int fill_search_body_ucs4(const char *str, size_t n, void *arg);

static void fill_search_body(struct searchinfo *si,
			     struct rfc2045 *rfcp, FILE *fp,
			     struct imapscanmessageinfo *mi)
{
	struct rfc2045src *src;
	struct rfc2045_decodemsgtoutf8_cb decodecb;
	struct fill_search_body_info decodeinfo;
	struct searchinfo *sip;

	src=rfc2045src_init_fd(fileno(fp));

	if (!src)
		return;

	memset(&decodecb, 0, sizeof(decodecb));
	memset(&decodeinfo, 0, sizeof(decodeinfo));

	decodecb.flags=RFC2045_DECODEMSG_NOHEADERS;
	decodecb.output_func=fill_search_body_utf8;
	decodecb.arg=&decodeinfo;

	decodeinfo.si=si;

	if ((decodeinfo.toucs4_handle=
	     unicode_convert_init("utf-8",
				    unicode_u_ucs4_native,
				    fill_search_body_ucs4,
				    &decodeinfo)) == NULL)
	{
		write_error_exit("unicode_convert_init");
	}

	for (sip=decodeinfo.si; sip; sip=sip->next)
		if ((sip->type == search_text || sip->type == search_body)
		    && sip->value <= 0)
		{
			rfc2045_decodemsgtoutf8(src, rfcp, &decodecb);
			break;
		}

	unicode_convert_deinit(decodeinfo.toucs4_handle, NULL);

	rfc2045src_deinit(src);
}

static int fill_search_body_utf8(const char *str, size_t n, void *arg)
{
	struct fill_search_body_info *decodeinfo=
		(struct fill_search_body_info *)arg;

	return unicode_convert(decodeinfo->toucs4_handle, str, n);
}

static int fill_search_body_ucs4(const char *str, size_t n, void *arg)
{
	struct fill_search_body_info *decodeinfo=
		(struct fill_search_body_info *)arg;
	struct searchinfo *sip;
	const char32_t *u=(const char32_t *)str;
	int notfound=1;

	n /= 4;

	for (sip=decodeinfo->si; sip; sip=sip->next)
		if ((sip->type == search_text || sip->type == search_body)
		    && sip->value <= 0)
		{
			size_t i;

			notfound=0;

			for (i=0; i<n; i++)
			{
				maildir_search_step_unicode_lc(&sip->sei, u[i]);

				if (maildir_search_found(&sip->sei))
				{
					sip->value=1;
					break;
				}
			}
		}

	return notfound;
}
