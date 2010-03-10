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
#include	"rfc822/rfc2047.h"
#include	"rfc2045/rfc2045.h"
#include	"unicode/unicode.h"
#include	"numlib/numlib.h"
#include	"searchinfo.h"
#include	"imapwrite.h"
#include	"imaptoken.h"
#include	"imapscanclient.h"

static const char rcsid[]="$Id: search.c,v 1.34 2009/11/08 18:14:47 mrsam Exp $";

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
			       const struct unicode_info *,
			       struct rfc2045 *, FILE *,
			       struct imapscanmessageinfo *);

static void fill_search_body(struct searchinfo *,
			     const struct unicode_info *,
	struct rfc2045 *, FILE *, struct imapscanmessageinfo *);

static int search_evaluate(struct searchinfo *);

static void searcherror(const char *errmsg,
                struct imapscanmessageinfo *mi)
{
        fprintf(stderr, "IMAP SEARCH ERROR: %s, uid=%u, filename=%s\n",
                errmsg, (unsigned)getuid(), mi->filename);
}

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
	      const struct unicode_info *mycharset, int isuid)
{
	search_internal(si, sihead, mycharset, isuid, search_callback, 0);
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
			      const struct unicode_info *mycharset, int isuid,
			      void (*callback_func)(struct searchinfo *,
						    struct searchinfo *, int,
						    unsigned long, void *),
			      void *voidarg);

static void search_byKeyword(struct searchinfo *tree,
			   struct searchinfo *keyword,
			   struct searchinfo *sihead,
			   const struct unicode_info *mycharset, int isuid,
			   void (*callback_func)(struct searchinfo *,
						 struct searchinfo *, int,
						 unsigned long, void *),
			   void *voidarg);

void search_internal(struct searchinfo *si, struct searchinfo *sihead,
		     const struct unicode_info *mycharset, int isuid,
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
		search_byKeyword(NULL, si, sihead, mycharset, isuid,
			       callback_func, voidarg);
	else if (si->type == search_and &&
		 si->a->type == search_msgkeyword && si->a->bs == NULL
		 && si->a->ke)
		search_byKeyword(si->b, si->a, sihead, mycharset, isuid,
			       callback_func, voidarg);
	else for (i=0; i<current_maildir_info.nmessages; i++)
		search_oneatatime(si, i, sihead, mycharset, isuid,
				  callback_func, voidarg);
}

static void search_byKeyword(struct searchinfo *tree,
			     struct searchinfo *keyword,
			     struct searchinfo *sihead,
			     const struct unicode_info *mycharset, int isuid,
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

		search_oneatatime(tree, n, sihead, mycharset, isuid,
				  callback_func, voidarg);
	}
}

/*
** Evaluate the search tree for a given message.
*/

