/*
** Copyright 1998 - 2003 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#if     HAVE_UNISTD_H
#include        <unistd.h>
#endif
#include	<errno.h>
#include	"imaptoken.h"
#include	"imapscanclient.h"
#include	"imapwrite.h"
#include	"storeinfo.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildiraclt.h"
#include	"outbox.h"

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<fcntl.h>
#include	<sys/stat.h>
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


#if SMAP
extern int smapflag;
#endif

extern char *get_reflagged_filename(const char *fn, struct imapflags *newfl);
extern int is_trash(const char *);
extern int get_flagname(const char *, struct imapflags *);
extern int get_flagsAndKeywords(struct imapflags *flags,
				struct libmail_kwMessage **kwPtr);
extern void get_message_flags( struct imapscanmessageinfo *,
			       char *, struct imapflags *);
extern int reflag_filename(struct imapscanmessageinfo *, struct imapflags *,
	int);
extern void fetchflags(unsigned long);
extern void fetchflags_byuid(unsigned long);
extern FILE *maildir_mkfilename(const char *, struct imapflags *,
				unsigned long, char **, char **);
extern int acl_flags_adjust(const char *access_rights,
			    struct imapflags *flags);

extern struct imapscaninfo current_maildir_info;
extern char *current_mailbox;
extern char *current_mailbox_acl;
extern int fastkeywords();

int storeinfo_init(struct storeinfo *si)
{
struct imaptoken *t=currenttoken();
const char *p;

	if (t->tokentype != IT_ATOM)	return (-1);
	si->plusminus=0;
	si->silent=0;

	p=t->tokenbuf;
	if (*p == '+' || *p == '-')
		si->plusminus= *p++;
	if (strncmp(p, "FLAGS", 5))	return (-1);
	p += 5;
	if (*p)
	{
		if (strcmp(p, ".SILENT"))	return (-1);
		si->silent=1;
	}

	memset(&si->flags, 0, sizeof(si->flags));

	if ((si->keywords=libmail_kwmCreate()) == NULL)
		write_error_exit(0);

	t=nexttoken_noparseliteral();
	if (t->tokentype == IT_LPAREN)
	{
		if (get_flagsAndKeywords(&si->flags, &si->keywords))
		{
			libmail_kwmDestroy(si->keywords);
			si->keywords=NULL;
			return (-1);
		}
		nexttoken();
	}
	else if (t->tokentype == IT_NIL)
		nexttoken();
	else if (t->tokentype == IT_ATOM)
	{
		if (get_flagname(t->tokenbuf, &si->flags))
			libmail_kwmSetName(current_maildir_info
				       .keywordList,
				       si->keywords,
				       t->tokenbuf);
		nexttoken();
	}
	return (0);
}

int do_store(unsigned long n, int byuid, void *voidptr)
{
struct storeinfo *si=(struct storeinfo *)voidptr;
int	fd;
 struct imapflags new_flags, old_flags;
int changedKeywords;
struct libmail_kwMessageEntry *kme;
int kwAllowed=1;

	--n;
	fd=imapscan_openfile(current_mailbox, &current_maildir_info, n);
	if (fd < 0)	return (0);

	changedKeywords=0;
	get_message_flags(current_maildir_info.msgs+n, 0, &new_flags);

	old_flags=new_flags;

	if (current_mailbox_acl)
	{
		if (strchr(current_mailbox_acl, ACL_WRITE[0]) == NULL)
			kwAllowed=0;
	}


	if (si->plusminus == '+')
	{
		if (si->flags.drafts)	new_flags.drafts=1;
		if (si->flags.seen)	new_flags.seen=1;
		if (si->flags.answered)	new_flags.answered=1;
		if (si->flags.deleted)	new_flags.deleted=1;
		if (si->flags.flagged)	new_flags.flagged=1;

		for (kme=si->keywords ? si->keywords->firstEntry:NULL;
		     kme; kme=kme->next)
		{
			int rc;

			if (!kwAllowed)
			{
				current_maildir_info.msgs[n].changedflags=1;
				continue;
			}

			imapscan_createKeyword(&current_maildir_info, n);

			if ((rc=libmail_kwmSet(current_maildir_info.msgs[n]
					       .keywordMsg,
					       kme->libmail_keywordEntryPtr))
			    < 0)
			{
				write_error_exit(0);
				return 0;
			}

			if (rc == 0)
			{
				if (fastkeywords())
					changedKeywords=1;
				current_maildir_info.msgs[n].changedflags=1;
			}
		}
	}
	else if (si->plusminus == '-')
	{
		if (si->flags.drafts)	new_flags.drafts=0;
		if (si->flags.seen)	new_flags.seen=0;
		if (si->flags.answered)	new_flags.answered=0;
		if (si->flags.deleted)	new_flags.deleted=0;
		if (si->flags.flagged)	new_flags.flagged=0;

		if (current_maildir_info.msgs[n].keywordMsg && kwAllowed)
			for (kme=si->keywords ?
				     si->keywords->firstEntry:NULL;
			     kme; kme=kme->next)
			{
				if (!kwAllowed)
				{
					current_maildir_info.msgs[n]
						.changedflags=1;
					continue;
				}

				if (libmail_kwmClear(current_maildir_info.msgs[n]
						 .keywordMsg,
						 kme->libmail_keywordEntryPtr)==0)
				{
					if (fastkeywords())
						changedKeywords=1;
					current_maildir_info.msgs[n]
						.changedflags=1;
				}
			}

		if (current_maildir_info.msgs[n].keywordMsg &&
		    !current_maildir_info.msgs[n].keywordMsg->firstEntry)
		{
			libmail_kwmDestroy(current_maildir_info.msgs[n]
					      .keywordMsg);
			current_maildir_info.msgs[n].keywordMsg=NULL;
		}
	}
	else
	{
		struct libmail_kwMessage *kw;

		new_flags=si->flags;

		kw=current_maildir_info.msgs[n].keywordMsg;

		if (kw && kw->firstEntry == NULL)
			kw=NULL;

		if (si->keywords && si->keywords->firstEntry == NULL)
			si->keywords=NULL;

		if ((si->keywords && !kw) ||
		    (!si->keywords && kw) ||
		    (si->keywords && kw && libmail_kwmCmp(si->keywords, kw)))
		{
			if (kwAllowed)
			{
				kw=current_maildir_info.msgs[n].keywordMsg;

				if (kw)
					libmail_kwmDestroy(kw);

				current_maildir_info.msgs[n].keywordMsg=NULL;

				if (si->keywords && si->keywords->firstEntry)
				{
					struct libmail_kwMessageEntry *kme;

					kw=imapscan_createKeyword(&current_maildir_info,
								  n);

					for (kme=si->keywords->lastEntry; kme;
					     kme=kme->prev)
						if (libmail_kwmSet(kw,
								   kme->libmail_keywordEntryPtr)
						    < 0)
							write_error_exit(0);
					current_maildir_info.msgs[n].keywordMsg=kw;
				}
			}

			changedKeywords=1;
		}
	}

	if (current_mailbox_acl)
	{
		if (strchr(current_mailbox_acl, ACL_WRITE[0]) == NULL)
		{
			new_flags.drafts=old_flags.drafts;
			new_flags.answered=old_flags.answered;
			new_flags.flagged=old_flags.flagged;
		}

		if (strchr(current_mailbox_acl, ACL_SEEN[0]) == NULL)
		{
			new_flags.seen=old_flags.seen;
		}

		if (strchr(current_mailbox_acl, ACL_DELETEMSGS[0])
		    == NULL)
		{
			new_flags.deleted=old_flags.deleted;
		}
	}

	if (changedKeywords)
	{
		current_maildir_info.msgs[n].changedflags=1;
		if (imapscan_updateKeywords(current_maildir_info.msgs[n]
					    .filename,
					    current_maildir_info.msgs[n]
					    .keywordMsg))
		{
			close(fd);
			return -1;
		}
	}

	if (reflag_filename(current_maildir_info.msgs+n, &new_flags, fd))
	{
		close(fd);
		return (-1);
	}
	close(fd);
	if (si->silent)
		current_maildir_info.msgs[n].changedflags=0;
	else
	{
#if SMAP
		/* SMAP flag notification is handled elsewhere */

		if (!smapflag)