static void search_oneatatime(struct searchinfo *si,
			      unsigned long i,
			      struct searchinfo *sihead,
			      const struct unicode_info *mycharset, int isuid,
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

			fill_search_header(sihead, mycharset, rfcp, fp,
				current_maildir_info.msgs+i);
			rc=search_evaluate(si);
                        rfc2045_free(rfcp); 

			if (rc < 0)
			{
				/* Ok, search message contents */
                                struct        rfc2045 *rfcp=rfc2045_fromfp(fp);

				fill_search_body(sihead, mycharset, rfcp, fp,
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

static time_t decode_date(char *p)
{
char	*s=malloc(strlen(p)+sizeof(" 00:00:00"));
unsigned        i;
time_t	t;

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

	t=rfc822_parsedt(s);
	free(s);
	return (t);
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
	return (rfc822_parsedt(buf2));
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
		time_t t=decode_date(p->as);

			if (t && timestamp_to_day(stat_buf->st_mtime) < t)
				p->value=1;
		}
		break;
	case search_since:
		p->value=0;
		{
		time_t t=decode_date(p->as);

			if (t && timestamp_to_day(stat_buf->st_mtime) >= t)
				p->value=1;
		}
		break;
	case search_on:
		p->value=0;
		{
		time_t t=decode_date(p->as);

			if (t && timestamp_to_day(stat_buf->st_mtime) == t)
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

static void fill_search_header_recursive(struct searchinfo *,
					 const struct unicode_info *,
	struct rfc2045 *, FILE *, struct imapscanmessageinfo *);

static void fill_search_header(struct searchinfo *si,
			       const struct unicode_info *mycharset,
	struct rfc2045 *rfcp, FILE *fp, struct imapscanmessageinfo *mi)
{
	off_t start_pos, end_pos, start_body;
	off_t nlines, nbodylines;
	struct searchinfo *sip;

	/* Consider the following dummy nodes as evaluated */

	for (sip=si; sip; sip=sip->next)
		switch (sip->type) {
		case search_orderedsubj:
		case search_references1:
		case search_references2:
		case search_references3:
		case search_references4:
			sip->value=1;
			break;
		default:
			break;
		}

	rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	/*
	** Since headers can contain any character set, we set the search
	** strings to their native character set.
	*/

	search_set_charset_conv(si, mycharset, mycharset);

	if (fseek(fp, start_pos, SEEK_SET) < 0)
	{
		searcherror("fseek failed", mi);
		return;
	}

	while (start_pos < start_body)
	{
	char	*p=readline(0, fp);
	char	*q=0;
	char	*r;
	int	isdate;
	char *conv_buf;
	char *conv_buf_uc;
	const struct unicode_info *search_chset;

		/* Collect multiline header into the readline buffer */

		while (p)
		{
		int	c;

			start_pos=ftell(fp);
			if (start_pos >= start_body)	break;

			c=getc(fp);
			if (c != EOF)	ungetc(c, fp);
			if (c == '\n' || !isspace((int)(unsigned char)c))
				break;
			p=readline(strlen(p), fp);
		}
		if (!p)	break;

		/* search_text applies to headers too */

		for (r=p; *r && *r != ':'; r++)
			*r=toupper(*r);

		if (r)
			*r++=0;

		while (*r && isspace((int)(unsigned char)*r))
			++r;

		search_chset=mycharset;

		if (search_chset->search_chset)
			search_chset=search_chset->search_chset;


		conv_buf=rfc822_display_hdrvalue_tobuf(p, r,
						       search_chset->chset,
						       NULL, NULL);
			/* Characters not in our charset are discarded */

		if (!conv_buf && errno != EINVAL)
			write_error_exit(0);

		conv_buf_uc= (*search_chset->toupper_func)(search_chset,
							   conv_buf, 0);
		if (!conv_buf_uc)
			write_error_exit(0);

		for (sip=si; sip; sip=sip->next)
		{
			if (sip->type == search_text &&
			    sip->bs &&
			    sip->value <= 0)
			{
				maildir_search_reset(&sip->sei);

				for (q=conv_buf_uc; *q; q++)
					maildir_search_step(&sip->sei,
						    (unsigned char)*q);
				if (maildir_search_found(&sip->sei))
					sip->value=1;
			}
		}

		q=conv_buf_uc;

		isdate=strcmp(p, "DATE");
		for (sip=si; sip; sip=sip->next)
		{
			if (sip->type == search_text)
			{
				if (sip->value <= 0 &&
				    sip->bs && maildir_search_found(&sip->sei))
					sip->value=1;
			}

			if (isdate == 0) switch (sip->type)	{
			time_t given_time, msg_time;

			case search_orderedsubj:
				if (sip->bs == 0)
				{
					sip->bs=strdup(q);
					if (!sip->bs)
						write_error_exit(0);
				}
				break;
			case search_sentbefore:
			case search_sentsince:
			case search_senton:

				if (sip->value > 0)	break;

				given_time=decode_date(sip->as);
				msg_time=rfc822_parsedt(q);

				if (given_time == 0 || msg_time == 0)
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
				break;
			default:
				break;
			}

			if (sip->type == search_cc && strcmp(p, "CC") == 0 &&
				sip->as == 0)
			{
			struct rfc822t *t=rfc822t_alloc_new(q, NULL, NULL);
			struct rfc822a *a=t ? rfc822a_alloc(t):0;

				sip->as= a && a->naddrs > 0 ?
					rfc822_getaddr(a, 0):0;
				if (a)	rfc822a_free(a);
				if (t)	rfc822t_free(t);
			}

			if (sip->type == search_date && strcmp(p, "DATE") == 0
				&& sip->as == 0)
			{
			time_t msg_time=rfc822_parsedt(q);

				sip->as=timestamp_for_sorting(msg_time);
			}

			if (sip->type == search_from && strcmp(p, "FROM") == 0
				&& sip->as == 0)
			{
			struct rfc822t *t=rfc822t_alloc_new(q, NULL, NULL);
			struct rfc822a *a=t ? rfc822a_alloc(t):0;

				sip->as= a && a->naddrs > 0 ?
					rfc822_getaddr(a, 0):0;
				if (a)	rfc822a_free(a);
				if (t)	rfc822t_free(t);
			}

			if (sip->type == search_to && strcmp(p, "TO") == 0 &&
				sip->as == 0)
			{
			struct rfc822t *t=rfc822t_alloc_new(q, NULL, NULL);
			struct rfc822a *a=t ? rfc822a_alloc(t):0;

				sip->as= a && a->naddrs > 0 ?
					rfc822_getaddr(a, 0):0;
				if (a)	rfc822a_free(a);
				if (t)	rfc822t_free(t);
			}

			switch (sip->type)	{
			case search_cc:
			case search_date:
			case search_from:
			case search_to:
				sip->value=1;
				break;
			default:
				break;
			}

			if (sip->type == search_references1 &&
			    strcmp(p, "REFERENCES") == 0 && sip->as == 0)
			{
				sip->as=strdup(conv_buf);
				if (!sip->as)
					write_error_exit(0);
			}

			if (sip->type == search_references1 &&
			    strcmp(p, "IN-REPLY-TO") == 0 && sip->bs == 0)
			{
				/* Extract first address from In-REPLY-To */

				struct rfc822t *t;
				struct rfc822a *a;
				char *s;

				t=rfc822t_alloc_new(conv_buf, NULL, NULL);
				if (!t) write_error_exit(0);
				a=rfc822a_alloc(t);
				if (!a) write_error_exit(0);
				s=a->naddrs > 0 ? rfc822_getaddr(a, 0):
					strdup("");
				rfc822a_free(a);
				rfc822t_free(t);
				if (!s) write_error_exit(0);

				sip->bs=malloc(strlen(s)+3);
				if (!sip->bs)
					write_error_exit(0);
				strcat(strcat(strcpy(sip->bs, "<"), s), ">");
				free(s);
			}

			if (sip->type == search_references2 &&
			    strcmp(p, "DATE") == 0 && sip->as == 0)
			{
				sip->as=strdup(q);
				if (!sip->as)
					write_error_exit(0);
			}

			if (sip->type == search_references3 &&
			    strcmp(p, "SUBJECT") == 0 && sip->as == 0)
			{
				sip->as=unicode_xconvert(q, search_chset,
							 &unicode_UTF8);
				if (!sip->as)
					write_error_exit(0);
			}

			if (sip->type == search_references4 &&
			    strcmp(p, "MESSAGE-ID") == 0 && sip->as == 0)
			{
				sip->as=strdup(conv_buf);
				if (!sip->as)
					write_error_exit(0);
			}

			if (sip->type == search_orderedsubj &&
				strcmp(p, "SUBJECT") == 0 && sip->as == 0)
			{
				int dummy;

				sip->as=rfc822_coresubj(q, &dummy);
				if (!sip->as)
					write_error_exit(0);
			}

			if (sip->type != search_header ||
			    sip->bs == 0 ||
				sip->value > 0)	continue;

			if (strcmp(sip->cs, p) == 0)
			{
				sip->value=0;
				maildir_search_reset(&sip->sei);
				for (r=q; *r; r++)
					maildir_search_step(&sip->sei, *r);
				if (maildir_search_found(&sip->sei))
					sip->value=1;
			}
		}
		free(conv_buf_uc);
		free(conv_buf);
	}
	fill_search_header_recursive(si, mycharset, rfcp, fp, mi);
}

/* Scan the headers of attached messages too! */

static void fill_search_header_recursive(struct searchinfo *si,
					 const struct unicode_info *mycharset,
	struct rfc2045 *rfcp, FILE *fp, struct imapscanmessageinfo *mi)
{
	if (!rfcp)	return;

	if (rfcp->firstpart && rfcp->firstpart->next == 0 &&
		!rfcp->firstpart->isdummy)
	{
		fill_search_header(si, mycharset, rfcp->firstpart, fp, mi);
		return;
	}

	rfcp=rfcp->firstpart;

	while (rfcp)
	{
		if (!rfcp->isdummy)
			fill_search_header_recursive(si, mycharset,
						     rfcp, fp, mi);
		rfcp=rfcp->next;
	}
}

static int search_body_func(const char *, size_t, void *);

/* Last straw - search message contents */

struct search_linebuf {
	char buf[BUFSIZ];
	int l;
	const struct unicode_info *uc;
	struct searchinfo *si;
} ;

static void fill_search_body(struct searchinfo *si,
			     const struct unicode_info *mycharset,
	struct rfc2045 *rfcp, FILE *fp, struct imapscanmessageinfo *mi)
{
struct searchinfo *sip;
off_t start_pos, end_pos, start_body;
off_t nlines, nbodylines;
const char *content_type_s;
const char *content_transfer_encoding_s;
const char *charset_s;
char	buf[BUFSIZ];
int	rc;

 struct search_linebuf slb;

	for (sip=si; sip; sip=sip->next)
		if ((sip->type == search_text || sip->type == search_body)
			&& sip->value <= 0)
			break;

	if (sip == 0)	return;	/* Nothing to search any more */

	if (rfcp->isdummy)	return;	/* stub RFC2045 section */

	if (rfcp->firstpart)	/* Search multiline contents */
	{
		for (rfcp=rfcp->firstpart; rfcp; rfcp=rfcp->next)
			fill_search_body(si, mycharset, rfcp, fp, mi);
		return;
	}

	/*
	** Since headers can contain any character set, we set the search
	** strings to their native character set.
	*/

	rfc2045_mimeinfo(rfcp, &content_type_s,
		&content_transfer_encoding_s, &charset_s);
	rfc2045_mimepos(rfcp, &start_pos, &end_pos, &start_body,
		&nlines, &nbodylines);

	slb.l=0;
	slb.uc=unicode_find(charset_s);
	slb.si=si;

	if (!slb.uc)
		return;	/* Don't know how to search this charset */

	search_set_charset_conv(si, mycharset, slb.uc);

	if (fseek(fp, start_body, SEEK_SET) < 0)
	{
		searcherror("fseek failed", mi);
		return;
	}
	rfc2045_cdecode_start(rfcp, search_body_func, &slb);

	for (sip=si; sip; sip=sip->next)
		if ((sip->type == search_text || sip->type == search_body)
		    && sip->bs
		    && sip->value <= 0)
		{
			si->value=0;
			maildir_search_reset(&si->sei);
		}

	rc=0;
	while (start_body < end_pos)
	{
	unsigned n=sizeof(buf);

		if (n > end_pos - start_body)
			n=end_pos - start_body;

		rc=fread(buf, 1, n, fp);
		if (rc <= 0)
		{
			searcherror("fread failed", mi);
			break;
		}
		start_body += rc;
		rc=rfc2045_cdecode(rfcp, buf, rc);
		if (rc)	break;
	}

	if (rc == 0)	(void)rfc2045_cdecode_end(rfcp);
	for (sip=si; sip; sip=sip->next)
		if ((sip->type == search_text || sip->type == search_body)
		    && sip->bs
		    && sip->value <= 0)
		{
			if (maildir_search_found(&sip->sei))
				sip->value=1;
			else
				sip->value=0;
		}
}

static int search_body_func(const char *ptr, size_t cnt, void *voidptr)
{
	struct search_linebuf *slb=(struct search_linebuf *)voidptr;

	struct searchinfo *si, *sihead=(struct searchinfo *)slb->si;
	size_t	i, j;

	for (i=0; i<cnt; i++)
	{
		char c=ptr[i];
		char *p;
		const struct unicode_info *uc_info;

		if (c != '\n' && c != '\r')
		{
			if (slb->l < sizeof(slb->buf)-2)
				slb->buf[slb->l++]=c;
			continue;
		}
		slb->buf[slb->l]=0;

		uc_info=slb->uc;

		if (uc_info->search_chset)
		{
			char *q;

			p=unicode_xconvert(slb->buf, uc_info,
					   uc_info->search_chset);

			if (p)
			{
				uc_info=uc_info->search_chset;
				q=(uc_info->toupper_func)(uc_info, p, 0);
				free(p);
				p=q;
			}

		}
		else
			p=(*uc_info->toupper_func)(uc_info, slb->buf, 0);
		if (!p)
			write_error_exit(0);

		for (si=sihead; si; si=si->next)
		{
			if ((si->type == search_text ||
			     si->type == search_body)
			    && si->bs && si->value <= 0)
			{
				si->value=0;
				maildir_search_reset(&si->sei);

				for (j=0; p[j]; j++)
					maildir_search_step(&si->sei,
						    (unsigned char)p[j]);

				if (maildir_search_found(&si->sei))
					si->value=1;
			}
		}
		free(p);
		slb->l=0;
	}
	return (0);
}