#endif
		{
			if (byuid)
				fetchflags_byuid(n);
			else
				fetchflags(n);
		}
	}

	return (0);
}

static int copy_message(int fd,
			struct do_copy_info *cpy_info,
			struct	imapflags *flags,
			struct libmail_kwMessage *keywords,
			unsigned long old_uid)
{
char	*tmpname;
char	*newname;
FILE	*fp;
struct	stat	stat_buf;
char	buf[BUFSIZ];
struct uidplus_info *new_uidplus_info;

	if (fstat(fd, &stat_buf) < 0
	    || (new_uidplus_info=(struct uidplus_info *)
		malloc(sizeof(struct uidplus_info))) == NULL)
	{
		return (-1);
	}
	memset(new_uidplus_info, 0, sizeof(*new_uidplus_info));

	fp=maildir_mkfilename(cpy_info->mailbox, flags, stat_buf.st_size,
			      &tmpname, &newname);

	if (!fp)
	{
		free(new_uidplus_info);
		return (-1);
	}

	while (stat_buf.st_size)
	{
	int	n=sizeof(buf);

		if (n > stat_buf.st_size)
			n=stat_buf.st_size;

		n=read(fd, buf, n);

		if (n <= 0 || fwrite(buf, 1, n, fp) != n)
		{
			fprintf(stderr,
			"ERR: error copying a message, user=%s, errno=%d\n",
				getenv("AUTHENTICATED"), errno);

			fclose(fp);
			unlink(tmpname);
			free(tmpname);
			free(newname);
			free(new_uidplus_info);
			return (-1);
		}
		stat_buf.st_size -= n;
	}

	if (fflush(fp) || ferror(fp))
	{
		fclose(fp);
		free(tmpname);
		free(newname);
		free(new_uidplus_info);
		return (-1);
	}
	fclose(fp);

	new_uidplus_info->mtime = stat_buf.st_mtime;

	if (check_outbox(tmpname, cpy_info->mailbox))
	{
		unlink(tmpname);
		free(tmpname);
		free(newname);
		free(new_uidplus_info);
		return (-1);
	}

	if (keywords && keywords->firstEntry &&
	    maildir_kwSave(cpy_info->mailbox,
			   strrchr(newname, '/')+1,
			   keywords,
			   &new_uidplus_info->tmpkeywords,
			   &new_uidplus_info->newkeywords, 0))
	{
		unlink(tmpname);
		free(tmpname);
		free(newname);
		free(new_uidplus_info);
		perror("maildir_kwSave");
		return (-1);
	}

	new_uidplus_info->tmpfilename=tmpname;
	new_uidplus_info->curfilename=newname;
	new_uidplus_info->next=NULL;
	new_uidplus_info->old_uid=old_uid;
	*cpy_info->uidplus_tail=new_uidplus_info;
	cpy_info->uidplus_tail=&new_uidplus_info->next;
	return (0);
}

int do_copy_message(unsigned long n, int byuid, void *voidptr)
{
	struct do_copy_info *cpy_info=(struct do_copy_info *)voidptr;
	int	fd;
	struct imapflags new_flags;

	--n;
	fd=imapscan_openfile(current_mailbox, &current_maildir_info, n);
	if (fd < 0)	return (0);
	get_message_flags(current_maildir_info.msgs+n, 0, &new_flags);

	if (copy_message(fd, cpy_info, &new_flags,

			 acl_flags_adjust(cpy_info->acls,
					  &new_flags) ? NULL
			 : current_maildir_info.msgs[n].keywordMsg,
			 current_maildir_info.msgs[n].uid))
	{
		close(fd);
		return (-1);
	}
	close(fd);
	current_maildir_info.msgs[n].copiedflag=1;
	return (0);
}

int do_copy_quota_calc(unsigned long n, int byuid, void *voidptr)
{
struct copyquotainfo *info=(struct copyquotainfo *)voidptr;
const char *filename;
unsigned long nbytes;
struct	stat	stat_buf;
int	fd;
struct imapflags flags;
char *ff;

	--n;

	fd=imapscan_openfile(current_mailbox, &current_maildir_info, n);
	if (fd < 0)	return (0);
	filename=current_maildir_info.msgs[n].filename;

	get_message_flags(&current_maildir_info.msgs[n], NULL, &flags);

	(void)acl_flags_adjust(info->acls, &flags);

	ff=get_reflagged_filename(filename, &flags);

	if (maildirquota_countfile(ff))
	{
		if (maildir_parsequota(ff, &nbytes))
		{
			if (fstat(fd, &stat_buf) < 0)
			{
				close(fd);
				free(ff);
				return (0);
			}
			nbytes=stat_buf.st_size;
		}
		info->nbytes += nbytes;
		info->nfiles += 1;
	}
	close(fd);
	free(ff);
	return (0);
}
