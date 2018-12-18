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
#include	<errno.h>
#include	<ctype.h>
#include	<signal.h>
#include	<fcntl.h>
#include	<pwd.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
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
#if HAVE_LOCALE_H
#include	<locale.h>
#endif
#ifdef HAVE_SYS_UTSNAME_H
#include    <sys/utsname.h>
#endif

#include	"maildir/maildiraclt.h"
#include	"maildir/maildirnewshared.h"

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"imapscanclient.h"

#include	"mysignal.h"
#include	"imapd.h"
#include	"fetchinfo.h"
#include	"searchinfo.h"
#include	"storeinfo.h"
#include	"mailboxlist.h"
#include	"thread.h"
#include	"outbox.h"
#include	"authlib/authmod.h"
#include	"authlib/auth.h"
#include	"rfc822/rfc822.h"
#include	"rfc2045/rfc2045.h"

#include	"maildir/config.h"
#include	"maildir/maildiraclt.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildirrequota.h"
#include	"maildir/maildirgetquota.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirwatch.h"
#include	"maildir/maildirkeywords.h"
#include	"maildir/maildirinfo.h"
#include	"maildir/loginexec.h"

#include	"unicode/courier-unicode.h"
#include	"maildir/maildirkeywords.h"

#define KEYWORD_IMAPVERBOTTEN " (){%*\"\\]"
#define KEYWORD_SMAPVERBOTTEN ","

static const char rcsid[]="$Id: imapd.c,v 1.163 2011/01/10 01:47:58 mrsam Exp $";
extern time_t rfc822_parsedt(const char *);
extern void fetchflags(unsigned long);
extern unsigned long header_count, body_count;
extern time_t start_time;
extern void smap();
extern void smap_fetchflags(unsigned long);

extern int do_fetch(unsigned long, int, void *);
extern void fetch_free_cached();
extern int keywords();
extern int fastkeywords();
extern void imapscanfail(const char *);
extern void bye_msg(const char *);

extern void mainloop();
extern void initcapability();
extern void imapcapability();
extern int magictrash();

#if SMAP
int smapflag=0;

extern void snapshot_save();
extern void snapshot_needed();

#endif

static const char *protocol;

char *dot_trash = "." TRASH;
char *trash = TRASH;

char *current_mailbox=0;	/* .folder */
FILE *debugfile=0;
#if 0
char *imapscanpath;
#endif

int current_temp_fd=-1;
const char *current_temp_fn=NULL;

struct imapscaninfo current_maildir_info;
int current_mailbox_ro;
char *current_mailbox_acl;

dev_t homedir_dev;
ino_t homedir_ino;
int enabled_utf8=0;

void rfc2045_error(const char *p)
{
	if (write(2, p, strlen(p)) < 0)
		_exit(1);
	_exit(0);
}

void writemailbox(const char *mailbox)
{
	char *encoded=imap_filename_to_foldername(enabled_utf8, mailbox);

	if (!encoded)
	{
		fprintf(stderr, "ERR: imap_filename_to_foldername(%s) failed\n",
			mailbox);
		exit(1);
	}
	writeqs(encoded);
	free(encoded);
}

extern int maildirsize_read(const char *,int *,off_t *,unsigned *,unsigned *,struct stat *);

int maildir_info_suppress(const char *maildir)
{
	struct stat stat_buf;

	if (stat(maildir, &stat_buf) < 0 ||
	    /* maildir inaccessible, perhaps another server? */

	    (stat_buf.st_dev == homedir_dev &&
	     stat_buf.st_ino == homedir_ino)
		    /* Exclude ourselves from the shared list */

	    )
	{
		return 1;
	}

	return 0;
}


void quotainfo_out(const char* qroot)
{
	char    quotabuf[QUOTABUFSIZE];
	char	qresult[200]="";
	char	qbuf[200];

	if ((maildir_getquota(".", quotabuf) == 0) && (strcmp(qroot,"ROOT") == 0))
	{
		struct maildirsize quotainfo;

		if (maildir_openquotafile(&quotainfo, ".") == 0)
			maildir_closequotafile(&quotainfo);
		else
			quotainfo.quota.nbytes=quotainfo.size.nbytes=
				quotainfo.quota.nmessages=
				quotainfo.size.nmessages=0;

		if (quotainfo.quota.nbytes > 0)
		{
			sprintf(qbuf,"STORAGE %ld %ld",
				(long)((quotainfo.size.nbytes+1023)/1024),
				(long)((quotainfo.quota.nbytes+1023)/1024));
			strcat(qresult,qbuf);
		}
		if (quotainfo.quota.nmessages > 0)
		{
			sprintf(qbuf,"MESSAGE %d %d",
				quotainfo.size.nmessages,
				quotainfo.quota.nmessages);
			if (strcmp(qresult,"")!=0) strcat(qresult," ");
			strcat(qresult,qbuf);
		}
	}

	writes("* ");
	writes("QUOTA \"");
	writes(qroot);
	writes("\"");
	if (strcmp(qresult,"")!=0)
	{
		writes(" (");
		writes(qresult);
		writes(")");
	};
	writes("\r\n");
}

int is_trash(const char *m)
{
	if (strcmp(m, dot_trash))
	{
		/*
		 * not trying to delete .Trash but folder inside of .Trash
		 */
		return (0);
	}
	else
	{
		/*
		 * trying to delete .Trash - stop them
		 */
		return (1);
	}
}

void emptytrash()
{
	char	*dir, *all_settings, *next_folder, *folder, *p;
	unsigned l;

	all_settings=getenv("IMAP_EMPTYTRASH");

	if (!all_settings)
		return;

	all_settings=strdup(all_settings);
	if (!all_settings)
		return;

	if (strchr(all_settings, ':') == 0 &&
	    strchr(all_settings, ',') == 0)
	{
		l=atoi(all_settings);

		if (l <= 0)
			l=1;

		maildir_getnew(".", trash, NULL, NULL);
		if ((dir=maildir_folderdir(".", trash)))
		{
			maildir_purge(dir, l * 24 * 60 * 60);
			free(dir);
		}
		free(all_settings);
		return;
	}

	for (folder=all_settings; folder && *folder; )
	{
		if (*folder == ',')
		{
			++folder;
			continue;
		}
		next_folder=strchr(folder, ',');
		if (next_folder)
			*next_folder++=0;

		p=strchr(folder, ':');
		if (!p)
		{
			folder=next_folder;
			continue;
		}

		*p++=0;

		l=atoi(p);
		if (l <= 0)	l=1;

		maildir_getnew(".", folder, NULL, NULL);
		if ((dir=maildir_folderdir(".", folder)))
		{
			maildir_purge(dir, l * 24 * 60 * 60);
			free(dir);
		}
		folder=next_folder;
	}
	free(all_settings);
}

#if 0
int is_draft(const char *m)
{
#if 1
	/* Fix some PINE bugs first */

	if (strcmp(m, "." DRAFTS))	return (0);
	return (1);
#else
	return (0);
#endif
}
#endif

int is_reserved(const char *m)
{
	if (strncmp(m, "./", 2) == 0) m += 2;

	if (is_trash(m))	return (1);
	return (0);
}

int is_reserved_name(const char *name)
{
	if (strncmp(name, INBOX, strlen(INBOX)) == 0)
		return is_trash(name+strlen(INBOX));
	return 0;
}

static char *decode_valid_mailbox_utf8(const char *p, int autosubscribe)
{
	struct maildir_info mi;
	char *q, *r;

	if (maildir_info_imap_find(&mi, p, getenv("AUTHENTICATED")) < 0)
	{
		return NULL;
	}

	if (mi.homedir && mi.maildir)
	{
		q=maildir_name2dir(mi.homedir, mi.maildir);

		if (q)
		{
			r=malloc(strlen(q)+sizeof("/."));
			if (!r)	write_error_exit(0);
			strcat(strcpy(r, q), "/.");
			if (access(r, 0) == 0)
			{
				free(r);
				maildir_info_destroy(&mi);
				return q;
			}
			free(r);
			free(q);
		}
		maildir_info_destroy(&mi);
		return NULL;
	}

	if (mi.mailbox_type == MAILBOXTYPE_OLDSHARED)
	{
		const char *q;
		char *r;

		if ((q=strchr(p, '.')) == NULL)
		{
			maildir_info_destroy(&mi);
			errno=EINVAL;
			return NULL;
		}

		r=maildir_shareddir(".", q+1);
		if (!r)
		{
			maildir_info_destroy(&mi);
			errno=EINVAL;
			return NULL;
		}

		if (access(r, 0) == 0)
		{
			maildir_info_destroy(&mi);
			return r;
		}

		maildir_shared_subscribe(".", q+1);
		if (access(r, 0) == 0)
		{
			maildir_info_destroy(&mi);
			return r;
		}

		free(r);
		maildir_info_destroy(&mi);
		return NULL;
	}
	maildir_info_destroy(&mi);
	return (NULL);
}

char *decode_valid_mailbox(const char *mailbox, int autosubscribe)
{
	char *p=imap_foldername_to_filename(enabled_utf8, mailbox);
	char *q;

	if (!p)
	{
		errno=EINVAL;
		return NULL;
	}

	q=decode_valid_mailbox_utf8(p, autosubscribe);
	free(p);
	return q;
}

static int decode_date_time(char *p, time_t *tret)
{
unsigned	i;

	/* Convert to format rfc822_parsedt likes */

	for (i=1; p[i] != ' '; i++)
	{
		if (!p[i])	return (0);
		if (p[i] == '-')	p[i]=' ';
	}
	return (rfc822_parsedate_chk(p, tret));
}

int get_flagname(const char *p, struct imapflags *flags)
{
	if (strcasecmp(p, "\\SEEN") == 0)
		flags->seen=1;
	else if (strcasecmp(p, "\\ANSWERED") == 0)
		flags->answered=1;
	else if (strcasecmp(p, "\\DRAFT") == 0)
		flags->drafts=1;
	else if (strcasecmp(p, "\\DELETED") == 0)
		flags->deleted=1;
	else if (strcasecmp(p, "\\FLAGGED") == 0)
		flags->flagged=1;
	else return (-1);
	return (0);
}

int valid_keyword(const char *kw)
{
	const char *p;

	if (!keywords())
		return 0;

	/* Check for valid keyword names */

	for (p=kw; *p; p++)
	{
		if ((unsigned char)*p <= ' '
		    || strchr(KEYWORD_IMAPVERBOTTEN, *p))
			return 0;
	}
	return 1;
}

int get_keyword(struct libmail_kwMessage **kwPtr, const char *kw)
{
	if (libmail_kwmSetName(current_maildir_info.keywordList, *kwPtr, kw) < 0)
		write_error_exit(0);

	return 0;
}


int get_flagsAndKeywords(struct imapflags *flags,
			 struct libmail_kwMessage **kwPtr)
{
struct imaptoken *t;

	while ((t=nexttoken_nouc())->tokentype == IT_ATOM)
	{
		if (get_flagname(t->tokenbuf, flags))
		{
			if (!valid_keyword(t->tokenbuf))
				return -1;

			if (get_keyword(kwPtr, t->tokenbuf))
				return -1;
		}
	}
	return (t->tokentype == IT_RPAREN ? 0:-1);
}

void get_message_flags(
	struct imapscanmessageinfo *mi,
	char *buf, struct imapflags *flags)
{
	const char *filename=mi->filename;

	const char *DRAFT="\\Draft";
	const char *FLAGGED="\\Flagged";
	const char *REPLIED="\\Answered";
	const char *SEEN="\\Seen";
	const char *DELETED="\\Deleted";
	const char *RECENT="\\Recent";

	const char *SPC=" ";

	if (buf)
		*buf=0;

	if (flags)
		flags->seen=flags->answered=flags->deleted=flags->flagged
		=flags->recent=flags->drafts=0;

	if ((filename=strrchr(filename, MDIRSEP[0])) == 0
		|| strncmp(filename, MDIRSEP "2,", 3))	return;

#if SMAP
	if (smapflag)
	{
		SPC=",";
		DRAFT="DRAFT";
		FLAGGED="MARKED";
		REPLIED="REPLIED";
		SEEN="SEEN";
		DELETED="DELETED";
		RECENT="RECENT";
	}
#endif

	if (strchr(filename, 'D'))
	{
		if (buf) strcat(buf, DRAFT);
		if (flags)  flags->drafts=1;
	}

	if (strchr(filename, 'F'))
	{
		if (buf) strcat(strcat(buf, *buf ? SPC:""), FLAGGED);
		if (flags)	flags->flagged=1;
	}
	if (strchr(filename, 'R'))
	{
		if (buf) strcat(strcat(buf, *buf ? SPC:""), REPLIED);
		if (flags)	flags->answered=1;
	}

	if (strchr(filename, 'S') != NULL)
	{
		if (buf) strcat(strcat(buf, *buf ? SPC:""), SEEN);
		if (flags)	flags->seen=1;
	}

	if (strchr(filename, 'T'))
	{
		if (buf) strcat(strcat(buf, *buf ? SPC:""), DELETED);
		if (flags)	flags->deleted=1;
	}

	if (mi->recentflag)
	{
		if (flags) flags->recent=1;
		if (buf) strcat(strcat(buf, *buf ? SPC:""), RECENT);
	}
}

static char *parse_mailbox_error(const char *tag,
	struct imaptoken *curtoken,
	int ok_hierarchy,	/* RFC 2060 errata - DELETE can take
				** a trailing hierarchy separator if the
				** IMAP server supports subfolders of
				** a real folder (such as this one).
				*/

	int autosubscribe)	/* Really DUMB clients that do a LIST,
				** and don't bother to check if a folder is
				** subscribed to, or not (Pine)
				*/
{
char	*mailbox;

	if (curtoken->tokentype != IT_NUMBER &&
		curtoken->tokentype != IT_ATOM &&
		curtoken->tokentype != IT_QUOTED_STRING)
	{
		writes(tag);
		writes(" BAD Invalid command\r\n");
		return (0);
	} 
	if (ok_hierarchy && (mailbox=strrchr(curtoken->tokenbuf, HIERCH)) && mailbox[1] == 0)
		*mailbox=0;

	mailbox=decode_valid_mailbox(curtoken->tokenbuf, autosubscribe);

	if ( mailbox == 0)
	{
		writes(tag);
		writes(" NO Mailbox does not exist, or must be subscribed to.\r\n");
		return (0);
	}
	return (mailbox);
}

/*
		STORE NEW MESSAGE INTO A MAILBOX
*/

void append_flags(char *buf, struct imapflags *flags)
{
	if (flags->drafts)	strcat(buf, "D");
	if (flags->flagged)	strcat(buf, "F");
	if (flags->answered)	strcat(buf, "R");
	if (flags->seen)	strcat(buf, "S");
	if (flags->deleted)	strcat(buf, "T");
}

	/* First, figure out the filenames used in tmp and new */

FILE *maildir_mkfilename(const char *mailbox, struct imapflags *flags,
			 unsigned long s, char **tmpname, char **newname)
{
	char	*p;
	char	uniqbuf[80];
	static unsigned uniqcnt=0;
	FILE	*fp;
	struct maildir_tmpcreate_info createInfo;

	sprintf(uniqbuf, "%u", uniqcnt++);

	maildir_tmpcreate_init(&createInfo);
	createInfo.openmode=0666;
	createInfo.maildir=mailbox;
	createInfo.uniq=uniqbuf;
	createInfo.msgsize=s;
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((fp=maildir_tmpcreate_fp(&createInfo)) == NULL)
		return NULL;

	*tmpname=createInfo.tmpname;
	*newname=createInfo.newname;

	createInfo.tmpname=NULL;
	createInfo.newname=NULL;
	maildir_tmpcreate_free(&createInfo);

	strcpy(uniqbuf, MDIRSEP "2,");
	append_flags(uniqbuf, flags);

	/* Ok, this message will really go to cur, not new */

	p=malloc(strlen(*newname)+strlen(uniqbuf)+1);
	if (!p)	write_error_exit(0);
	strcpy(p, *newname);
	memcpy(strrchr(p, '/')-3, "cur", 3);	/* HACK OF THE MILLENIA */
	strcat(p, uniqbuf);
	free(*newname);
	*newname=p;
	return fp;
}

void set_time(const char *tmpname, time_t timestamp)
{
#if	HAVE_UTIME
	if (timestamp)
	{
	struct	utimbuf ub;

		ub.actime=ub.modtime=timestamp;
		utime(tmpname, &ub);
	}
#else
#if	HAVE_UTIMES
	if (timestamp)
	{
	struct	timeval	tv;

		tv.tv_sec=timestamp;
		tv.tv_usec=0;
		utimes(tmpname, &tv);
	}
#endif
#endif
}

static int store_mailbox(const char *tag, const char *mailbox,
			 struct	imapflags *flags,
			 struct libmail_kwMessage *keywords,
			 time_t	timestamp,
			 struct imaptoken *curtoken,
			 unsigned long *new_uidv,
			 unsigned long *new_uid,
 			 int *utf8_error)
{
	unsigned long nbytes=curtoken->tokennum;
	char	*tmpname;
	char	*newname;
	char	*p, *q;
	char    *e;
	FILE	*fp;
	unsigned long n;
	static const char nowrite[]=" NO [ALERT] Cannot create message - no write permission or out of disk space.\r\n";
	int	lastnl;
	int     rb;
	int	errflag;
	struct rfc2045 *rfc2045_parser;
	const char *errmsg=nowrite;

	fp=maildir_mkfilename(mailbox, flags, 0, &tmpname, &newname);

	if (!fp)
	{
		writes(tag);
		writes(nowrite);
		return -1;
	}

	writes("+ OK\r\n");
	writeflush();
	lastnl=0;

	current_temp_fd=fileno(fp);
	current_temp_fn=tmpname;

	rfc2045_parser=rfc2045_alloc();

	while (nbytes)
	{
		read_string(&p, &n, nbytes);
		nbytes -= n;
		if (p[n-1] == '\n') lastnl = 1;
		else lastnl = 0;

		while (n)
		{
			e = memchr(p, '\r', n);
			if (e && p == e)
			{
				rb=1;
			}
			else if (e)
			{
				rfc2045_parse(rfc2045_parser, p, e-p);
				rb = fwrite(p, 1, e-p, fp);
			}
			else
			{
				rfc2045_parse(rfc2045_parser, p, n);
				rb = fwrite(p, 1, n, fp);
			}
			n -= rb;
			p += rb;
		}
	}
	if (!lastnl)
	{
		putc('\n', fp);
		rfc2045_parse(rfc2045_parser, "\n", 1);
	}

	current_temp_fd=-1;
	current_temp_fn=NULL;
	errflag=0;

	if (fflush(fp) || ferror(fp))
	{
		fprintf(stderr, "ERR: error storing a message, user=%s, errno=%d\n",
                                getenv("AUTHENTICATED"), errno);
		errflag=1;
	}

	q = getenv("ENABLE_UTF8_COMPLIANCE");
	if ((q && *q) &&(rfc2045_parser->rfcviolation & RFC2045_ERR8BITHEADER) &&
	    curtoken->tokentype != IT_LITERAL8_STRING_START)
	{
		/* in order to [ALERT] the client */
		*utf8_error=1;
	}

	rfc2045_free(rfc2045_parser);

	if (errflag)
	{
		fclose(fp);
		unlink(tmpname);
		writes(tag);
		writes(errmsg);
		free(tmpname);
		free(newname);
		return (-1);
	}

	nbytes=ftell(fp);
	if (nbytes == (unsigned long)-1 ||
		(p=maildir_requota(newname, nbytes)) == 0)

	{
		fclose(fp);
		unlink(tmpname);
		writes(tag);
		writes(nowrite);
		free(tmpname);
		free(newname);
		return (-1);
	}

	free(newname);

	fclose(fp);

	if (maildirquota_countfolder(mailbox) &&
	    maildirquota_countfile(p))
	{
		struct maildirsize quotainfo;

		if (maildir_quota_add_start(mailbox, &quotainfo, nbytes, 1,
					    getenv("MAILDIRQUOTA")))
		{
			unlink(tmpname);
			free(tmpname);
			writes(tag);
			writes(" NO [ALERT] You exceeded your mail quota.\r\n");
			return (-1);
		}
		maildir_quota_add_end(&quotainfo, nbytes, 1);
	}

	if (check_outbox(tmpname, mailbox))
	{
		unlink(tmpname);
		writes(tag);
		writes(" NO [ALERT] Unable to send E-mail message.\r\n");
		free(tmpname);
		return (-1);
	}

	{
		struct imapscaninfo new_maildir_info;
		struct uidplus_info new_uidplus_info;
		int rc;

		imapscan_init(&new_maildir_info);

		memset(&new_uidplus_info, 0, sizeof(new_uidplus_info));

		new_uidplus_info.tmpfilename=tmpname;
		new_uidplus_info.curfilename=p;
		new_uidplus_info.mtime=timestamp;

		if (keywords && keywords->firstEntry)
		{
			if (maildir_kwSave(mailbox,
					   strrchr(p, '/')+1,
					   keywords,
					   &new_uidplus_info.tmpkeywords,
					   &new_uidplus_info.newkeywords,
					   0))
			{
				unlink(tmpname);
				writes(tag);
				writes(" NO [ALERT] ");
				writes(strerror(errno));
				free(tmpname);
				return (-1);
			}
		}

		rc=imapscan_maildir(&new_maildir_info, mailbox, 0, 0,
				    &new_uidplus_info);

		if (new_uidplus_info.tmpkeywords)
			free(new_uidplus_info.tmpkeywords);

		if (new_uidplus_info.newkeywords)
			free(new_uidplus_info.newkeywords);

		if (rc)
		{
			free(tmpname);
			writes(tag);
			writes(nowrite);
			return -1;
		}

		*new_uidv=new_maildir_info.uidv;
		*new_uid=new_uidplus_info.uid;
		imapscan_free(&new_maildir_info);
	}

	free(tmpname);
	return (0);
}


/************** Create and delete folders **************/

#if 0
static int checksubdir(const char *s)
{
DIR	*dirp;
struct	dirent *de;

	dirp=opendir(s);
	while (dirp && (de=readdir(dirp)) != 0)
		if (de->d_name[0] != '.')
		{
			closedir(dirp);
			return (1);
		}
	if (dirp)	closedir(dirp);
	return (0);
}
#endif

int mddelete(const char *s)
{
	int	rc;
	struct stat stat_buf;

	/* If the top level maildir is a sym link, don't delete it */

	if (stat(s, &stat_buf) < 0 &&
	    S_ISLNK(stat_buf.st_mode))
		return -1;

	trap_signals();
	rc=maildir_del(s);
	if (release_signals())	_exit(0);
	return (rc);
}

int mdcreate(const char *mailbox)
{
	trap_signals();
	if (maildir_make(mailbox, 0700, 0700, 1) < 0)
	{
		if (release_signals())	_exit(0);
		return (-1);
	}

	if (release_signals())	_exit(0);
	return (0);
}

/****************************************************************************/

/* do_msgset parses a message set, and calls a processing func for each msg */

/****************************************************************************/

static int do_msgset(const char *msgset,
	int (*msgfunc)(unsigned long, int, void *),
	void *msgfunc_arg, int isuid)
{
unsigned long i, j;
int	rc;
unsigned long last=0;

	if (current_maildir_info.nmessages > 0)
	{
		last=current_maildir_info.nmessages;
		if (isuid)
		{
			last=current_maildir_info.msgs[last-1].uid;
		}
	}

	while (isdigit((int)(unsigned char)*msgset) || *msgset == '*')
	{
		i=0;
		if (*msgset == '*')
		{
			i=last;
			++msgset;
		}
		else while (isdigit((int)(unsigned char)*msgset))
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
				j=last;
				++msgset;
			}
			else while (isdigit((int)(unsigned char)*msgset))
			{
				j=j*10 + (*msgset++-'0');
			}
		}
		if (j < i)
		{
#if 0
	/* BUGS issue */
			writes("* NO Invalid message set: ");
			writen(i);
			writes(":");
			writen(j);
			writes("\r\n");
#endif
		}
		else if (isuid)
		{
		unsigned long k;

			for (k=0; k<current_maildir_info.nmessages; k++)
				if (current_maildir_info.msgs[k].uid >= i)
					break;
			if (k >= current_maildir_info.nmessages ||
				current_maildir_info.msgs[k].uid > j)
			{
#if 0
	/* BUGS issue */
				writes("* NO Invalid message: UID ");
				writen(i);
				if (j > i)
				{
					writes(":");
					writen(j);
				}
				writes("\r\n");
#endif
			}
			else while (k < current_maildir_info.nmessages &&
				current_maildir_info.msgs[k].uid <= j)
			{
				if ((rc=(*msgfunc)(k+1, 1, msgfunc_arg)) != 0)
					return (rc);
				++k;
			}
		}
		else
		{
			do
			{
				if (i == 0 ||
				    i > current_maildir_info.nmessages)
				{
					writes("* NO Invalid message sequence number: ");
					writen(i);
					writes("\r\n");
					break;
				}

				if ((rc=(*msgfunc)(i, 0, msgfunc_arg)) != 0)
					return (rc);
			} while (i++ < j);
		}

		if (*msgset++ != ',')	break;
	}
	return (0);
}

/** Show currently defined flags and keywords **/

static int write_keyword_name(struct libmail_keywordEntry *, void *);

static void mailboxflags(int ro)
{
#if SMAP
	if (smapflag)
		return;
#endif

	writes("* FLAGS (");

	if (current_maildir_info.keywordList)
	{
		void (*writefunc)(const char *)=writes;

		libmail_kwEnumerate(current_maildir_info.keywordList,
				    &write_keyword_name, &writefunc);
	}

	writes("\\Draft \\Answered \\Flagged"
	       " \\Deleted \\Seen \\Recent)\r\n");
	writes("* OK [PERMANENTFLAGS (");


	if (ro)
	{
		writes(")] No permanent flags permitted\r\n");
	}
	else
	{
		if (current_maildir_info.keywordList)
		{
			void (*writefunc)(const char *)=writes;

			libmail_kwEnumerate(current_maildir_info
					    .keywordList,
					    &write_keyword_name,
					    &writefunc);
		}

		if (keywords())
			writes("\\* ");

		writes("\\Draft \\Answered \\Flagged \\Deleted \\Seen)] Limited\r\n");
	}
}

static int write_keyword_name(struct libmail_keywordEntry *kw, void *dummy)
{
	void (**writefunc)(const char *)=(void (**)(const char *))dummy;

	(**writefunc)(keywordName(kw));
	(**writefunc)(" ");
	return 0;
}

/****************************************************************************/

/* Show how many messages are in the mailbox                                */

/****************************************************************************/

static void mailboxmetrics()
{
unsigned long i,j;

#if SMAP
	if (smapflag)
	{
		writes("* EXISTS ");
		writen(current_maildir_info.nmessages);
		writes("\n");
		return;
	}
#endif


	writes("* ");
	writen(current_maildir_info.nmessages);
	writes(" EXISTS\r\n");

	writes("* ");
	i=0;

	for (j=0; j<current_maildir_info.nmessages; j++)
		if (current_maildir_info.msgs[j].recentflag)
			++i;
	writen(i);
	writes(" RECENT\r\n");
}

/****************************************************************************/

/* Do the NOOP stuff                                                        */

/****************************************************************************/

struct noopKeywordUpdateInfo {
	struct libmail_kwHashtable *keywordList;
	struct libmail_kwMessage *messagePtr;
};

static int noopAddKeyword(struct libmail_keywordEntry *ke, void *vp)
{
	struct noopKeywordUpdateInfo *kui=
		(struct noopKeywordUpdateInfo *)vp;

	libmail_kwmSetName(kui->keywordList, kui->messagePtr, keywordName(ke));
	/*
	** ke originates from a different keyword namespace, so must use
	** its name.
	*/
	return 0;
}

void doNoop(int real_noop)
{
	struct imapscaninfo new_maildir_info;
	unsigned long i, j;
	int	needstats=0;
	unsigned long expunged;
#if SMAP
	unsigned long smap_expunge_count=0;
	unsigned long smap_expunge_bias=0;

	unsigned long smap_last=0;
	unsigned long smap_range=0;

	int takeSnapshot=1;
#endif
	struct noopKeywordUpdateInfo kui;

	imapscan_init(&new_maildir_info);

	if (imapscan_maildir(&new_maildir_info, current_mailbox, 0,
			     current_mailbox_ro, NULL))
		return;

	j=0;
	expunged=0;
	for (i=0; i<current_maildir_info.nmessages; i++)
	{
		struct libmail_kwMessage *a, *b;

		while (j < new_maildir_info.nmessages &&
			new_maildir_info.msgs[j].uid <
				current_maildir_info.msgs[i].uid)
		{
			/* How did this happen??? */

			new_maildir_info.msgs[j].changedflags=1;
			++j;
			needstats=1;
#if SMAP
			takeSnapshot=0;
#endif
		}

		if (j >= new_maildir_info.nmessages ||
			new_maildir_info.msgs[j].uid >
				current_maildir_info.msgs[i].uid)
		{
#if SMAP
			if (smapflag)
			{
				takeSnapshot=0;

				if (smap_expunge_count > 100)
				{
					if (smap_range > 0)
					{
						writes("-");
						writen(smap_last + smap_range
						       - smap_expunge_bias
						       + 1);
					}
					writes("\n");
					smap_expunge_count=0;
				}

				if (smap_expunge_count == 0)
				{
					smap_expunge_bias=expunged;

					writes("* EXPUNGE ");
					writen(i+1-smap_expunge_bias);
					smap_last=i;
					smap_range=0;
				}
				else if (smap_last + smap_range + 1 == i)
				{
					++smap_range;
				}
				else
				{
					if (smap_range > 0)
					{
						writes("-");
						writen(smap_last + smap_range
						       - smap_expunge_bias
						       + 1);
					}
					writes(" ");
					writen(i+1-smap_expunge_bias);
					smap_last=i;
					smap_range=0;
				}
				++smap_expunge_count;
			}
			else
#endif
			{
				writes("* ");
				writen(i+1-expunged);
				writes(" EXPUNGE\r\n");
				needstats=1;
			}

			++expunged;
			continue;
		}

		/* Must be the same */

		a=current_maildir_info.msgs[i].keywordMsg;
		b=new_maildir_info.msgs[j].keywordMsg;

		if (strcmp(current_maildir_info.msgs[i].filename,
			new_maildir_info.msgs[j].filename) ||
		    (a && !b) || (!a && b) ||
		    (a && b && libmail_kwmCmp(a, b)))
		{
			new_maildir_info.msgs[j].changedflags=1;
#if SMAP
			takeSnapshot=0;
#endif
		}
		if (current_maildir_info.msgs[i].recentflag)
			new_maildir_info.msgs[j].recentflag=1;
#if SMAP
		if (smapflag)
			new_maildir_info.msgs[j].recentflag=0;
#endif
		new_maildir_info.msgs[j].copiedflag=
			current_maildir_info.msgs[i].copiedflag;
		new_maildir_info.msgs[j].err8bitflag=
			current_maildir_info.msgs[i].err8bitflag;
		++j;
	}

#if SMAP
	if (smapflag && smap_expunge_count)
	{
		if (smap_range > 0)
		{
			writes("-");
			writen(smap_last + smap_range - smap_expunge_bias + 1);
		}
		writes("\n");
	}

#endif

	while (j < new_maildir_info.nmessages)
	{
#if SMAP
		if (smapflag)
			takeSnapshot=0;
#endif
		++j;
		needstats=1;
	}

	new_maildir_info.keywordList->keywordAddedRemoved=0;


	/**********************************************************
	 **
	 ** current_maildir_info: existing keywords
	 ** new_maildir_info: new keywords
	 **
	 ** Process new/deleted keywords as follows:
	 */

	/*
	** 1. Make sure that the old keyword list includes any new keywords.
	*/

	kui.keywordList=current_maildir_info.keywordList;
	kui.messagePtr=libmail_kwmCreate();

	if (!kui.messagePtr)
		write_error_exit(0);

	current_maildir_info.keywordList->keywordAddedRemoved=0;
	libmail_kwEnumerate(new_maildir_info.keywordList,
			 noopAddKeyword, &kui);

	if (current_maildir_info.keywordList->keywordAddedRemoved)
		mailboxflags(current_mailbox_ro);
	libmail_kwmDestroy(kui.messagePtr);


	/*
	** 2. Temporarily add all existing keywords to the new keyword list.
	*/

	kui.keywordList=new_maildir_info.keywordList;
	kui.messagePtr=libmail_kwmCreate();

	if (!kui.messagePtr)
		write_error_exit(0);
	libmail_kwEnumerate(current_maildir_info.keywordList,
			 noopAddKeyword, &kui);

	imapscan_copy(&current_maildir_info, &new_maildir_info);
	imapscan_free(&new_maildir_info);

#if SMAP
	if (takeSnapshot)
	{
		if (real_noop && smapflag)
			snapshot_save();
	}
	else
		snapshot_needed();
#endif

	if (needstats)
		mailboxmetrics();

	for (j=0; j < current_maildir_info.nmessages; j++)
		if (current_maildir_info.msgs[j].changedflags)
		{
#if SMAP
			if (smapflag)
				smap_fetchflags(j);
			else
#endif
				fetchflags(j);
		}

	/*
	** After sending changed flags to the client, see if any keywords
	** have gone away.
	*/

	current_maildir_info.keywordList->keywordAddedRemoved=0;
	libmail_kwmDestroy(kui.messagePtr);
	if (current_maildir_info.keywordList->keywordAddedRemoved)
		mailboxflags(current_mailbox_ro);
}

static char *alloc_filename(const char *mbox, const char *name)
{
char	*p=malloc(strlen(mbox)+strlen(name)+sizeof("/cur/"));

	if (!p)	write_error_exit(0);

	strcat(strcat(strcpy(p, mbox), "/cur/"), name);
	return (p);
}

/****************************************************************************/

/* Do the ID stuff                                                        */

/****************************************************************************/
static int doId()
{
	const char *ev = getenv("IMAP_ID_FIELDS");
	unsigned int flags=0;
	struct	imaptoken *curtoken;

	if (!ev)
		return -1;

	flags = atoi(ev);

	/* The data sent by the client isn't used for anything, but make sure
	 * it is syntactically correct. */
	curtoken = nexttoken();
	switch (curtoken->tokentype) {
	case IT_NIL:
		break;
	case IT_LPAREN:
		{
		unsigned int k = 0;

		curtoken = nexttoken();

		fprintf(stderr, "INFO: ID, user=%s, ip=[%s]",
			getenv("AUTHENTICATED"),
			getenv("TCPREMOTEIP"));

		while ((k < 30) && (curtoken->tokentype != IT_RPAREN)) {
			k++;
			if (curtoken->tokentype != IT_QUOTED_STRING)
			{
				fprintf(stderr, "\n");
				fflush(stderr);
				return -1;
			}
			fprintf(stderr, ", %s=", curtoken->tokenbuf);

			curtoken = nexttoken();
			if ((curtoken->tokentype != IT_QUOTED_STRING) &&
					(curtoken->tokentype != IT_NIL))
			{
				fprintf(stderr, "\n");
				fflush(stderr);
				return -1;
			}
			fprintf(stderr, "%s",
				curtoken->tokentype == IT_QUOTED_STRING
				? curtoken->tokenbuf:"(nil)");
			curtoken = nexttoken();
		}
		fprintf(stderr, "\n");
		fflush(stderr);

		/* no strings sent */
		if (k == 0)
			return -1;

		/* at most 30 pairs allowed */
		if ((k >= 30) && (curtoken->tokentype != IT_RPAREN))
			return -1;

		break;
		}
	default:
		return -1;
	}

	writes("* ID (\"name\" \"Courier-IMAP");

	if (flags & 1) {
		const char *sp = strchr(PROGRAMVERSION, ' ') + 1;
		writes("\" \"version\" \"");
		writemem(sp, strchr(sp, '/') - sp);
	}

#if HAVE_SYS_UTSNAME_H
	if (flags & 6) {
		struct utsname uts;
		if (uname(&uts) == 0)
		{
			if (flags & 2) {
				writes("\" \"os\" \"");
				writeqs(uts.sysname);
			}
			if (flags & 4) {
				writes("\" \"os-version\" \"");
				writeqs(uts.release);
			}

		}
	}
#endif

	writes("\" \"vendor\" \"Double Precision, Inc.\")\r\n");

	return 0;
}

/****************************************************************************/

/* Do the IDLE stuff                                                        */

/****************************************************************************/

extern int doidle(time_t, int);

int imapenhancedidle(void)
{
	struct maildirwatch *w;
	struct maildirwatch_contents c;
	int rc;
	int started=0;

	if (!current_mailbox)
		return (-1);

	if ((w=maildirwatch_alloc(current_mailbox)) == NULL)
	{
		perror(current_mailbox);
		fprintf(stderr, "This may be a problem with FAM or Gamin\n");
		return (-1);
	}

	rc=maildirwatch_start(w, &c);

	if (rc < 0)
	{
		perror("maildirwatch_start");
		maildirwatch_free(w);
		return (-1);
	}

	if (rc > 0)
	{
		maildirwatch_free(w);
		return (-1); /* Fallback */
	}

#if SMAP
	if (smapflag)
	{
		writes("+OK Entering ENHANCED idle mode\n");
	}
	else
#endif
		writes("+ entering ENHANCED idle mode\r\n");
	writeflush();

	for (;;)
	{
		if (!started)
		{
			int fd;
			int rc;

			rc=maildirwatch_started(&c, &fd);

			if (rc > 0)
			{
				started=1;
				doNoop(0);
				writeflush();
			}
			else
			{
				if (rc < 0)
					perror("maildirwatch_started");
				if (doidle(60, fd))
					break;
			}
			continue;
		}

		if (started < 0) /* Error fallback mode*/
		{
			if (doidle(60, -1))
				break;
		}
		else
		{
			int changed;
			int fd;
			int timeout;

			if (maildirwatch_check(&c, &changed, &fd, &timeout)
			    == 0)
			{
				if (!changed)
				{
					if (doidle(timeout, fd))
						break;
					continue;
				}

				maildirwatch_end(&c);
				doNoop(0);

				rc=maildirwatch_start(w, &c);

				if (rc < 0)
				{
					perror("maildirwatch_start");
					started= -1;
				}
				else
					started=0;
			}
			else
			{
				started= -1;
			}
		}

		doNoop(0);
		writeflush();
	}

	maildirwatch_end(&c);
	maildirwatch_free(w);
	return (0);
}

void imapidle(void)
{
	const char * envp = getenv("IMAP_IDLE_TIMEOUT");
	const int idleTimeout = envp ? atoi(envp) : 60;

#if SMAP
	if (smapflag)
	{
		writes("+OK Entering idle mode...\n");
	}
	else
#endif
		writes("+ entering idle mode\r\n");
	if (current_mailbox)
		doNoop(0);
	writeflush();
	while (!doidle(idleTimeout, -1))
	{
		if (current_mailbox)
			doNoop(0);
		writeflush();
	}
}

/****************************************************************************/

/* Do the EXPUNGE stuff                                                     */

/****************************************************************************/

void do_expunge(unsigned long from, unsigned long to, int force);

static int uid_expunge(unsigned long msgnum, int uidflag, void *void_arg)
{
	do_expunge(msgnum-1, msgnum, 0);
	return 0;
}

void expunge()
{
	do_expunge(0, current_maildir_info.nmessages, 0);
}

void do_expunge(unsigned long expunge_start,
		unsigned long expunge_end,
		int force)
{
unsigned long j;
struct imapflags flags;
char	*old_name;
int	move_to_trash=0;
struct stat stat_buf;
const char *cp=getenv("IMAP_LOG_DELETIONS");
int log_deletions= cp && *cp != '0';

	fetch_free_cache();

	if (magictrash() &&
	    !is_trash(strncmp(current_mailbox, "./", 2) == 0?
		      current_mailbox+2:current_mailbox))
		move_to_trash=1;

	for (j=expunge_start; j < expunge_end; j++)
	{
		int file_counted=0;

		get_message_flags(current_maildir_info.msgs+j, 0, &flags);

		if (!flags.deleted && !force)
			continue;

		old_name=alloc_filename(current_mailbox,
			current_maildir_info.msgs[j].filename);

		if (stat(old_name, &stat_buf) < 0)
		{
			free(old_name);
			continue;
		}

		if (maildirquota_countfolder(current_mailbox) &&
		    maildirquota_countfile(old_name))
			file_counted=1;

		if (is_sharedsubdir(current_mailbox))
		{
			maildir_unlinksharedmsg(old_name);
		}
		else if (move_to_trash && current_maildir_info
			 .msgs[j].copiedflag == 0)
		{
		char	*new_name;
		char	*p;
		int will_count=0;

			new_name=alloc_filename(dot_trash,
				current_maildir_info.msgs[j].filename);

			if (maildirquota_countfolder(dot_trash))
				will_count=1;

			if (file_counted != will_count)
			{
				unsigned long filesize=0;

				if (maildir_parsequota(old_name, &filesize))
				{
					if (stat(old_name, &stat_buf))
						stat_buf.st_size=0;
					filesize=(unsigned long)
						stat_buf.st_size;
				}

				maildir_quota_deleted(".",
						      (long)filesize *
						      (will_count
						       -file_counted),
						      will_count
						      -file_counted);
			}

			if ((p=strrchr(new_name, '/')) &&
			    (p=strrchr(p, MDIRSEP[0])) &&
			    strncmp(p, MDIRSEP "2,", 3) == 0)
			{
				char *q;

				/* Don't mark it as deleted in the Trash */

				if ((q=strchr(p, 'T')) != NULL)
					while ((*q=q[1]) != 0)
						++q;

				/* Don't mark it as a draft msg in the Trash */

				if ((q=strchr(p, 'D')) != NULL)
					while ((*q=q[1]) != 0)
						++q;
			}

			if (log_deletions)
				fprintf(stderr, "INFO: EXPUNGED, user=%s, ip=[%s], old_name=%s, new_name=%s: only new_name will be left\n",
					getenv("AUTHENTICATED"),
					getenv("TCPREMOTEIP"),
					old_name, new_name);

			if (rename(old_name, new_name))
			{
				mdcreate(dot_trash);
				rename(old_name, new_name);
			}

			unlink(old_name);
			/* triggers linux kernel bug if also moved to Trash by
			sqwebmail */

			free(new_name);
		}
		else
		{
			unlink(old_name);

			if (file_counted)
			{
				unsigned long filesize=0;

				if (maildir_parsequota(old_name, &filesize))
				{
					if (stat(old_name, &stat_buf))
						stat_buf.st_size=0;
					filesize=(unsigned long)
						stat_buf.st_size;
				}

				maildir_quota_deleted(".",-(long)filesize, -1);
			}
		}

		if (log_deletions)
			fprintf(stderr, "INFO: EXPUNGED, user=%s, ip=[%s], old_name=%s\n",
				getenv("AUTHENTICATED"),
				getenv("TCPREMOTEIP"),
				old_name);
		free(old_name);
	}
}


static FILE *newsubscribefile(char **tmpname)
{
	char    uniqbuf[80];
	static  unsigned tmpuniqcnt=0;
	FILE	*fp;
	struct maildir_tmpcreate_info createInfo;

	maildir_tmpcreate_init(&createInfo);

	sprintf(uniqbuf, "imapsubscribe%u", tmpuniqcnt++);

	createInfo.uniq=uniqbuf;
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((fp=maildir_tmpcreate_fp(&createInfo)) == NULL)
		write_error_exit(0);

	*tmpname=createInfo.tmpname;
	createInfo.tmpname=NULL;
	maildir_tmpcreate_free(&createInfo);

	return (fp);
}

static int sub_strcmp(const char *a, const char *b)
{
	if (strncasecmp(a, "inbox", 5) == 0 &&
		strncasecmp(b, "inbox", 5) == 0)
	{
		a += 5;
		b += 5;
	}
	return (strcmp(a, b));
}

static void subscribe(const char *f)
{
	char *newf;
	FILE *newfp=newsubscribefile(&newf);
	FILE *oldfp;

	if ((oldfp=fopen(SUBSCRIBEFILE, "r")) != 0)
	{
	char	buf[BUFSIZ];

		while (fgets(buf, sizeof(buf), oldfp) != 0)
		{
		char *p=strchr(buf, '\n');

			if (p)	*p=0;
			if (sub_strcmp(buf, f) == 0)
			{
				fclose(oldfp);
				fclose(newfp);
				unlink(newf);
				free(newf);
				return;	/* Already subscribed */
			}
			fprintf(newfp, "%s\n", buf);
		}
		fclose(oldfp);
	}
	fprintf(newfp, "%s\n", f);
	if (fflush(newfp) || ferror(newfp))
		write_error_exit(0);
	fclose(newfp);
	rename(newf, SUBSCRIBEFILE);
	free(newf);
}

static void unsubscribe(const char *f)
{
	char *newf;
	FILE *newfp=newsubscribefile(&newf);
	FILE *oldfp;

	if ((oldfp=fopen(SUBSCRIBEFILE, "r")) != 0)
	{
	char	buf[BUFSIZ];

		while (fgets(buf, sizeof(buf), oldfp) != 0)
		{
		char *p=strchr(buf, '\n');

			if (p)	*p=0;
			if (sub_strcmp(buf, f) == 0)
				continue;
			fprintf(newfp, "%s\n", buf);
		}
		fclose(oldfp);
	}
	if (fflush(newfp) || ferror(newfp))
		write_error_exit(0);
	fclose(newfp);
	rename(newf, SUBSCRIBEFILE);
	free(newf);
}

/*
** Count selected messages (if there's >1 copy to OUTBOX should fail).
*/

static int do_count(unsigned long n, int byuid, void *voidptr)
{
	const char *p=getenv("OUTBOX_MULTIPLE_SEND");

	++ *(int *)voidptr;

	if (p && atoi(p))
		*(int *)voidptr=1; /* Suppress the error, below */

	return 0;
}

static void dirsync(const char *folder)
{
#if EXPLICITDIRSYNC

	char *p=malloc(strlen(folder)+sizeof("/new"));
	int fd;

	if (!p)
		write_error_exit(0);

	p=strcat(strcpy(p, folder), "/new");

	fd=open(p, O_RDONLY);

	if (fd >= 0)
	{
		fsync(fd);
		close(fd);
	}

	p=strcat(strcpy(p, folder), "/cur");

	fd=open(p, O_RDONLY);

	if (fd >= 0)
	{
		fsync(fd);
		close(fd);
	}

	free(p);
#endif
}

/*
** Keyword updates for +FLAGS and -FLAGS
*/

static int addRemoveKeywords1(void *);

static int addRemoveKeywords2(int (*callback_func)(void *, void *),
			      void *callback_func_arg,
			      struct storeinfo *storeinfo_s,
			      int *tryagain);

struct addremove_info {
	int (*callback_func)(void *, void *);
	void *callback_func_arg;
	struct storeinfo *storeinfo_s;
	int *tryagain;
};


int addRemoveKeywords(int (*callback_func)(void *, void *),
		      void *callback_func_arg,
		      struct storeinfo *storeinfo_s)
{
	int tryagain;
	struct addremove_info ai;

	if (!keywords())
		return 0;

	if (current_mailbox_acl &&
	    strchr(current_mailbox_acl, ACL_WRITE[0]) == NULL)
		return 0; /* No permission */
	do
	{
		ai.callback_func=callback_func;
		ai.callback_func_arg=callback_func_arg;
		ai.storeinfo_s=storeinfo_s;
		ai.tryagain= &tryagain;

		if (imapmaildirlock(&current_maildir_info,
				    current_mailbox,
				    addRemoveKeywords1,
				    &ai))
			return -1;
	} while (tryagain);

	return 0;
}

static int addRemoveKeywords1(void *void_arg)
{
	struct addremove_info *ai=(struct addremove_info *)void_arg;

	return addRemoveKeywords2(ai->callback_func,
				  ai->callback_func_arg,
				  ai->storeinfo_s,
				  ai->tryagain);
}

int doAddRemoveKeywords(unsigned long, int, void *);

struct addRemoveKeywordInfo {
	struct libmail_kwGeneric kwg;
	struct storeinfo *storeinfo;
};

static int addRemoveKeywords2(int (*callback_func)(void *, void *),
			      void *callback_func_arg,
			      struct storeinfo *storeinfo_s,
			      int *tryagain)
{
	struct addRemoveKeywordInfo arki;
	int rc;

	*tryagain=0;

	libmail_kwgInit(&arki.kwg);

	arki.storeinfo=storeinfo_s;

	rc=libmail_kwgReadMaildir(&arki.kwg, current_mailbox);

	if (rc == 0)
		rc= (*callback_func)(callback_func_arg, &arki);

	if (rc < 0)
	{
		libmail_kwgDestroy(&arki.kwg);
		return -1;
	}

	if (rc > 0) /* Race */
	{
		libmail_kwgDestroy(&arki.kwg);
		*tryagain=1;
		return 0;
	}

	libmail_kwgDestroy(&arki.kwg);
	return 0;
}

int doAddRemoveKeywords(unsigned long n, int uid, void *vp)
{
	struct addRemoveKeywordInfo *arki=
		(struct addRemoveKeywordInfo *)vp;
	struct libmail_kwGenericEntry *ge=
		libmail_kwgFindByName(&arki->kwg,
				      current_maildir_info.msgs[--n].filename);
	char *tmpname=NULL, *newname=NULL;
	struct stat stat_buf;
	int rc;

	if (!ge || ge->keywords == NULL)
	{
		if (arki->storeinfo->plusminus == '+')
		{
			rc=maildir_kwSave(current_mailbox,
					  current_maildir_info.msgs[n].
					  filename,
					  arki->storeinfo->keywords,
					  &tmpname, &newname, 1);

			if (rc < 0)
				return -1;
		}
	}
	else if (arki->storeinfo->plusminus == '+')
	{
		int flag=0;
		struct libmail_kwMessageEntry *kme;

		for (kme=arki->storeinfo->keywords->firstEntry;
		     kme; kme=kme->next)
		{
			if ((rc=libmail_kwmSet(ge->keywords,
					       kme->libmail_keywordEntryPtr))
			    < 0)
			{
				write_error_exit(0);
				return 0;
			}

			if (rc == 0)
				flag=1;
		}

		if (flag)
		{
			rc=maildir_kwSave(current_mailbox,
					  current_maildir_info.msgs[n].
					  filename,
					  ge->keywords,
					  &tmpname, &newname, 1);

			if (rc < 0)
				return -1;
		}
	}
	else
	{
		int flag=0;
		struct libmail_kwMessageEntry *kme;

		for (kme=arki->storeinfo->keywords->firstEntry;
		     kme; kme=kme->next)
		{
			struct libmail_keywordEntry *kwe;

			if ((kwe=libmail_kweFind(&arki->kwg.kwHashTable,
						 keywordName(kme->
							     libmail_keywordEntryPtr),
						 0)) != NULL &&

			    libmail_kwmClear(ge->keywords, kwe) == 0)
				flag=1;
		}

		if (flag)
		{
			rc=maildir_kwSave(current_mailbox,
					  current_maildir_info.msgs[n].
					  filename,
					  ge->keywords,
					  &tmpname, &newname, 1);

			if (rc < 0)
				return -1;
		}


	}

	if (tmpname)
	{
		current_maildir_info.msgs[n].changedflags=1;

		if (link(tmpname, newname) < 0)
		{
			unlink(tmpname);
			free(tmpname);
			free(newname);

			return (errno == EEXIST ? 1:-1);
		}

		if (stat(tmpname, &stat_buf) == 0 &&
		    stat_buf.st_nlink == 2)
		{
			unlink(tmpname);
			free(tmpname);
			free(newname);
			return 0;
		}
		unlink(tmpname);
		free(tmpname);
		free(newname);
		return 1;
	}

	return 0;
}

/* IMAP interface to add/remove keyword */

struct imap_addRemoveKeywordInfo {
	char *msgset;
	int uid;
};

static int markmessages(unsigned long n, int i, void *dummy)
{
	--n;
	current_maildir_info.msgs[n].storeflag=1;
	return 0;
}

static int imap_addRemoveKeywords(void *myVoidArg, void *addRemoveVoidArg)
{
	struct imap_addRemoveKeywordInfo *i=
		(struct imap_addRemoveKeywordInfo *)myVoidArg;
	unsigned long j;

	for (j=0; j<current_maildir_info.nmessages; j++)
		current_maildir_info.msgs[j].storeflag=0;

	do_msgset(i->msgset, markmessages, NULL, i->uid);

	for (j=0; j<current_maildir_info.nmessages; j++)
	{
		int rc;

		if (!current_maildir_info.msgs[j].storeflag)
			continue;

		rc=doAddRemoveKeywords(j+1, i->uid, addRemoveVoidArg);
		if (rc)
			return rc;
	}
	return 0;
}

/*
** After adding messages to a maildir, compute their new UIDs.
*/

static int uidplus_fill(const char *mailbox,
			struct uidplus_info *uidplus_list,
			unsigned long *uidv)
{
	struct imapscaninfo scan_info;

	imapscan_init(&scan_info);

	if (imapscan_maildir(&scan_info, mailbox, 0, 0, uidplus_list))
		return -1;

	*uidv=scan_info.uidv;

	imapscan_free(&scan_info);
	return 0;
}

static void uidplus_writemsgset(struct uidplus_info *uidplus_list,
				int new_uids)
{
#define UIDN(u) ( new_uids ? (u)->uid:(u)->old_uid )

	unsigned long uid_start, uid_end;
	const char *comma="";

	writes(" ");
	while (uidplus_list)
	{
		uid_start=UIDN(uidplus_list);
		uid_end=uid_start;

		while (uidplus_list->next &&
		       UIDN(uidplus_list->next) == uid_end + 1)
		{
			uidplus_list=uidplus_list->next;
			++uid_end;
		}

		writes(comma);
		writen(uid_start);
		if (uid_end != uid_start)
		{
			writes(":");
			writen(uid_end);
		}
		comma=",";
		uidplus_list=uidplus_list->next;
	}

#undef UIDN
}

static void uidplus_free(struct uidplus_info *uidplus_list)
{
	struct uidplus_info *u;

	while ((u=uidplus_list) != NULL)
	{
		uidplus_list=u->next;
		free(u->tmpfilename);
		free(u->curfilename);

		if (u->tmpkeywords)
			free(u->tmpkeywords);
		if (u->newkeywords)
			free(u->newkeywords);
		free(u);
	}
}

/* Abort a partially-filled uidplus */

static void uidplus_abort(struct uidplus_info *uidplus_list)
{
	struct uidplus_info *u;

	while ((u=uidplus_list) != NULL)
	{
		uidplus_list=u->next;
		unlink(u->tmpfilename);
		unlink(u->curfilename);

		if (u->tmpkeywords)
		{
			unlink(u->tmpkeywords);
			free(u->tmpkeywords);
		}

		if (u->newkeywords)
		{
			unlink(u->newkeywords);
			free(u->newkeywords);
		}

		free(u->tmpfilename);
		free(u->curfilename);
		free(u);
	}
}

static void rename_callback(const char *old_path, const char *new_path)
{
struct imapscaninfo minfo;

	char *p=malloc(strlen(new_path)+sizeof("/" IMAPDB));

	if (!p)
		write_error_exit(0);

	strcat(strcpy(p, new_path), "/" IMAPDB);
	unlink(p);
	free(p);
	imapscan_init(&minfo);
	imapscan_maildir(&minfo, new_path, 0,0, NULL);
	imapscan_free(&minfo);
}

static int broken_uidvs()
{
	const char *p=getenv("IMAP_BROKENUIDV");

	return (p && atoi(p) != 0);
}

static void logoutmsg()
{
	bye_msg("INFO: LOGOUT");
}

void bye()
{
	if (current_temp_fd >= 0)
		close(current_temp_fd);
	if (current_temp_fn)
		unlink(current_temp_fn);

	if (current_mailbox)
		free(current_mailbox);
	imapscan_free(&current_maildir_info);
	maildirwatch_cleanup();
	fetch_free_cached();
	exit(0);
}

char *get_myrightson(const char *mailbox);

static void list_acl(const char *mailbox,
		     maildir_aclt_list *);
static int get_acllist(maildir_aclt_list *l,
		       const char *mailbox,
		       char **computed_mailbox_owner);

static void list_myrights(const char *mailbox,
			  const char *myrights);
static void list_postaddress(const char *mailbox);

static void accessdenied(const char *cmd, const char *folder,
			 const char *acl_required);

static int list_callback(const char *hiersep,
			 const char *mailbox,
			 int flags,
			 void *void_arg)
{
	const char *sep="";
	const char *cmd=(const char *)void_arg;
	maildir_aclt_list l;

	char *myrights=NULL;

	/*
	** If we're going to list ACLs, grab the ACLs now, because
	** get_acllist2() may end up calling myrights(), which generates
	** its own output.
	*/

	if (flags & (LIST_MYRIGHTS | LIST_ACL))
	{
		myrights=get_myrightson(mailbox);

		if (flags & LIST_MYRIGHTS)
		{
			if (!maildir_acl_canlistrights(myrights))
			{
				flags &= ~LIST_MYRIGHTS;
#if 0
				/* F.Y.I. only, should not be enabled
				   'cause make check may fail on systems
				   which return directory entries in a
				   different order */

				writes("* ACLFAILED \"");
				writemailbox(mailbox);
				writes("\"");
				accessdenied("LIST(MYRIGHTS)",
					     mailbox,
					     ACL_LOOKUP
					     ACL_READ
					     ACL_INSERT
					     ACL_CREATE
					     ACL_DELETEFOLDER
					     ACL_EXPUNGE
					     ACL_ADMINISTER);
#endif
			}
		}

		if (flags & LIST_ACL)
		{
			if (!strchr(myrights, ACL_ADMINISTER[0]))
			{
				flags &= ~LIST_ACL;
#if 0
				/* F.Y.I. only, should not be enabled
				   'cause make check may fail on systems
				   which return directory entries in a
				   different order */

				writes("* ACLFAILED \"");
				writemailbox(mailbox);
				writes("\"");
				accessdenied("LIST(ACL)",
					     mailbox,
					     ACL_ADMINISTER);
#endif
			}
		}
	}

	if (flags & LIST_ACL)
	{
		if (get_acllist(&l, mailbox, NULL) < 0)
		{
			fprintf(stderr,
				"ERR: Error reading ACLs for %s: %s\n",
				mailbox, strerror(errno));
			flags &= ~LIST_ACL;
		}
	}

	writes("* ");
	writes(cmd);
	writes(" (");

#define DO_FLAG(flag, flagname) \
	if (flags & (flag)) { writes(sep); writes(flagname); \
				sep=" "; }

	DO_FLAG(MAILBOX_NOSELECT, "\\Noselect");
	DO_FLAG(MAILBOX_UNMARKED, "\\Unmarked");
	DO_FLAG(MAILBOX_MARKED, "\\Marked");
	DO_FLAG(MAILBOX_NOCHILDREN, "\\HasNoChildren");
	DO_FLAG(MAILBOX_NOINFERIORS, "\\Noinferiors");
	DO_FLAG(MAILBOX_CHILDREN, "\\HasChildren");
#undef DO_FLAG

	writes(") ");
	writes(hiersep);
	writes(" \"");
	writemailbox(mailbox);
	writes("\"");

	if (flags & (LIST_ACL|LIST_MYRIGHTS|LIST_POSTADDRESS))
	{
		writes(" (");
		if (flags & LIST_ACL)
			list_acl(mailbox, &l);
		if (flags & LIST_MYRIGHTS)
			list_myrights(mailbox, myrights);
		if (flags & LIST_POSTADDRESS)
			list_postaddress(mailbox);
		writes(")");
	}

	writes("\r\n");
	if (myrights)
		free(myrights);
	if (flags & LIST_ACL)
		maildir_aclt_list_destroy(&l);

	return 0;
}

static void list_postaddress(const char *mailbox)
{
	writes("(POSTADDRESS NIL)");
}

/*
** Wrapper for maildir_aclt_read and maildir_aclt_write
*/

static int acl_read_folder(maildir_aclt_list *aclt_list,
			   const char *maildir,
			   const char *path)
{
	char *p, *q;
	int rc;

	/* Handle legacy shared. namespace */

	if (strcmp(path, SHARED) == 0)
	{
		maildir_aclt_list_init(aclt_list);
		if (maildir_aclt_list_add(aclt_list, "anyone",
					  ACL_LOOKUP, NULL) < 0)
		{
			maildir_aclt_list_destroy(aclt_list);
			return -1;
		}
		return 0;
	}

	if (strncmp(path, SHARED ".", sizeof(SHARED)) == 0)
	{
		maildir_aclt_list_init(aclt_list);

		if (strchr(path+sizeof(SHARED), '.') == 0)
		{
			if (maildir_aclt_list_add(aclt_list,
						  "anyone",
						  ACL_LOOKUP, NULL) < 0)
			{
				maildir_aclt_list_destroy(aclt_list);
				return -1;
			}
		}

		p=maildir_shareddir(maildir, path+sizeof(SHARED));
		if (!p)
		{
			if (maildir_aclt_list_add(aclt_list,
						  "anyone",
						  ACL_LOOKUP, NULL) < 0)
			{
				maildir_aclt_list_destroy(aclt_list);
				return -1;
			}
			return 0;
		}
		q=malloc(strlen(p)+sizeof("/new"));
		if (!q)
		{
			free(p);
			maildir_aclt_list_destroy(aclt_list);
			return -1;
		}
		strcat(strcpy(q, p), "/new");
		free(p);

		if (maildir_aclt_list_add(aclt_list,
					  "anyone",
					  access(q, W_OK) == 0 ?
					  ACL_LOOKUP ACL_READ
					  ACL_SEEN ACL_WRITE ACL_INSERT

					  ACL_DELETEFOLDER /* Wrong, but needed for ACL1 compatibility */

					  ACL_DELETEMSGS ACL_EXPUNGE:
					  ACL_LOOKUP ACL_READ ACL_SEEN
					  ACL_WRITE, NULL) < 0)
		{
			free(q);
			maildir_aclt_list_destroy(aclt_list);
			return -1;
		}
		free(q);
		return 0;
	}

	p=maildir_name2dir(".", path);

	if (!p)
		return -1;

	rc=maildir_acl_read(aclt_list, maildir, p[0] == '.' && p[1] == '/'
			    ? p+2:p);
	free(p);
	return rc;
}

static int acl_write_folder(maildir_aclt_list *aclt_list,
			    const char *maildir,
			    const char *path,

			    const char *owner,
			    const char **err_failedrights)
{
	char *p;
	int rc;

	if (strcmp(path, SHARED) == 0 ||
	    strncmp(path, SHARED ".", sizeof(SHARED ".")-1) == 0)
	{
		/* Legacy */

		return 1;
	}

	p=maildir_name2dir(".", path);

	if (!p)
		return -1;

	rc=maildir_acl_write(aclt_list, maildir, p[0] == '.' && p[1] == '/'
			     ? p+2:p,
			     owner, err_failedrights);
	free(p);
	return rc;
}

static void writeacl(const char *);

static void myrights()
{
	writes("* OK [MYRIGHTS \"");
	writeacl(current_mailbox_acl);
	writes("\"] ACL\r\n");
}

static int list_acl_cb(const char *ident,
		       const maildir_aclt *acl,
		       void *cb_arg);

char *compute_myrights(maildir_aclt_list *l,
		       const char *l_owner);

static int get_acllist(maildir_aclt_list *l,
		       const char *p,
		       char **computed_mailbox_owner)
{
	int rc;
	char *v;

	struct maildir_info mi;
	char *dummy_mailbox_owner=NULL;

	if (!computed_mailbox_owner)
		computed_mailbox_owner= &dummy_mailbox_owner;

	if (maildir_info_imap_find(&mi, p, getenv("AUTHENTICATED")) < 0)
		return -1;

	v=NULL;

	if (mi.homedir && mi.maildir)
	{
		rc=acl_read_folder(l, mi.homedir, mi.maildir);
		v=maildir_name2dir(mi.homedir, mi.maildir);
	}
	else if (mi.mailbox_type == MAILBOXTYPE_OLDSHARED)
	{
		rc=acl_read_folder(l, ".", p); /* It handles it */
	}
	else if (mi.mailbox_type == MAILBOXTYPE_NEWSHARED)
	{
		/* Intermediate #shared node.  Punt */

		maildir_aclt_list_init(l);
		rc=0;

		if (maildir_aclt_list_add(l, "anyone",
					  ACL_LOOKUP, NULL) < 0)
		{
			maildir_aclt_list_destroy(l);
			rc=-1;
		}
	}
	else
	{
		/* Unknown mailbox type, no ACLs */

		maildir_aclt_list_init(l);
		rc=0;
	}

	if (rc)
	{
		if (v)
			free(v);
		maildir_info_destroy(&mi);
		return -1;
	}

	*computed_mailbox_owner=my_strdup(mi.owner);

	maildir_info_destroy(&mi);

	/* Detect if ACLs on the currently-open folder have changed */

	if (rc == 0 && current_mailbox &&
#if SMAP
	    !smapflag &&
#endif

	    v && strcmp(v, current_mailbox) == 0)
	{
		char *new_acl=compute_myrights(l, *computed_mailbox_owner);

		if (new_acl && strcmp(new_acl, current_mailbox_acl))
		{
			free(current_mailbox_acl);
			current_mailbox_acl=new_acl;
			new_acl=NULL;
			myrights();
		}

		if (new_acl)
			free(new_acl);
	}
	if (v)
		free(v);
	if (dummy_mailbox_owner)
		free(dummy_mailbox_owner);
	return rc;
}

static void list_acl(const char *mailbox,
		     maildir_aclt_list *l)
{
	writes("(\"ACL\" (");
	maildir_aclt_list_enum(l, list_acl_cb, NULL);
	writes("))");
}

static void writeacl(const char *aclstr)
{
	char *p, *q, *r;
	const char *cp;
	int n=0;

	for (cp=aclstr; *cp; cp++)
		if (*cp == ACL_EXPUNGE[0])
			n |= 1;
		else if (*cp == ACL_DELETEMSGS[0])
			n |= 2;
		else if (*cp == ACL_DELETEFOLDER[0])
			n |= 4;

	if (n != 7)
	{
		writeqs(aclstr);
		return;
	}

	p=my_strdup(aclstr);

	*strchr(p, ACL_EXPUNGE[0])= ACL_DELETE_SPECIAL[0];

	q=r=p;
	while (*q)
	{
		if (*q != ACL_EXPUNGE[0] && *q != ACL_DELETEMSGS[0] &&
		    *q != ACL_DELETEFOLDER[0])
			*r++= *q;
		++q;
	}
	*r=0;
	writeqs(p);
	free(p);
}

static int list_acl_cb(const char *ident,
		       const maildir_aclt *acl,
		       void *cb_arg)
{
	writes("(\"");
	writeqs(ident);
	writes("\" \"");
	writeacl(maildir_aclt_ascstr(acl));
	writes("\")");
	return 0;
}

static void writeacl1(char *p)
{
	char *q, *r;

	if (strchr(p, ACL_EXPUNGE[0]) &&
	    strchr(p, ACL_DELETEMSGS[0]) &&
	    strchr(p, ACL_DELETEFOLDER[0]))
		*strchr(p, ACL_EXPUNGE[0])=ACL_DELETE_SPECIAL[0];
	/* See no evil */


	for (q=r=p; *q; q++)
	{
		if (*q == ACL_EXPUNGE[0] ||
		    *q == ACL_DELETEMSGS[0] ||
		    *q == ACL_DELETEFOLDER[0])
		{
			continue;
		}

		*r++= *q;
	}
	*r=0;
	writeqs(p);
}

static int getacl_cb(const char *ident,
		     const maildir_aclt *acl,
		     void *cb_arg)
{
	char *p;
	int isneg=0;

	if (*ident == '-')
	{
		isneg=1;
		++ident;
	}

	if (strchr(ident, '='))
	{
		if (strncmp(ident, "user=", 5))
			return 0; /* Hear no evil */
		ident += 5;
	}

	writes(" \"");
	if (isneg)
		writes("-");
	writeqs(ident);
	writes("\" \"");


	p=my_strdup(maildir_aclt_ascstr(acl));

	writeacl1(p);

	free(p);
	writes("\"");
	return 0;
}

char *get_myrightson(const char *mailbox)
{
	maildir_aclt_list l;
	char *mailbox_owner;
	char *rights;

	if (get_acllist(&l, mailbox, &mailbox_owner) < 0)
		return NULL;

	rights=compute_myrights(&l, mailbox_owner);
	free(mailbox_owner);
	maildir_aclt_list_destroy(&l);
	return rights;
}

char *get_myrightson_folder(const char *folder)
{
	char *p=imap_foldername_to_filename(enabled_utf8, folder);
	char *r;

	if (!p)
		return NULL;

	r=get_myrightson(p);
	free(p);
	return r;
}

char *compute_myrights(maildir_aclt_list *l, const char *l_owner)
{
	maildir_aclt aa;
	char *a;

	if (maildir_acl_computerights(&aa, l, getenv("AUTHENTICATED"),
				      l_owner) < 0)
		return NULL;

	a=my_strdup(maildir_aclt_ascstr(&aa));
	maildir_aclt_destroy(&aa);
	return a;
}

void check_rights(const char *mailbox, char *rights_buf)
{
	char *r=get_myrightson(mailbox);
	char *p, *q;

	if (!r)
	{
		fprintf(stderr, "ERR: Error reading ACLs for %s: %s\n",
			mailbox, strerror(errno));
		*rights_buf=0;
		return;
	}

	for (p=q=rights_buf; *p; p++)
	{
		if (strchr(r, *p) == NULL)
			continue;

		*q++ = *p;
	}
	*q=0;
	free(r);
}

static void list_myrights(const char *mailbox,
			  const char *r)
{

	writes("(\"MYRIGHTS\" ");

	if (r == NULL)
	{
		fprintf(stderr, "ERR: Error reading ACLs for %s: %s\n",
			mailbox, strerror(errno));
		writes(" NIL");
	}
	else
	{
		writes("\"");
		writeacl(r);
		writes("\"");
	}
	writes(")");
}


struct temp_acl_mailbox_list {
	struct temp_acl_mailbox_list *next;
	char *mailbox;
	char *hier;
	int flags;
};

/*
** Callback function from aclmailbox_scan that saves the listed mailboxes into
** a temporary link list.
*/

static int aclmailbox_scan(const char *hiersep,
			   const char *mailbox,
			   int flags,
			   void *void_arg)
{
	struct temp_acl_mailbox_list **p
		=(struct temp_acl_mailbox_list **)void_arg,
		*q=malloc(sizeof(struct temp_acl_mailbox_list));

	if (!q || !(q->mailbox=malloc(strlen(mailbox)+1)))
	{
		if (q) free(q);
		return -1;
	}
	if (!(q->hier=strdup(hiersep)))
	{
		free(q->mailbox);
		free(q);
		return -1;
	}
	strcpy(q->mailbox, mailbox);
	q->next= *p;
	q->flags=flags;
	*p=q;
	return 0;
}

static void free_temp_acl_mailbox(struct temp_acl_mailbox_list *p)
{
	free(p->mailbox);
	free(p->hier);
}

static int cmp_mb(const void *a, const void *b)
{
	return (strcmp ( ((struct temp_acl_mailbox_list *)a)->mailbox,
			 ((struct temp_acl_mailbox_list *)b)->mailbox));
}

/*
** Combine mailbox patterns; remove duplicate mailboxes.
**
** mailbox_merge takes the temp_acl_mailbox_list generated from a pattern,
** combines it with the current mailbox list (kept as an array), sorts the
** end result, then removes dupes.
*/

static int aclmailbox_merge(struct temp_acl_mailbox_list *l,
			    struct temp_acl_mailbox_list **mailbox_list)
{
	size_t n, o;
	struct temp_acl_mailbox_list *p;
	struct temp_acl_mailbox_list *newa;

	/* Count # of mailboxes so far */

	for (n=0; *mailbox_list && (*mailbox_list)[n].mailbox; n++)
		;

	/* Count # of new mailboxes */

	for (p=l, o=0; p; p=p->next)
		++o;

	if (n + o == 0)
		return 0; /* The list is empty */

	/* Expand the array */

	if ((newa= *mailbox_list == NULL ? malloc( (n+o+1)*sizeof(*l)):
	     realloc(*mailbox_list, (n+o+1)*sizeof(*l))) == NULL)
		return -1;

	*mailbox_list=newa;
	while (l)
	{
		newa[n]= *l;

		if ((newa[n].mailbox=strdup(l->mailbox)) == NULL)
			return -1;

		if ((newa[n].hier=strdup(l->hier)) == NULL)
		{
			free(newa[n].mailbox);
			newa[n].mailbox=NULL;
			return -1;
		}

		++n;
		memset(&newa[n], 0, sizeof(newa[n]));

		l=l->next;
	}
	qsort(newa, n, sizeof(*l), cmp_mb);

	/* Remove dupes */

	for (n=o=0; newa[n].mailbox; n++)
	{
		if (newa[n+1].mailbox &&
		    strcmp(newa[n].mailbox, newa[n+1].mailbox) == 0)
		{
			free_temp_acl_mailbox(&newa[n]);
			continue;
		}
		newa[o]=newa[n];
		++o;
	}
	memset(&newa[o], 0, sizeof(newa[o]));

	return 0;
}

static int aclstore(const char *, struct temp_acl_mailbox_list *);
static int aclset(const char *, struct temp_acl_mailbox_list *);
static int acldelete(const char *, struct temp_acl_mailbox_list *);

static void free_mailboxlist(struct temp_acl_mailbox_list *mailboxlist)
{
	size_t i;
	for (i=0; mailboxlist && mailboxlist[i].mailbox; i++)
	{
		free(mailboxlist[i].hier);
		free(mailboxlist[i].mailbox);
	}
	if (mailboxlist)
		free(mailboxlist);
}

static void free_tempmailboxlist(struct temp_acl_mailbox_list *mailboxlist)
{
	struct temp_acl_mailbox_list *p;

	while ((p=mailboxlist) != NULL)
	{
		mailboxlist=p->next;
		free_temp_acl_mailbox(p);
		free(p);
	}
}

static int aclcmd(const char *tag)
{
	char aclcmd[11];
	struct temp_acl_mailbox_list *mailboxlist;
	struct temp_acl_mailbox_list *mblist;
	struct	imaptoken *curtoken;
	size_t i;
	int rc;
	int (*aclfunc)(const char *, struct temp_acl_mailbox_list *);

	/* Expect ACL followed only by: STORE/DELETE/SET */

	if ((curtoken=nexttoken())->tokentype != IT_ATOM ||
	    strlen(curtoken->tokenbuf) > sizeof(aclcmd)-1)
	{
		errno=EINVAL;
		return -1;
	}

	strcpy(aclcmd, curtoken->tokenbuf);

	mailboxlist=NULL;

	switch ((curtoken=nexttoken_nouc())->tokentype) {
	case IT_LPAREN:
		while ((curtoken=nexttoken_nouc())->tokentype != IT_RPAREN)
		{
			char *p;
			if (curtoken->tokentype != IT_QUOTED_STRING &&
				curtoken->tokentype != IT_ATOM &&
				curtoken->tokentype != IT_NUMBER)
			{
				writes(tag);
				writes(" BAD Invalid command\r\n");
				return 0;
			}

			p=imap_foldername_to_filename(enabled_utf8,
						      curtoken->tokenbuf);
			if (!p)
			{
				errno=EINVAL;
				return -1;
			}
			mblist=NULL;

			if (mailbox_scan("", p, 0,
					 aclmailbox_scan, &mblist) ||
			    aclmailbox_merge(mblist, &mailboxlist))
			{
				free(p);
				free_tempmailboxlist(mblist);
				free_mailboxlist(mailboxlist);
				return -1;

			}
			free(p);
			free_tempmailboxlist(mblist);
		}
		break;
	case IT_QUOTED_STRING:
	case IT_ATOM:
	case IT_NUMBER:

		mblist=NULL;
		{
			char *p=imap_foldername_to_filename(enabled_utf8,
							    curtoken->tokenbuf);

			if (mailbox_scan("", p, LIST_CHECK1FOLDER,
					 aclmailbox_scan, &mblist) ||
			    aclmailbox_merge(mblist, &mailboxlist))
			{
				free(p);
				free_tempmailboxlist(mblist);
				free_mailboxlist(mailboxlist);
				return -1;
			}
			free(p);
			free_tempmailboxlist(mblist);
		}
		break;
	case IT_ERROR:
		writes(tag);
		writes(" BAD Invalid command\r\n");
		return 0;
	}

	rc=0;

	aclfunc=strcmp(aclcmd, "STORE") == 0 ? aclstore:
		strcmp(aclcmd, "DELETE") == 0 ? acldelete:
		strcmp(aclcmd, "SET") == 0 ? aclset:NULL;

	rc= aclfunc ? (*aclfunc)(tag, mailboxlist): -1;

	if (rc == 0)
	{
		for (i=0; mailboxlist && mailboxlist[i].mailbox; i++)
		{
			if (mailboxlist[i].mailbox[0])
				list_callback(mailboxlist[i].hier,
					      mailboxlist[i].mailbox,
					      LIST_ACL | mailboxlist[i].flags,
					      "LIST");
		}
	}
	free_mailboxlist(mailboxlist);

	if (rc == 0)
	{
		writes(tag);
		writes(" OK ACL ");
		writes(aclcmd);
		writes(" completed.\r\n");
	}
	else
	{
		errno=EINVAL;
	}
	return rc;
}

static int fix_acl_delete(int (*func)(maildir_aclt *, const char *,
				      const maildir_aclt *),
			  maildir_aclt *newacl,
			  const char *aclstr)
{
	char *p, *q;
	int rc;

	if (strchr(aclstr, ACL_DELETE_SPECIAL[0]) == NULL)
		return (*func)(newacl, aclstr, NULL);


	p=malloc(strlen(aclstr)+sizeof(ACL_DELETEFOLDER
				       ACL_DELETEMSGS
				       ACL_EXPUNGE));
	if (!p)
		return -1;

	q=p;
	while (*aclstr)
	{
		if (*aclstr != ACL_DELETE_SPECIAL[0])
			*q++ = *aclstr;
		++aclstr;
	}

	strcpy(q, ACL_DELETEFOLDER ACL_DELETEMSGS ACL_EXPUNGE);

	rc=(*func)(newacl, p, NULL);
	free(p);
	return rc;
}

static int fix_acl_delete2(maildir_aclt_list *aclt_list,
			   const char *identifier,
			   const char *aclstr)
{
	char *p, *q;
	int rc;

	if (strchr(aclstr, ACL_DELETE_SPECIAL[0]) == NULL)
		return maildir_aclt_list_add(aclt_list, identifier, aclstr,
					     NULL);


	p=malloc(strlen(aclstr)+sizeof(ACL_DELETEFOLDER
				       ACL_DELETEMSGS
				       ACL_EXPUNGE));
	if (!p)
		return -1;

	q=p;
	while (*aclstr)
	{
		if (*aclstr != ACL_DELETE_SPECIAL[0])
			*q++ = *aclstr;
		++aclstr;
	}

	strcpy(q, ACL_DELETEFOLDER ACL_DELETEMSGS ACL_EXPUNGE);

	rc=maildir_aclt_list_add(aclt_list, identifier, p, NULL);
	free(p);
	return rc;
}

void aclminimum(const char *identifier)
{
	if (strcmp(identifier, "administrators") == 0 ||
	    strcmp(identifier, "group=administrators") == 0)
	{
		writes(ACL_ALL);
		return;
	}

	if (strcmp(identifier, "-administrators") == 0 ||
	    strcmp(identifier, "-group=administrators") == 0)
	{
		writes("\"\"");
		return;
	}

	writes(*identifier == '-' ? "\"\"":ACL_ADMINISTER ACL_LOOKUP);
	writes(" " ACL_CREATE
	       " " ACL_EXPUNGE
	       " " ACL_INSERT
	       " " ACL_POST
	       " " ACL_READ
	       " " ACL_SEEN
	       " " ACL_DELETEMSGS
	       " " ACL_WRITE
	       " " ACL_DELETEFOLDER);
}

static void aclfailed(const char *mailbox, const char *identifier)
{
	if (!identifier)
	{
		writes("* ACLFAILED \"");
		writemailbox(mailbox);
		writes("\" ");
		writes(strerror(errno));
		writes("\r\n");
		return;
	}

	writes("* RIGHTS-INFO \"");
	writemailbox(mailbox);
	writes("\" \"");
	writeqs(identifier);
	writes("\" ");

	aclminimum(identifier);
	writes("\r\n");
}

static int acl_settable_folder(char *mailbox,
			       struct maildir_info *mi)
{
	if (maildir_info_imap_find(mi, mailbox, getenv("AUTHENTICATED")) < 0)
	{
		aclfailed(mailbox, NULL);
		*mailbox=0;
		return (-1);
	}

	if (mi->homedir == NULL || mi->maildir == NULL)
	{
		writes("* ACLFAILED \"");
		writemailbox(mailbox);
		writes("\" ACLs may not be modified for special mailbox\r\n");
		maildir_info_destroy(mi);
		*mailbox=0;
		return -1;
	}
	return 0;
}

int acl_lock(const char *homedir,
	     int (*func)(void *),
	     void *void_arg)
{
	struct imapscaninfo ii;
	int rc;

	imapscan_init(&ii);
	rc=imapmaildirlock(&ii, homedir, func, void_arg);
	imapscan_free(&ii);
	return rc;
}

static int do_acl_mod_0(void *);

struct do_acl_info {
	maildir_aclt_list *aclt_list;
	struct maildir_info *mi;
	const char *identifier;
	const char *newrights;
	const char **acl_error;
};


static int do_acl_mod(maildir_aclt_list *aclt_list,
		      struct maildir_info *mi,
		      const char *identifier,
		      const char *newrights,
		      const char **acl_error)
{
	struct do_acl_info dai;

	*acl_error=NULL;

	dai.aclt_list=aclt_list;
	dai.mi=mi;
	dai.identifier=identifier;
	dai.newrights=newrights;
	dai.acl_error=acl_error;
	return acl_lock(mi->homedir, do_acl_mod_0, &dai);
}

static int do_acl_mod_0(void *void_arg)
{
	struct do_acl_info *dai=
		(struct do_acl_info *)void_arg;
	maildir_aclt_list *aclt_list=dai->aclt_list;
	struct maildir_info *mi=dai->mi;
	const char *identifier=dai->identifier;
	const char *newrights=dai->newrights;
	const char **acl_error=dai->acl_error;

	if (newrights[0] == '+')
	{
		maildir_aclt newacl;
		const maildir_aclt *oldacl;

		if (fix_acl_delete(&maildir_aclt_init,
				   &newacl, newrights+1) < 0
		    || ((oldacl=maildir_aclt_list_find(aclt_list, identifier))
			!= NULL && maildir_aclt_add(&newacl, NULL, oldacl) < 0)
		    || maildir_aclt_list_add(aclt_list, identifier, NULL,
					     &newacl) < 0 ||
		    acl_write_folder(aclt_list, mi->homedir,
					     mi->maildir,
					     mi->owner, acl_error) < 0)

			{
				maildir_aclt_destroy(&newacl);
				return -1;
			}
		maildir_aclt_destroy(&newacl);
	}
	else if (newrights[0] == '-')
	{
		maildir_aclt newacl;
		const maildir_aclt *oldacl;

		oldacl=maildir_aclt_list_find(aclt_list, identifier);

		if (maildir_aclt_init(&newacl, oldacl == NULL ? "":NULL,
				      oldacl) < 0
		    || fix_acl_delete(&maildir_aclt_del,
				      &newacl, newrights+1) < 0
		    || (strlen(maildir_aclt_ascstr(&newacl)) == 0 ?
			maildir_aclt_list_del(aclt_list, identifier):
			maildir_aclt_list_add(aclt_list, identifier,
					      NULL, &newacl)) < 0 ||
		    acl_write_folder(aclt_list, mi->homedir,
					     mi->maildir,
					     mi->owner,
					     acl_error) < 0)
			{
				maildir_aclt_destroy(&newacl);
				return -1;
			}
			maildir_aclt_destroy(&newacl);
	}
	else
	{
		acl_error=NULL;

		if ((newrights[0] == 0 ?
		     maildir_aclt_list_del(aclt_list, identifier):
		     fix_acl_delete2(aclt_list, identifier, newrights)) < 0
		    || acl_write_folder(aclt_list, mi->homedir,
					mi->maildir, mi->owner,
					acl_error) < 0)
		{
			return -1;
		}
	}

	return 0;
}

static int aclstore(const char *tag, struct temp_acl_mailbox_list *mailboxes)
{
	struct imaptoken *curtoken;
	char *identifier;
	size_t i;

	if ((curtoken=nexttoken_nouc())->tokentype != IT_QUOTED_STRING &&
	    curtoken->tokentype != IT_ATOM &&
	    curtoken->tokentype != IT_NUMBER)
		return -1;

	if ((identifier=strdup(curtoken->tokenbuf)) == NULL)
		write_error_exit(0);

	if ((curtoken=nexttoken_nouc())->tokentype != IT_QUOTED_STRING &&
	    curtoken->tokentype != IT_ATOM &&
	    curtoken->tokentype != IT_NUMBER)
	{
		free(identifier);
		return -1;
	}

	for (i=0; mailboxes && mailboxes[i].mailbox; i++)
	{
		maildir_aclt_list aclt_list;
		const char *acl_error;
		struct maildir_info mi;

		if (acl_settable_folder(mailboxes[i].mailbox, &mi))
			continue;

		{
			CHECK_RIGHTSM(mailboxes[i].mailbox,
				      acl_rights,
				      ACL_ADMINISTER);
			if (acl_rights[0] == 0)
			{
				writes("* ACLFAILED \"");
				writemailbox(mailboxes[i].mailbox);
				writes("\"");
				accessdenied("ACL STORE",
					     mailboxes[i].mailbox,
					     ACL_ADMINISTER);
				maildir_info_destroy(&mi);
				mailboxes[i].mailbox[0]=0;
				continue;
			}
		}

		if (acl_read_folder(&aclt_list, mi.homedir, mi.maildir))
		{
			aclfailed(mailboxes[i].mailbox, NULL);
			maildir_info_destroy(&mi);
			continue;
		}

		if (do_acl_mod(&aclt_list, &mi, identifier, curtoken->tokenbuf,
			       &acl_error) < 0)
		{
			aclfailed(mailboxes[i].mailbox, acl_error);

			maildir_aclt_list_destroy(&aclt_list);
			maildir_info_destroy(&mi);
			continue;
		}
		maildir_aclt_list_destroy(&aclt_list);
		maildir_info_destroy(&mi);
	}

	free(identifier);
	return 0;
}

struct aclset_info {
	struct maildir_info *mi;
	maildir_aclt_list *newlist;
	const char **acl_error;
};

static int do_aclset(void *);

static int aclset(const char *tag, struct temp_acl_mailbox_list *mailboxes)
{
	struct imaptoken *curtoken;
	char *identifier;
	maildir_aclt_list newlist;
	size_t i;

	maildir_aclt_list_init(&newlist);

	while ((curtoken=nexttoken_nouc())->tokentype != IT_EOL)
	{
		if (curtoken->tokentype != IT_QUOTED_STRING &&
		    curtoken->tokentype != IT_ATOM &&
		    curtoken->tokentype != IT_NUMBER)
			return -1;
		if ((identifier=strdup(curtoken->tokenbuf)) == NULL)
			write_error_exit(0);

		if ((curtoken=nexttoken_nouc())->tokentype
		    != IT_QUOTED_STRING &&
		    curtoken->tokentype != IT_ATOM &&
		    curtoken->tokentype != IT_NUMBER)
		{
			free(identifier);
			maildir_aclt_list_destroy(&newlist);
			return -1;
		}

		if (fix_acl_delete2(&newlist, identifier,
				    curtoken->tokenbuf) < 0)
		{
			maildir_aclt_list_destroy(&newlist);
			writes(tag);
			writes(" NO ACL SET <");
			writes(identifier);
			writes(", ");
			writes(curtoken->tokenbuf);
			writes("> failed.\r\n");
			free(identifier);
			return 0;
		}
		free(identifier);
	}

	for (i=0; mailboxes && mailboxes[i].mailbox; i++)
	{
		const char *acl_error;
		struct maildir_info mi;
		struct aclset_info ai;

		if (acl_settable_folder(mailboxes[i].mailbox, &mi))
			continue;

		{
			CHECK_RIGHTSM(mailboxes[i].mailbox,
				      acl_rights,
				      ACL_ADMINISTER);
			if (acl_rights[0] == 0)
			{
				maildir_info_destroy(&mi);
				writes("* ACLFAILED \"");
				writemailbox(mailboxes[i].mailbox);
				writes("\"");
				accessdenied("ACL SET", mailboxes[i].mailbox,
					     ACL_ADMINISTER);
				mailboxes[i].mailbox[0]=0;
				continue;
			}
		}

		acl_error=NULL;
		ai.mi=&mi;
		ai.acl_error= &acl_error;
		ai.newlist= &newlist;

		if (acl_lock(mi.homedir, do_aclset, &ai))
		{
			aclfailed(mailboxes[i].mailbox, acl_error);
			maildir_info_destroy(&mi);
			mailboxes[i].mailbox[0]=0;
			continue;
		}
		maildir_info_destroy(&mi);
	}
	maildir_aclt_list_destroy(&newlist);
	return 0;
}

static int do_aclset(void *void_arg)
{
	struct aclset_info *ai=(struct aclset_info *)void_arg;

	return acl_write_folder(ai->newlist, ai->mi->homedir,
				ai->mi->maildir,
				ai->mi->owner, ai->acl_error);
}

struct acldelete_info {
	const char *mailbox;
	const char *identifier;
	struct maildir_info *mi;
};

static int do_acldelete(void *);

static int acldelete(const char *tag, struct temp_acl_mailbox_list *mailboxes)
{
	struct imaptoken *curtoken;
	const char *identifier;
	size_t i;

	if ((curtoken=nexttoken_nouc())->tokentype != IT_QUOTED_STRING &&
	    curtoken->tokentype != IT_ATOM &&
	    curtoken->tokentype != IT_NUMBER)
		return -1;

	identifier=curtoken->tokenbuf;

	for (i=0; mailboxes && mailboxes[i].mailbox; i++)
	{
		struct maildir_info mi;
		struct acldelete_info ai;

		if (acl_settable_folder(mailboxes[i].mailbox, &mi))
			continue;

		{
			CHECK_RIGHTSM(mailboxes[i].mailbox,
				      acl_rights,
				      ACL_ADMINISTER);
			if (acl_rights[0] == 0)
			{
				writes("* ACLFAILED \"");
				writemailbox(mailboxes[i].mailbox);
				writes("\"");
				accessdenied("ACL DELETE",
					     mailboxes[i].mailbox,
					     ACL_ADMINISTER);
				maildir_info_destroy(&mi);
				mailboxes[i].mailbox[0]=0;
				continue;
			}
		}

		ai.mailbox=mailboxes[i].mailbox;
		ai.mi= &mi;
		ai.identifier=identifier;
		if (acl_lock(mi.homedir, do_acldelete, &ai))
			mailboxes[i].mailbox[0]=0;
		maildir_info_destroy(&mi);
	}
	return 0;
}

static int do_acldelete(void *void_arg)
{
	struct acldelete_info *ai=
		(struct acldelete_info *)void_arg;
	const char *mailbox=ai->mailbox;
	struct maildir_info *mi=ai->mi;

	maildir_aclt_list aclt_list;
	const char *acl_error;

	if (acl_read_folder(&aclt_list, mi->homedir, mi->maildir) < 0)
	{
		writes("* NO Error reading ACLs for ");
		writes(mailbox);
		writes(": ");
		writes(strerror(errno));
		writes("\r\n");
		return -1;
	}

	acl_error=NULL;

	if (maildir_aclt_list_del(&aclt_list, ai->identifier) < 0 ||
	    acl_write_folder(&aclt_list, mi->homedir, mi->maildir,
			     mi->owner, &acl_error) < 0)
	{
		aclfailed(mailbox, acl_error);
		maildir_aclt_list_destroy(&aclt_list);
		return -1;
	}
	maildir_aclt_list_destroy(&aclt_list);
	return 0;
}


static void accessdenied(const char *cmd, const char *folder,
			 const char *acl_required)
{
	writes(" NO Access denied for ");
	writes(cmd);
	writes(" on ");
	writes(folder);
	writes(" (ACL \"");
	writes(acl_required);
	writes("\" required)\r\n");
}

/* Even if the folder does not exist, if there are subfolders it exists
** virtually.
*/

static int folder_exists_cb(const char *hiersep,
			    const char *mailbox,
			    int flags,
			    void *void_arg)
{
	*(int *)void_arg=1;
	return 0;
}

static int folder_exists(const char *folder)
{
	int flag=0;

	if (mailbox_scan("", folder, LIST_CHECK1FOLDER,
			 folder_exists_cb, &flag))
		return 0;
	return flag;
}

int do_folder_delete(char *mailbox_name)
{
	maildir_aclt_list l;
	const char *acl_error;
	struct maildir_info mi;

	if (maildir_info_imap_find(&mi, mailbox_name, getenv("AUTHENTICATED"))
	    < 0)
		return -1;

	if (mi.homedir == NULL || mi.maildir == NULL)
	{
		maildir_info_destroy(&mi);
		return -1;
	}

	if (acl_read_folder(&l, mi.homedir, mi.maildir) < 0)
		return -1;

	if (strcasecmp(mi.maildir, INBOX))
	{
		char *p=maildir_name2dir(mi.homedir, mi.maildir);

		if (p && is_reserved(p) == 0 && mddelete(p) == 0)
		{
			if (folder_exists(mailbox_name))
			{
				acl_write_folder(&l, mi.homedir,
						 mi.maildir, NULL,
						 &acl_error);
			}
			maildir_aclt_list_destroy(&l);
			maildir_quota_recalculate(mi.homedir);
			free(p);
			maildir_info_destroy(&mi);
			return 0;
		}

		if (p)
			free(p);
	}
	maildir_aclt_list_destroy(&l);
	return -1;
}

int acl_flags_adjust(const char *access_rights,
		     struct imapflags *flags)
{
	if (strchr(access_rights, ACL_DELETEMSGS[0]) == NULL)
		flags->deleted=0;
	if (strchr(access_rights, ACL_SEEN[0]) == NULL)
		flags->seen=0;
	if (strchr(access_rights, ACL_WRITE[0]) == NULL)
	{
		flags->answered=flags->flagged=flags->drafts=0;
		return 1;
	}
	return 0;
}

static int append(const char *tag, const char *mailbox, const char *path)
{

	struct	imapflags flags;
	struct libmail_kwMessage *keywords;
	time_t	timestamp=0;
	unsigned long new_uidv, new_uid;
	char access_rights[8];
	struct imaptoken *curtoken;
	int need_rparen;
 	int utf8_error=0;

	if (access(path, 0))
	{
		writes(tag);
		writes(" NO [TRYCREATE] Must create mailbox before append\r\n");
		return (0);
	}

	{
		CHECK_RIGHTSM(mailbox,
			      append_rights,
			      ACL_INSERT ACL_DELETEMSGS
			      ACL_SEEN ACL_WRITE);

		if (strchr(append_rights, ACL_INSERT[0]) == NULL)
		{
			writes(tag);
			accessdenied("APPEND",
				     mailbox,
				     ACL_INSERT);
			return 0;
		}

		strcpy(access_rights, append_rights);
	}

	if (current_mailbox &&
	    strcmp(path, current_mailbox) == 0 && current_mailbox_ro)
	{
		writes(tag);
		writes(" NO Current box is selected READ-ONLY.\r\n");
		return (0);
	}

	curtoken=nexttoken_noparseliteral();
	memset(&flags, 0, sizeof(flags));
	if ((keywords=libmail_kwmCreate()) == NULL)
		write_error_exit(0);

	if (curtoken->tokentype == IT_LPAREN)
	{
		if (get_flagsAndKeywords(&flags, &keywords))
		{
			libmail_kwmDestroy(keywords);
			return (-1);
		}
		curtoken=nexttoken_noparseliteral();
	}
	else if (curtoken->tokentype == IT_ATOM)
	{
		if (get_flagname(curtoken->tokenbuf, &flags))
		{
			if (!valid_keyword(curtoken->tokenbuf))
			{
				libmail_kwmDestroy(keywords);
				return -1;
			}

			libmail_kwmSetName(current_maildir_info.keywordList,
					   keywords,
					   curtoken->tokenbuf);
		}
		curtoken=nexttoken_noparseliteral();
	}
	else if (curtoken->tokentype == IT_NIL)
		curtoken=nexttoken_noparseliteral();

	if (curtoken->tokentype == IT_QUOTED_STRING)
	{
		if (decode_date_time(curtoken->tokenbuf, &timestamp))
		{
			libmail_kwmDestroy(keywords);
			return (-1);
		}
		curtoken=nexttoken_noparseliteral();
	}
	else if (curtoken->tokentype == IT_NIL)
		curtoken=nexttoken_noparseliteral();

	need_rparen=0;

	if (enabled_utf8 && curtoken->tokentype == IT_ATOM &&
	    strcmp(curtoken->tokenbuf, "UTF8") == 0)
	{
		/* See also: https://bugs.python.org/issue34138 */

		curtoken=nexttoken();
		if (curtoken->tokentype != IT_LPAREN)
		{
			libmail_kwmDestroy(keywords);
			return (-1);
		}

		curtoken=nexttoken_noparseliteral();
		if (curtoken->tokentype != IT_LITERAL8_STRING_START)
		{
			libmail_kwmDestroy(keywords);

			/* Don't break the protocol level */
			convert_literal_tokens(curtoken);
			return (-1);
		}
		need_rparen=1;
	}
	else if (curtoken->tokentype != IT_LITERAL_STRING_START)
	{
		libmail_kwmDestroy(keywords);

		/* Don't break the protocol level */
		convert_literal_tokens(curtoken);
		return (-1);
	}

	acl_flags_adjust(access_rights, &flags);

	if (store_mailbox(tag, path, &flags,
			  acl_flags_adjust(access_rights, &flags)
			  ? NULL:keywords,
			  timestamp,
 			  curtoken, &new_uidv, &new_uid, &utf8_error))
	{
		libmail_kwmDestroy(keywords);
		unread('\n');
		return (0);
	}
	libmail_kwmDestroy(keywords);

	if (need_rparen)
	{
		if (nexttoken()->tokentype != IT_RPAREN)
		{
			return (-1);
		}
	}

	if (nexttoken()->tokentype != IT_EOL)
	{
		return (-1);
	}

	dirsync(path);
 	if (utf8_error) {
 		writes(" [ALERT] Your IMAP client does not appear to "
 				"correctly implement Unicode messages, "
 				"see https://tools.ietf.org/html/rfc6855.html");
 	}
	writes(tag);
	writes(" OK [APPENDUID ");
	writen(new_uidv);
	writes(" ");
	writen(new_uid);
 	writes("] APPEND Ok.");
 	writes("\r\n");
	return (0);
}


/* Check for 'c' rights on the parent directory. */

static int check_parent_create(const char *tag,
			       const char *cmd, char *folder)
{
	char *parentPtr;

	parentPtr=strrchr(folder, HIERCH);

	if (parentPtr)
	{
		*parentPtr=0;

		{
			CHECK_RIGHTSM(folder,
				      create_rights,
				      ACL_CREATE);

			if (create_rights[0])
			{
				if (parentPtr)
					*parentPtr=HIERCH;
				return 0;
			}
		}
	}

	writes(tag);
	accessdenied(cmd, folder, ACL_CREATE);
	if (parentPtr)
		*parentPtr=HIERCH;
	return -1;
}

/* Convert ACL1 identifiers to ACL2 */

static char *acl2_identifier(const char *tag,
			     const char *identifier)
{
	const char *ident_orig=identifier;

	char *p;
	int isneg=0;

	if (*identifier == '-')
	{
		isneg=1;
		++identifier;
	}

	if (strcmp(identifier, "anyone") == 0 ||
	    strcmp(identifier, "anonymous") == 0 ||
	    strcmp(identifier, "authuser") == 0 ||
	    strcmp(identifier, "owner") == 0 ||
	    strcmp(identifier, "administrators") == 0)
		return my_strdup(ident_orig);

	if (strchr(identifier, '='))
	{
		writes(tag);
		writes(" NO Invalid ACL identifier.\r\n");
		return NULL;
	}

	p=malloc(sizeof("-user=")+strlen(identifier));

	if (!p)
		write_error_exit(0);
	return strcat(strcat(strcpy(p, isneg ? "-":""), "user="), identifier);
}


int folder_rename(struct maildir_info *mi1,
		  struct maildir_info *mi2,
		  const char **errmsg)
{
	char *old_mailbox, *new_mailbox;

	if (mi1->homedir == NULL || mi1->maildir == NULL)
	{
		*errmsg="Invalid mailbox name";
		return -1;
	}

	if (mi2->homedir == NULL || mi2->maildir == NULL)
	{
		*errmsg="Invalid new mailbox name";
		return -1;
	}

	if (current_mailbox)
	{
		char *mailbox=maildir_name2dir(mi1->homedir,
					       mi1->maildir);
		size_t l;

		if (!mailbox)
		{
			*errmsg="Invalid mailbox name";
			return -1;
		}

		l=strlen(mailbox);

		if (strncmp(mailbox, current_mailbox, l) == 0 &&
		    (current_mailbox[l] == 0 ||
		     current_mailbox[l] == HIERCH))
		{
			free(mailbox);
			*errmsg="Can't RENAME the currently-open folder";
			return -1;
		}
		free(mailbox);
	}

	if (strcmp(mi1->homedir, mi2->homedir))
	{
		*errmsg="Cannot move a folder to a different account.";
		return -1;
	}

	if (strcmp(mi1->maildir, INBOX) == 0 ||
	    strcmp(mi2->maildir, INBOX) == 0)
	{
		*errmsg="INBOX rename not implemented.";
		return -1;
	}

	if (is_reserved_name(mi1->maildir) ||
	    is_reserved_name(mi2->maildir))
	{
		*errmsg="Reserved folder name - cannot rename.";
		return -1;
	}

	/* Depend on maildir_name2dir returning ./.folder, see
	** maildir_rename() call. */

	if ((old_mailbox=maildir_name2dir(".", mi1->maildir)) == NULL ||
	    strncmp(old_mailbox, "./", 2))
	{
		if (old_mailbox)
			free(old_mailbox);
		*errmsg="Internal error in RENAME: maildir_name2dir failed"
			" for the old folder rename.";
		return -1;
	}

	if ((new_mailbox=maildir_name2dir(".", mi2->maildir)) == NULL ||
	    strncmp(new_mailbox, "./", 2))
	{
		free(old_mailbox);
		if (new_mailbox)
			free(new_mailbox);
		*errmsg="Internal error in RENAME: maildir_name2dir failed"
			" for the new folder rename.";
		return -1;
	}

	fetch_free_cache();

	if (maildir_rename(mi1->homedir,
			   old_mailbox+2, new_mailbox+2,
			   MAILDIR_RENAME_FOLDER |
			   MAILDIR_RENAME_SUBFOLDERS,
			   &rename_callback))
	{
		free(old_mailbox);
		free(new_mailbox);

		*errmsg="@RENAME failed: ";
		return -1;
	}

	maildir_quota_recalculate(mi1->homedir);
	free(old_mailbox);
	free(new_mailbox);
	return 0;
}

static int validate_charset(const char *tag, char **charset)
{
	unicode_convert_handle_t conv;
	char32_t *ucptr;
	size_t ucsize;

	if (*charset == NULL)
		*charset=my_strdup("UTF-8");

	if (enabled_utf8 && strcmp(*charset, "UTF-8"))
	{
		writes(tag);
		writes(" BAD [BADCHARSET] Only UTF-8 charset is valid after enabling RFC 6855 support\r\n");
		return (-1);
	}

	conv=unicode_convert_tou_init(*charset, &ucptr, &ucsize, 1);

	if (!conv)
	{
		writes(tag);
		writes(" NO [BADCHARSET] The requested character set is not supported.\r\n");
		return (-1);
	}
	if (unicode_convert_deinit(conv, NULL) == 0)
		free(ucptr);
	return (0);
}

int do_imap_command(const char *tag, int *flushflag)
{
struct	imaptoken *curtoken=nexttoken();
int	uid=0;

	if (curtoken->tokentype != IT_ATOM)	return (-1);

	/* Commands that work in authenticated state */

	if (strcmp(curtoken->tokenbuf, "CAPABILITY") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);
		writes("* CAPABILITY ");
		imapcapability();
		writes("\r\n");
		writes(tag);
		writes(" OK CAPABILITY completed\r\n");
		return (0);
	}
	if (strcmp(curtoken->tokenbuf, "NOOP") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);
		if (current_mailbox)
			doNoop(1);
		writes(tag);
		writes(" OK NOOP completed\r\n");
		return (0);
	}
	if (strcmp(curtoken->tokenbuf, "ID") == 0)
	{
		if (doId() < 0)
			return (-1);
		writes(tag);
		writes(" OK ID completed\r\n");
		return (0);
	}
    if (strcmp(curtoken->tokenbuf, "IDLE") == 0)
    {
	     const char *p;

        if (nexttoken()->tokentype != IT_EOL)
			return (-1);

      read_eol();

      if ((p=getenv("IMAP_ENHANCEDIDLE")) == NULL
		   || !atoi(p)
		   || imapenhancedidle())
		       imapidle();
      curtoken=nexttoken();
      if (strcmp(curtoken->tokenbuf, "DONE") == 0)
      {
	       if (current_mailbox)
		       doNoop(0);
	       writes(tag);
	       writes(" OK IDLE completed\r\n");
	       return (0);
       }
       return (-1);
    }
	if (strcmp(curtoken->tokenbuf, "LOGOUT") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);
		fetch_free_cache();
		writes("* BYE IMAP4rev1 server shutting down\r\n");
		writes(tag);
		writes(" OK LOGOUT completed\r\n");
		writeflush();
		emptytrash();
		logoutmsg();
		bye();
	}

	if (strcmp(curtoken->tokenbuf, "LIST") == 0
		|| strcmp(curtoken->tokenbuf, "LSUB") == 0)
	{
		char	*reference, *name;
		int	rc;
		char	cmdbuf[5];
		int	list_flags=0;

		strcpy(cmdbuf, curtoken->tokenbuf);

		curtoken=nexttoken_nouc();
		if (curtoken->tokentype == IT_LPAREN)
		{
			while ((curtoken=nexttoken())->tokentype != IT_RPAREN)
			{
				if (curtoken->tokentype != IT_QUOTED_STRING &&
				    curtoken->tokentype != IT_ATOM &&
				    curtoken->tokentype != IT_NUMBER)
					return (-1);

				if (strcmp(curtoken->tokenbuf, "ACL") == 0)
					list_flags |= LIST_ACL;
				if (strcmp(curtoken->tokenbuf, "MYRIGHTS")==0)
					list_flags |= LIST_MYRIGHTS;
				if (strcmp(curtoken->tokenbuf,
					   "POSTADDRESS")==0)
					list_flags |= LIST_POSTADDRESS;
			}

			curtoken=nexttoken_nouc();
		}


		if (curtoken->tokentype == IT_NIL)
			reference=my_strdup("");
		else
		{
			if (curtoken->tokentype != IT_QUOTED_STRING &&
				curtoken->tokentype != IT_ATOM &&
				curtoken->tokentype != IT_NUMBER)
			{
				writes(tag);
				writes(" BAD Invalid command\r\n");
				return (0);
			}
			reference=imap_foldername_to_filename
				(enabled_utf8, curtoken->tokenbuf);
			if (!reference)
				return -1;
		}
		curtoken=nexttoken_nouc();

		if (curtoken->tokentype == IT_NIL)
			name=my_strdup("");
		else
		{
			if (curtoken->tokentype != IT_QUOTED_STRING &&
				curtoken->tokentype != IT_ATOM &&
				curtoken->tokentype != IT_NUMBER)
			{
				free(reference);
				writes(tag);
				writes(" BAD Invalid command\r\n");
				return(0);
			}
			name=imap_foldername_to_filename(enabled_utf8,
							 curtoken->tokenbuf);

			if (!name)
			{
				free(reference);
				return -1;
			}
		}
		if (nexttoken()->tokentype != IT_EOL)	return (-1);

		if (strcmp(cmdbuf, "LIST"))
			list_flags |= LIST_SUBSCRIBED;

		rc=mailbox_scan(reference, name,
				list_flags,
				list_callback, cmdbuf);

		free(reference);
		free(name);
		if (rc == 0)
		{
			writes(tag);
			writes(" OK ");
			writes(cmdbuf);
			writes(" completed\r\n");
		}
		else
		{
			writes(tag);
			writes(" NO ");
			writes(strerror(errno));
			writes("\r\n");
			rc=0;
		}
		writeflush();
		return (rc);
	}

	if (strcmp(curtoken->tokenbuf, "APPEND") == 0)
	{
		struct	imaptoken *tok=nexttoken_nouc();
		struct maildir_info mi;
		char *mailbox;

		if (tok->tokentype != IT_NUMBER &&
			tok->tokentype != IT_ATOM &&
			tok->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    tok->tokenbuf);
		if (!mailbox ||
		    maildir_info_imap_find(&mi, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			if (mailbox)
				free(mailbox);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		if (mi.homedir && mi.maildir)
		{
			char *p=maildir_name2dir(mi.homedir, mi.maildir);
			int rc;

			if (!p)
			{
				maildir_info_destroy(&mi);
				writes(tag);
				accessdenied("APPEND",
					     mailbox,
					     ACL_INSERT);
				free(mailbox);
				return 0;
			}

			rc=append(tag, mailbox, p);
			free(p);
			maildir_info_destroy(&mi);
			free(mailbox);
			return (rc);
		}
		else if (mi.mailbox_type == MAILBOXTYPE_OLDSHARED)
		{
			char *p=strchr(mailbox, '.');

			if (p && (p=maildir_shareddir(".", p+1)) != NULL)
			{
				int rc;
				char	*q=malloc(strlen(p)+sizeof("/shared"));

				if (!q)	write_error_exit(0);

				strcat(strcpy(q, p), "/shared");
				free(p);
				rc=append(tag, mailbox, q);
				free(q);
				free(mailbox);
				maildir_info_destroy(&mi);
				return rc;
			}
		}
		free(mailbox);
		writes(tag);
		accessdenied("APPEND", "folder", ACL_INSERT);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "GETQUOTAROOT") == 0)
	{
		char	qroot[20];
		struct maildir_info minfo;
		char *mailbox;

		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);

		if (!mailbox ||
		    maildir_info_imap_find(&minfo, mailbox,
					   getenv("AUTHENTICATED")))
		{
			if (mailbox)
				free(mailbox);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		switch (minfo.mailbox_type) {
		case MAILBOXTYPE_INBOX:
			strcpy(qroot, "ROOT");
			break;
		case MAILBOXTYPE_OLDSHARED:
			strcpy(qroot, "SHARED");
			break;
		case MAILBOXTYPE_NEWSHARED:
			strcpy(qroot, "PUBLIC");
			break;
		}
		maildir_info_destroy(&minfo);

		writes("*");
		writes(" QUOTAROOT \"");
		writemailbox(mailbox);
		writes("\" \"");
		writes(qroot);
		writes("\"\r\n");
		quotainfo_out(qroot);
		writes(tag);
		writes(" OK GETQUOTAROOT Ok.\r\n");
		free(mailbox);
		return(0);
	}


	if (strcmp(curtoken->tokenbuf, "SETQUOTA") == 0)
	{
		writes(tag);
		writes(" NO SETQUOTA No permission.\r\n");
		return(0);
	}

	if (strcmp(curtoken->tokenbuf, "ENABLE") == 0)
	{
		while (nexttoken()->tokentype != IT_EOL)
		{
			switch (curtoken->tokentype) {
			case IT_NUMBER:
			case IT_ATOM:
			case IT_QUOTED_STRING:
				if (strcmp(curtoken->tokenbuf, "UTF8=ACCEPT")
				    == 0)
				{
					enabled_utf8=1;
				}
				continue;
			default:
				return -1;
			}
		}

		writes("* ENABLED");
		if (enabled_utf8)
			writes(" UTF8=ACCEPT");
		writes("\r\n");
		writes(tag);
		writes(" OK Options enabled\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "GETQUOTA") == 0)
	{
		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
			return (-1);

		quotainfo_out(curtoken->tokenbuf);
		writes(tag);
		writes(" OK GETQUOTA Ok.\r\n");
		return(0);
	}

	if (strcmp(curtoken->tokenbuf, "STATUS") == 0)
	{
		char	*mailbox;
		int	get_messages=0,
			get_recent=0,
			get_uidnext=0,
			get_uidvalidity=0,
			get_unseen=0;

		struct imapscaninfo other_info, *loaded_infoptr,
			*infoptr;
		const char *p;
		char	*orig_mailbox;
		int	oneonly;

		curtoken=nexttoken_nouc();
		mailbox=parse_mailbox_error(tag, curtoken, 0, 0);
		if ( mailbox == 0)
			return (0);

		orig_mailbox=imap_foldername_to_filename(enabled_utf8,
							 curtoken->tokenbuf);

		if (!orig_mailbox)
		{
			free(mailbox);
			return -1;
		}
		curtoken=nexttoken();

		oneonly=0;
		if (curtoken->tokentype != IT_LPAREN)
		{
			if (curtoken->tokentype != IT_ATOM)
			{
				free(mailbox);
				free(orig_mailbox);
				return (-1);
			}
			oneonly=1;
		}
		else	nexttoken();

		while ((curtoken=currenttoken())->tokentype == IT_ATOM)
		{
			if (strcmp(curtoken->tokenbuf, "MESSAGES") == 0)
				get_messages=1;
			if (strcmp(curtoken->tokenbuf, "RECENT") == 0)
				get_recent=1;
			if (strcmp(curtoken->tokenbuf, "UIDNEXT") == 0)
				get_uidnext=1;
			if (strcmp(curtoken->tokenbuf, "UIDVALIDITY") == 0)
				get_uidvalidity=1;
			if (strcmp(curtoken->tokenbuf, "UNSEEN") == 0)
				get_unseen=1;
			nexttoken();
			if (oneonly)	break;
		}

		if ((!oneonly && curtoken->tokentype != IT_RPAREN) ||
			nexttoken()->tokentype != IT_EOL)
		{
			free(mailbox);
			free(orig_mailbox);
			return (-1);
		}

		{
			CHECK_RIGHTSM(orig_mailbox, status_rights, ACL_READ);

			if (!status_rights[0])
			{
				writes(tag);
				accessdenied("STATUS", orig_mailbox,
					     ACL_READ);
				free(mailbox);
				free(orig_mailbox);
				return 0;
			}
		}


		if (current_mailbox && strcmp(current_mailbox, mailbox) == 0)
		{
			loaded_infoptr=0;
			infoptr= &current_maildir_info;
		}
		else
		{
			loaded_infoptr= &other_info;
			infoptr=loaded_infoptr;

			imapscan_init(loaded_infoptr);

			if (imapscan_maildir(infoptr, mailbox, 1, 1, NULL))
			{
				writes(tag);
				writes(" NO [ALERT] STATUS failed\r\n");
				free(mailbox);
				free(orig_mailbox);
				return (0);
			}
		}

		writes("*");
		writes(" STATUS \"");
		writemailbox(orig_mailbox);
		writes("\" (");
		p="";
		if (get_messages)
		{
			writes("MESSAGES ");
			writen(infoptr->nmessages+infoptr->left_unseen);
			p=" ";
		}
		if (get_recent)
		{
		unsigned long n=infoptr->left_unseen;
		unsigned long i;

			for (i=0; i<infoptr->nmessages; i++)
				if (infoptr->msgs[i].recentflag)
					++n;
			writes(p);
			writes("RECENT ");
			writen(n);
			p=" ";
		}

		if (get_uidnext)
		{
			writes(p);
			writes("UIDNEXT ");
			writen(infoptr->nextuid);
			p=" ";
		}

		if (get_uidvalidity)
		{
			writes(p);
			writes("UIDVALIDITY ");
			writen(infoptr->uidv);
			p=" ";
		}

		if (get_unseen)
		{
		unsigned long n=infoptr->left_unseen, i;

			for (i=0; i<infoptr->nmessages; i++)
			{
			const char *p=infoptr->msgs[i].filename;

				p=strrchr(p, MDIRSEP[0]);
				if (p && strncmp(p, MDIRSEP "2,", 3) == 0 &&
					strchr(p, 'S'))	continue;
				++n;
			}
			writes(p);
			writes("UNSEEN ");
			writen(n);
		}
		writes(")\r\n");
		if (loaded_infoptr)
			imapscan_free(loaded_infoptr);
		free(mailbox);
		free(orig_mailbox);
		writes(tag);
		writes(" OK STATUS Completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "CREATE") == 0)
	{
		char	*mailbox, *orig_mailbox, *p;
		int	isdummy;
		struct maildir_info mi;
		struct imapscaninfo minfo;

		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return 0;
		}

		isdummy=0;

		p=strrchr(curtoken->tokenbuf, HIERCH);
		if (p && p[1] == '\0')
		{
			*p=0;
			isdummy=1;	/* Ignore hierarchy creation */
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);
		if (!mailbox)
			return -1;

		if (maildir_info_imap_find(&mi, mailbox,
					   getenv("AUTHENTICATED")))
		{
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			free(mailbox);
			return (0);
		}
		free(mailbox);

		if (!mi.homedir || !mi.maildir)
		{
			maildir_info_destroy(&mi);
			writes(tag);
			accessdenied("CREATE",
				     curtoken->tokenbuf,
				     ACL_CREATE);
			maildir_info_destroy(&mi);
			return (0);
		}

		mailbox=maildir_name2dir(mi.homedir, mi.maildir);
		if (!mailbox)
		{
			writes(tag);
			writes(" NO Invalid mailbox name\r\n");
			maildir_info_destroy(&mi);
			return (0);
		}

		if (strcmp(mailbox, ".") == 0)
		{
			writes(tag);
			writes(" NO INBOX already exists!\r\n");
			free(mailbox);
			maildir_info_destroy(&mi);
			return (0);
		}

		if (check_parent_create(tag, "CREATE", curtoken->tokenbuf))
		{
			free(mailbox);
			maildir_info_destroy(&mi);
			return (0);
		}

		if (isdummy)	*p=HIERCH;
		orig_mailbox=imap_foldername_to_filename(enabled_utf8,
							 curtoken->tokenbuf);

		if (!orig_mailbox)
		{
			free(mailbox);
			maildir_info_destroy(&mi);
			return (-1);
		}

		if (nexttoken()->tokentype != IT_EOL)
		{
			free(mailbox);
			free(orig_mailbox);
			maildir_info_destroy(&mi);
			return (-1);
		}

		if (!isdummy)
		{
			int did_exist;
			maildir_aclt_list l;

			if ((did_exist=folder_exists(orig_mailbox)) != 0)
			{
				if (acl_read_folder(&l,
						    mi.homedir,
						    mi.maildir) < 0)
				{
					free(mailbox);
					free(orig_mailbox);
					writes(tag);
					writes(" NO Cannot create this folder"
					       ".\r\n");
					maildir_info_destroy(&mi);
					return (0);
				}
				maildir_acl_delete(mi.homedir, mi.maildir);
				/* Clear out fluff */
			}

			if (mdcreate(mailbox))
			{
				if (did_exist)
					maildir_aclt_list_destroy(&l);
				free(mailbox);
				free(orig_mailbox);
				writes(tag);
				writes(" NO Cannot create this folder.\r\n");
				maildir_info_destroy(&mi);
				return (0);
			}
			if (did_exist)
			{
				const char *acl_error;

				acl_write_folder(&l, mi.homedir,
						 mi.maildir, NULL,
						 &acl_error);
				maildir_aclt_list_destroy(&l);
			}
		}
		writes(tag);
		writes(" OK \"");
		writemailbox(orig_mailbox);
		writes("\" created.\r\n");

		/*
		** This is a dummy call to acl_read_folder that initialized
		** the default ACLs for this folder to its parent.
		*/

		{
			CHECK_RIGHTSM(curtoken->tokenbuf, create_rights,
				      ACL_CREATE);
		}

		imapscan_init(&minfo);
		imapscan_maildir(&minfo, mailbox, 0,0, NULL);
		imapscan_free(&minfo);

		free(mailbox);
		free(orig_mailbox);
		maildir_info_destroy(&mi);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "DELETE") == 0)
	{
	char	*mailbox;
	char	*p;
	char	*mailbox_name;

		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		p=strrchr(curtoken->tokenbuf, HIERCH);
		if (p && p[1] == '\0')		/* Ignore hierarchy DELETE */
		{
			if (nexttoken()->tokentype != IT_EOL)
				return (-1);
			writes(tag);
			writes(" OK Folder directory delete punted.\r\n");
			return (0);
		}

		mailbox=parse_mailbox_error(tag, curtoken, 1, 0);
		if ( mailbox == 0)
			return 0;

		mailbox_name=imap_foldername_to_filename(enabled_utf8,
							 curtoken->tokenbuf);

		if (!mailbox_name)
		{
			free(mailbox);
			return (-1);
		}
		if (nexttoken()->tokentype != IT_EOL)
		{
			free(mailbox_name);
			free(mailbox);
			return (-1);
		}

		if (current_mailbox && strcmp(mailbox, current_mailbox) == 0)
		{
			free(mailbox_name);
			free(mailbox);
			writes(tag);
			writes(" NO Cannot delete currently-open folder.\r\n");
			return (0);
		}

		if (strncmp(mailbox_name, SHARED HIERCHS,
			    sizeof(SHARED HIERCHS)-1) == 0)
		{
			maildir_shared_unsubscribe(0, mailbox_name + sizeof(SHARED HIERCHS)-1);
			free(mailbox_name);
			free(mailbox);
			writes(tag);
			writes(" OK UNSUBSCRIBEd a shared folder.\r\n");
			return (0);
		}

		{
			CHECK_RIGHTSM(curtoken->tokenbuf,
				      delete_rights,
				      ACL_DELETEFOLDER);
			if (delete_rights[0] == 0)
			{
				free(mailbox_name);
				free(mailbox);
				writes(tag);
				accessdenied("DELETE",
					     curtoken->tokenbuf,
					     ACL_DELETEFOLDER);
				return 0;
			}
		}

		if (!broken_uidvs())
			sleep(2); /* Make sure we never recycle them*/

		fetch_free_cache();


		if (do_folder_delete(mailbox_name))
		{
			writes(tag);
			writes(" NO Cannot delete this folder.\r\n");
		}
		else
		{
			writes(tag);
			writes(" OK Folder deleted.\r\n");
		}

		free(mailbox_name);
		free(mailbox);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "RENAME") == 0)
	{
		char *p;
		struct maildir_info mi1, mi2;
		const char *errmsg;
		char *mailbox;

		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
		    curtoken->tokentype != IT_ATOM &&
		    curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		if ((p=strrchr(curtoken->tokenbuf, HIERCH))  && p[1] == 0)
			*p=0;

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);

		if (!mailbox)
			return -1;

		if (maildir_info_imap_find(&mi1, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			free(mailbox);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		if (mi1.homedir == NULL || mi1.maildir == NULL)
		{
			free(mailbox);
			maildir_info_destroy(&mi1);
			writes(tag);
			writes(" NO Invalid mailbox\r\n");
			return (0);
		}

		{
			CHECK_RIGHTSM(mailbox,
				      rename_rights, ACL_DELETEFOLDER);

			if (rename_rights[0] == 0)
			{
				free(mailbox);
				maildir_info_destroy(&mi1);
				writes(tag);
				accessdenied("RENAME", curtoken->tokenbuf,
					     ACL_DELETEFOLDER);
				return (0);
			}
		}
		free(mailbox);

		curtoken=nexttoken_nouc();
		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			maildir_info_destroy(&mi1);
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		if ((p=strrchr(curtoken->tokenbuf, HIERCH)) && p[1] == 0)
		{
			*p=0;
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);

		if (!mailbox)
		{
			maildir_info_destroy(&mi1);
			return -1;
		}

		if (maildir_info_imap_find(&mi2, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			free(mailbox);
			maildir_info_destroy(&mi1);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		if (check_parent_create(tag, "RENAME", mailbox))
		{
			free(mailbox);
			maildir_info_destroy(&mi1);
			maildir_info_destroy(&mi2);
			return 0;
		}
		free(mailbox);

		if (nexttoken()->tokentype != IT_EOL)
		{
			maildir_info_destroy(&mi1);
			maildir_info_destroy(&mi2);
			return (-1);
		}

		if (!broken_uidvs())
			sleep(2);
		/* Make sure IMAP uidvs are not recycled */

		if (folder_rename(&mi1, &mi2, &errmsg))
		{
			writes(tag);
			writes(" NO ");
			writes(*errmsg == '@' ? errmsg+1:errmsg);
			if (*errmsg == '@')
				writes(strerror(errno));
			writes("\r\n");
		}
		else
		{
			writes(tag);
			writes(" OK Folder renamed.\r\n");
		}

		maildir_info_destroy(&mi1);
		maildir_info_destroy(&mi2);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "SELECT") == 0 ||
		strcmp(curtoken->tokenbuf, "EXAMINE") == 0)
	{
	char	*mailbox;
	int	ro=curtoken->tokenbuf[0] == 'E';
	const char *p;


		curtoken=nexttoken_nouc();

		if (current_mailbox)
		{
			free(current_mailbox);
			imapscan_free(&current_maildir_info);
			imapscan_init(&current_maildir_info);
			current_mailbox=0;
		}

		if (current_mailbox_acl)
			free(current_mailbox_acl);
		current_mailbox_acl=0;

		mailbox=parse_mailbox_error(tag, curtoken, 0, 1);
		if ( mailbox == 0)
			return (0);

		current_mailbox_acl=get_myrightson_folder(curtoken->tokenbuf);
		if (current_mailbox_acl == NULL)
		{
			free(mailbox);
			writes(tag);
			writes(" NO Unable to read ACLs for ");
			writes(curtoken->tokenbuf);
			writes(": ");
			writes(strerror(errno));
			writes("\r\n");
			return 0;
		}

		if (strchr(current_mailbox_acl, ACL_READ[0]) == NULL)
		{
			free(mailbox);
			free(current_mailbox_acl);
			current_mailbox_acl=NULL;
			writes(tag);
			accessdenied("SELECT/EXAMINE", curtoken->tokenbuf,
				     ACL_READ);
			return 0;
		}

		if (nexttoken()->tokentype != IT_EOL)
		{
			free(mailbox);
			return (-1);
		}

		if (imapscan_maildir(&current_maildir_info, mailbox, 0, ro,
				     NULL))
		{
			free(mailbox);
			writes(tag);
			writes(" NO Unable to open this mailbox.\r\n");
			return (0);
		}
		current_mailbox=mailbox;

		/* check if this is a shared read-only folder */

		if (is_sharedsubdir(mailbox) &&
			maildir_sharedisro(mailbox))
			ro=1;

		current_mailbox_ro=ro;

		mailboxflags(ro);
		mailboxmetrics();
		writes("* OK [UIDVALIDITY ");
		writen(current_maildir_info.uidv);
		writes("] Ok\r\n");
		myrights();
		writes(tag);

		for (p=current_mailbox_acl; *p; p++)
			if (strchr(ACL_INSERT ACL_EXPUNGE
				   ACL_SEEN ACL_WRITE ACL_DELETEMSGS,
				   *p))
				break;

		if (*p == 0)
			ro=1;

		writes(ro ? " OK [READ-ONLY] Ok\r\n":" OK [READ-WRITE] Ok\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "SUBSCRIBE") == 0)
	{
	char	*mailbox;
	char	*p;
	struct maildir_info mi;

		curtoken=nexttoken_nouc();
		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		p=strrchr(curtoken->tokenbuf, HIERCH);
		if (p && p[1] == '\0')		/* Ignore hierarchy DELETE */
		{
			if (nexttoken()->tokentype != IT_EOL)
				return (-1);
			writes(tag);
			writes(" OK Folder directory subscribe punted.\r\n");
			return (0);
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);
		if (!mailbox)
			return -1;
		if (nexttoken()->tokentype != IT_EOL)
			return (-1);

		if (maildir_info_imap_find(&mi, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			free(mailbox);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		if (mi.mailbox_type != MAILBOXTYPE_OLDSHARED)
		{
			maildir_info_destroy(&mi);
			subscribe(mailbox);
			free(mailbox);
			writes(tag);
			writes(" OK Folder subscribed.\r\n");
			return (0);
		}
		maildir_info_destroy(&mi);

		p=strchr(mailbox, '.');

		p=p ? maildir_shareddir(".", p+1):NULL;

		if (p == NULL || access(p, 0) == 0)
		{
			if (p)
				free(p);
			free(mailbox);
			writes(tag);
			writes(" OK Already subscribed.\r\n");
			return (0);
		}

		if (!p || maildir_shared_subscribe(0, strchr(mailbox, '.')+1))
		{
			if (p)
				free(p);
			free(mailbox);
			writes(tag);
			writes(" NO Cannot subscribe to this folder.\r\n");
			return (0);
		}
		if (p)
			free(p);
		free(mailbox);
		writes(tag);
		writes(" OK SUBSCRIBE completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "UNSUBSCRIBE") == 0)
	{
	char	*mailbox;
	char	*p;
	struct maildir_info mi;

		curtoken=nexttoken_nouc();
		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		p=strrchr(curtoken->tokenbuf, HIERCH);
		if (p && p[1] == '\0')		/* Ignore hierarchy DELETE */
		{
			if (nexttoken()->tokentype != IT_EOL)
				return (-1);
			writes(tag);
			writes(" OK Folder directory unsubscribe punted.\r\n");
			return (0);
		}

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);

		if (!mailbox)
			return -1;

		if (nexttoken()->tokentype != IT_EOL)
			return (-1);

		if (maildir_info_imap_find(&mi, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			free(mailbox);
			writes(tag);
			writes(" NO Invalid mailbox name.\r\n");
			return (0);
		}

		if (mi.mailbox_type != MAILBOXTYPE_OLDSHARED)
		{
			maildir_info_destroy(&mi);
			unsubscribe(mailbox);
			free(mailbox);
			writes(tag);
			writes(" OK Folder unsubscribed.\r\n");
			return (0);
		}
		maildir_info_destroy(&mi);

		p=strchr(mailbox, '.');

		p=p ? maildir_shareddir(".", p+1):NULL;


		if (p == NULL || access(p, 0))
		{
			if (p)
				free(p);
			free(mailbox);
			writes(tag);
			writes(" OK Already unsubscribed.\r\n");
			return (0);
		}

		fetch_free_cache();

		if (!p || maildir_shared_unsubscribe(0,
						     strchr(mailbox, '.')+1))
		{
			if (p)
				free(p);
			free(mailbox);
			writes(tag);
			writes(" NO Cannot subscribe to this folder.\r\n");
			return (0);
		}
		if (p)
			free(p);
		free(mailbox);
		writes(tag);
		writes(" OK UNSUBSCRIBE completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "NAMESPACE") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)
			return (-1);
		writes("* NAMESPACE ((\"INBOX.\" \".\")) NIL "
		       "((\"#shared.\" \".\")(\""
			SHARED ".\" \".\"))\r\n");
		writes(tag);
		writes(" OK NAMESPACE completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "ACL") == 0)
	{
		if (aclcmd(tag))
		{
			writes(tag);
			writes(" ACL FAILED: ");
			writes(strerror(errno));
			writes("\r\n");
		}
		return 0;
	}

	/* RFC 2086 */

	if (strcmp(curtoken->tokenbuf, "SETACL") == 0 ||
	    strcmp(curtoken->tokenbuf, "DELETEACL") == 0)
	{
		char *mailbox;
		char *identifier;
		struct maildir_info mi;
		maildir_aclt_list aclt_list;
		const char *acl_error;
		int doset=curtoken->tokenbuf[0] == 'S';
		const char *origcmd=doset ? "SETACL":"DELETEACL";

		curtoken=nexttoken_nouc();

		mailbox=parse_mailbox_error(tag, curtoken, 0, 0);
		if (!mailbox)
			return 0;
		free(mailbox);

		mailbox=imap_foldername_to_filename(enabled_utf8,
						    curtoken->tokenbuf);
		if (!mailbox)
			return -1;

		if (maildir_info_imap_find(&mi, mailbox,
					   getenv("AUTHENTICATED")) < 0)
		{
			writes(tag);
			writes(" NO Invalid mailbox.\r\n");
			free(mailbox);
			return 0;
		}

		if (mi.homedir == NULL || mi.maildir == NULL)
		{
			maildir_info_destroy(&mi);
			writes(tag);
			writes(" NO Cannot set ACLs for this mailbox\r\n");
			free(mailbox);
			return 0;
		}

		switch ((curtoken=nexttoken_nouc())->tokentype) {
		case IT_QUOTED_STRING:
		case IT_ATOM:
		case IT_NUMBER:
			break;
		default:
			maildir_info_destroy(&mi);
			free(mailbox);
			return -1;
		}

		identifier=acl2_identifier(tag, curtoken->tokenbuf);

		if (identifier == NULL)
		{
			maildir_info_destroy(&mi);
			free(mailbox);
			return 0;
		}

		if (doset)
		{
			switch ((curtoken=nexttoken_nouc())->tokentype) {
			case IT_QUOTED_STRING:
			case IT_ATOM:
			case IT_NUMBER:
				break;
			default:
				free(identifier);
				maildir_info_destroy(&mi);
				free(mailbox);
				return -1;
			}
		}

		{
			CHECK_RIGHTSM(mailbox,
				      acl_rights,
				      ACL_ADMINISTER);
			if (acl_rights[0] == 0)
			{
				writes(tag);
				accessdenied(origcmd, mailbox,
					     ACL_ADMINISTER);
				free(identifier);
				maildir_info_destroy(&mi);
				free(mailbox);
				return 0;
			}
		}

		if (acl_read_folder(&aclt_list, mi.homedir, mi.maildir))
		{
			writes(tag);
			writes(" NO Cannot read existing ACLs.\r\n");
			free(identifier);
			maildir_info_destroy(&mi);
			free(mailbox);
			return 0;
		}

		if (do_acl_mod(&aclt_list, &mi, identifier,
			       doset ? curtoken->tokenbuf:"",
			       &acl_error) < 0)
		{
			writes(tag);
			writes(acl_error ?
			       " NO Cannot modify ACLs as requested.\r\n" :
			       " NO Cannot modify ACLs on this mailbox.\r\n");
		}
		else
		{
			char *p=get_myrightson(mailbox);

			if (p)
				free(p);
			/* Side effect - change current folder's ACL */

			writes(tag);
			writes(" OK ACLs updated.\r\n");
		}

		maildir_aclt_list_destroy(&aclt_list);
		maildir_info_destroy(&mi);
		free(identifier);
		free(mailbox);
		return 0;
	}

	if (strcmp(curtoken->tokenbuf, "GETACL") == 0)
	{
		maildir_aclt_list l;
		char *mailbox_owner;
		char *mb;
		char *f;

		curtoken=nexttoken_nouc();

		mb=parse_mailbox_error(tag, curtoken, 0, 0);
		if (!mb)
			return 0;
		free(mb);

		f=imap_foldername_to_filename(enabled_utf8, curtoken->tokenbuf);

		if (!f)
			return -1;

		{
			CHECK_RIGHTSM(f,
				      acl_rights,
				      ACL_ADMINISTER);
			if (acl_rights[0] == 0)
			{
				writes(tag);
				accessdenied("GETACL", f,
					     ACL_ADMINISTER);
				return 0;
			}
		}

		if (get_acllist(&l, f,
				&mailbox_owner) < 0)
		{
			free(f);
			writes(tag);
			writes(" NO Cannot retrieve ACLs for mailbox.\r\n");
			return 0;
		}
		free(mailbox_owner);

		writes("* ACL \"");
		writemailbox(f);
		writes("\"");
		maildir_aclt_list_enum(&l, getacl_cb, NULL);
		writes("\r\n");
		writes(tag);
		writes(" OK GETACL completed.\r\n");
		maildir_aclt_list_destroy(&l);
		free(f);
		return 0;
	}

	if (strcmp(curtoken->tokenbuf, "LISTRIGHTS") == 0)
	{
		maildir_aclt_list l;
		char *mailbox_owner;
		char *mb;

		curtoken=nexttoken_nouc();

		mb=parse_mailbox_error(tag, curtoken, 0, 0);
		if (!mb)
			return 0;
		free(mb);
		mb=imap_foldername_to_filename(enabled_utf8,
					       curtoken->tokenbuf);

		if (!mb)
			return -1;

		{
			char *myrights=get_myrightson(mb);

			if (!strchr(myrights, ACL_LOOKUP[0]) &&
			    !strchr(myrights, ACL_READ[0]) &&
			    !strchr(myrights, ACL_INSERT[0]) &&
			    !strchr(myrights, ACL_CREATE[0]) &&
			    !strchr(myrights, ACL_DELETEFOLDER[0]) &&
			    !strchr(myrights, ACL_EXPUNGE[0]) &&
			    !strchr(myrights, ACL_ADMINISTER[0]))
			{
				free(myrights);
				writes(tag);
				accessdenied("GETACL", mb,
					     ACL_ADMINISTER);
				free(mb);
				return 0;
			}
			free(myrights);
		}

		if (get_acllist(&l, mb,
				&mailbox_owner) < 0)
		{
			free(mb);
			writes(tag);
			writes(" NO Cannot retrieve ACLs for mailbox.\r\n");
			return 0;
		}

		switch ((curtoken=nexttoken_nouc())->tokentype) {
		case IT_QUOTED_STRING:
		case IT_ATOM:
		case IT_NUMBER:
			break;
		default:
			free(mb);
			free(mailbox_owner);
			maildir_aclt_list_destroy(&l);
			return -1;
		}

		writes("* LISTRIGHTS \"");
		writemailbox(mb);
		writes("\" \"");
		writeqs(curtoken->tokenbuf);
		writes("\"");
		free(mb);


		if (curtoken->tokenbuf[0] == '-' &&
		    (MAILDIR_ACL_ANYONE(curtoken->tokenbuf+1) ||
		     (strncmp(mailbox_owner, "user=", 5) == 0 &&
		      strcmp(curtoken->tokenbuf+1, mailbox_owner+5) == 0)))
		{
			writes(" \"\" "
			       ACL_CREATE " "
			       ACL_DELETE_SPECIAL " "
			       ACL_INSERT " "
			       ACL_POST " "
			       ACL_READ " "
			       ACL_SEEN " "
			       ACL_WRITE "\r\n");
		}
		else if (strncmp(mailbox_owner, "user=", 5) == 0 &&
			 strcmp(curtoken->tokenbuf, mailbox_owner+5) == 0)
		{
			writes(" \""
			       ACL_ADMINISTER
			       ACL_LOOKUP "\" "
			       ACL_CREATE " "
			       ACL_DELETE_SPECIAL " "
			       ACL_INSERT " "
			       ACL_POST " "
			       ACL_READ " "
			       ACL_SEEN " "
			       ACL_WRITE "\r\n");
		}
		else
		{
			writes(" \"\" "
			       ACL_ADMINISTER " "
			       ACL_CREATE " "
			       ACL_DELETE_SPECIAL " "
			       ACL_INSERT " "
			       ACL_LOOKUP " "
			       ACL_POST " "
			       ACL_READ " "
			       ACL_SEEN " "
			       ACL_WRITE "\r\n");
		}
		writes(tag);
		writes(" OK LISTRIGHTS completed.\r\n");
		free(mailbox_owner);
		maildir_aclt_list_destroy(&l);
		return 0;
	}

	if (strcmp(curtoken->tokenbuf, "MYRIGHTS") == 0)
	{
		char *mb;
		char *f;

		curtoken=nexttoken_nouc();

		mb=parse_mailbox_error(tag, curtoken, 0, 0);
		if (!mb)
			return 0;
		free(mb);

		f=imap_foldername_to_filename(enabled_utf8, curtoken->tokenbuf);
		if (!f)
			return -1;

		{
			char *myrights=get_myrightson(f);

			if (!strchr(myrights, ACL_LOOKUP[0]) &&
			    !strchr(myrights, ACL_READ[0]) &&
			    !strchr(myrights, ACL_INSERT[0]) &&
			    !strchr(myrights, ACL_CREATE[0]) &&
			    !strchr(myrights, ACL_DELETEFOLDER[0]) &&
			    !strchr(myrights, ACL_EXPUNGE[0]) &&
			    !strchr(myrights, ACL_ADMINISTER[0]))
			{
				free(myrights);
				writes(tag);
				accessdenied("GETACL", f,
					     ACL_ADMINISTER);
				free(f);
				return 0;
			}
			free(myrights);
		}

		mb=get_myrightson(f);

		if (!mb)
		{
			free(f);
			writes(tag);
			writes(" NO Cannot retrieve ACLs for mailbox.\r\n");
			return 0;
		}

		writes("* MYRIGHTS \"");
		writemailbox(f);
		writes("\" \"");
		free(f);

		writeacl1(mb);
		free(mb);
		writes("\"\r\n");
		writes(tag);
		writes(" OK MYRIGHTS completed.\r\n");
		return 0;
	}

	/* mailbox commands */

	if (current_mailbox == 0)	return (-1);

	if (strcmp(curtoken->tokenbuf, "UID") == 0)
	{
		uid=1;
		if ((curtoken=nexttoken())->tokentype != IT_ATOM)
			return (-1);
		if (strcmp(curtoken->tokenbuf, "COPY") &&
		    strcmp(curtoken->tokenbuf, "FETCH") &&
		    strcmp(curtoken->tokenbuf, "SEARCH") &&
		    strcmp(curtoken->tokenbuf, "THREAD") &&
		    strcmp(curtoken->tokenbuf, "SORT") &&
		    strcmp(curtoken->tokenbuf, "STORE") &&
		    strcmp(curtoken->tokenbuf, "EXPUNGE"))
			return (-1);
	}

	if (strcmp(curtoken->tokenbuf, "CLOSE") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)
			return (-1);

		if (!current_mailbox_ro
		    && strchr(current_mailbox_acl, ACL_EXPUNGE[0]))
			expunge();
		free(current_mailbox);
		imapscan_free(&current_maildir_info);
		imapscan_init(&current_maildir_info);
		current_mailbox=0;
		writes(tag);
		writes(" OK mailbox closed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "FETCH") == 0)
	{
	struct fetchinfo *fi;
	char	*msgset;

		curtoken=nexttoken();
		if (!ismsgset(curtoken))	return (-1);
		msgset=my_strdup(curtoken->tokenbuf);

		if ((curtoken=nexttoken())->tokentype != IT_LPAREN)
		{
			if (curtoken->tokentype != IT_ATOM)
			{
				free(msgset);
				return (-1);
			}
			fi=fetchinfo_alloc(1);
		}
		else
		{
			(void)nexttoken();
			fi=fetchinfo_alloc(0);
			if (fi && currenttoken()->tokentype != IT_RPAREN)
			{
				fetchinfo_free(fi);
				fi=0;
			}
			nexttoken();
		}

		if (fi == 0 || currenttoken()->tokentype != IT_EOL)
		{
			free(msgset);
			if (fi)	fetchinfo_free(fi);
			return (-1);
		}

		do_msgset(msgset, &do_fetch, fi, uid);
		fetchinfo_free(fi);
		free(msgset);
		writes(tag);
		writes(" OK FETCH completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "STORE") == 0)
	{
	char	*msgset;
	struct storeinfo storeinfo_s;

		curtoken=nexttoken();
		if (!ismsgset(curtoken))	return (-1);
		msgset=my_strdup(curtoken->tokenbuf);

		(void)nexttoken();
		current_maildir_info.keywordList->keywordAddedRemoved=0;

		if (storeinfo_init(&storeinfo_s) ||
			currenttoken()->tokentype != IT_EOL)
		{
			if (storeinfo_s.keywords)
				libmail_kwmDestroy(storeinfo_s.keywords);
			free(msgset);
			return (-1);
		}

		/* Do not change \Deleted if this is a readonly mailbox */

		if (current_mailbox_ro && storeinfo_s.flags.deleted)
		{
			if (storeinfo_s.keywords)
				libmail_kwmDestroy(storeinfo_s.keywords);
			free(msgset);
			writes(tag);
			writes(" NO Current box is selected READ-ONLY.\r\n");
			return (0);
		}

		fetch_free_cache();

		if (current_maildir_info.keywordList->keywordAddedRemoved)
			mailboxflags(current_mailbox_ro);

		current_maildir_info.keywordList->keywordAddedRemoved=0;

		if (do_msgset(msgset, &do_store, &storeinfo_s, uid))
		{
			if (storeinfo_s.keywords)
				libmail_kwmDestroy(storeinfo_s.keywords);
			free(msgset);
			if (current_maildir_info.keywordList
			    ->keywordAddedRemoved)
				mailboxflags(current_mailbox_ro);

			writes(tag);
			writes(" NO [ALERT] You exceeded your mail quota.\r\n");
			return (0);
		}
		if (storeinfo_s.keywords)
		{
			struct imap_addRemoveKeywordInfo imapInfo;

			switch (storeinfo_s.plusminus) {
			case '+':
			case '-':

				imapInfo.msgset=msgset;
				imapInfo.uid=uid;

				if (!fastkeywords() &&
				    addRemoveKeywords(&imap_addRemoveKeywords,
						      &imapInfo, &storeinfo_s))
				{
					libmail_kwmDestroy(storeinfo_s.keywords);
					free(msgset);
					writes(tag);
					writes(" NO An error occured while"
					       " updating keywords: ");
					writes(strerror(errno));
					writes(".\r\n");
					return 0;
				}
				break;
			}

			libmail_kwmDestroy(storeinfo_s.keywords);
		}
		free(msgset);
		if (current_maildir_info.keywordList->keywordAddedRemoved)
			mailboxflags(current_mailbox_ro);
		writes(tag);
		writes(" OK STORE completed.\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "SEARCH") == 0)
	{
		char *charset=0;
		struct searchinfo *si, *sihead;
		unsigned long i;

		curtoken=nexttoken_okbracket();
		if (curtoken->tokentype == IT_ATOM &&
			strcmp(curtoken->tokenbuf, "CHARSET") == 0)
		{
			if (enabled_utf8)
			{
				writes(tag);
				writes(" NO CHARSET is not valid in UTF8 mode "
				       "as per RFC 6855\r\n");
				return (0);
			}

			curtoken=nexttoken();
			if (curtoken->tokentype != IT_ATOM &&
				curtoken->tokentype != IT_QUOTED_STRING)
				return (-1);

			charset=my_strdup(curtoken->tokenbuf);
			curtoken=nexttoken();
		}

		if (validate_charset(tag, &charset))
		{
			if (charset)
				free(charset);
			return (0);
		}

		if ((si=alloc_parsesearch(&sihead)) == 0)
		{
			if (charset)	free(charset);
			return (-1);
		}
		if (currenttoken()->tokentype != IT_EOL)
		{
			if (charset)	free(charset);
			free_search(sihead);
			return (-1);
		}

#if 0
		writes("* OK ");
		debug_search(si);
		writes("\r\n");
#endif
		writes("* SEARCH");
		dosearch(si, sihead, charset, uid);
		writes("\r\n");
		if (charset)
			free(charset);

		for (i=0; i<current_maildir_info.nmessages; i++)
			if (current_maildir_info.msgs[i].changedflags)
				fetchflags(i);
		writes(tag);
		writes(" OK SEARCH done.\r\n");
		free_search(sihead);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "THREAD") == 0)
	{
	char *charset=0;
	struct searchinfo *si, *sihead;
	unsigned long i;

		/* The following jazz is mainly for future extensions */

	void (*thread_func)(struct searchinfo *, struct searchinfo *,
			    const char *, int);
	search_type thread_type;

		{
		const char *p=getenv("IMAP_DISABLETHREADSORT");
		int n= p ? atoi(p):0;

			if (n > 0)
			{
				writes(tag);
				writes(" NO This command is disabled by the system administrator.\r\n");
				return (0);
			}
		}

		curtoken=nexttoken();
		if (curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
			return (-1);

		if (strcmp(curtoken->tokenbuf, "ORDEREDSUBJECT") == 0)
		{
			thread_func=dothreadorderedsubj;
			thread_type=search_orderedsubj;
		}
		else if (strcmp(curtoken->tokenbuf, "REFERENCES") == 0)
		{
			thread_func=dothreadreferences;
			thread_type=search_references1;
		}
		else
		{
			return (-1);
		}

		curtoken=nexttoken();
		if (curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
			return (-1);

		charset=my_strdup(curtoken->tokenbuf);
		curtoken=nexttoken();

		if ((si=alloc_parsesearch(&sihead)) == 0)
		{
			if (charset)	free(charset);
			return (-1);
		}

		si=alloc_searchextra(si, &sihead, thread_type);

		if (currenttoken()->tokentype != IT_EOL)
		{
			if (charset)	free(charset);
			free_search(sihead);
			return (-1);
		}

		if (validate_charset(tag, &charset))
		{
			if (charset)
				free(charset);
			return (0);
		}

		writes("* THREAD ");
		(*thread_func)(si, sihead, charset, uid);
		writes("\r\n");
		free(charset);

		for (i=0; i<current_maildir_info.nmessages; i++)
			if (current_maildir_info.msgs[i].changedflags)
				fetchflags(i);
		writes(tag);
		writes(" OK THREAD done.\r\n");
		free_search(sihead);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "SORT") == 0)
	{
	char *charset=0;
	struct searchinfo *si, *sihead;
	unsigned long i;
	struct temp_sort_stack *ts=0;

		{
		const char *p=getenv("IMAP_DISABLETHREADSORT");
		int n= p ? atoi(p):0;

			if (n > 0)
			{
				writes(tag);
				writes(" NO This command is disabled by the system administrator.\r\n");
				return (0);
			}
		}

		curtoken=nexttoken();
		if (curtoken->tokentype != IT_LPAREN)	return (-1);
		while ((curtoken=nexttoken())->tokentype != IT_RPAREN)
		{
		search_type st;
		struct temp_sort_stack *newts;

			if (curtoken->tokentype != IT_ATOM &&
				curtoken->tokentype != IT_QUOTED_STRING)
			{
				free_temp_sort_stack(ts);
				return (-1);
			}

			if (strcmp(curtoken->tokenbuf, "SUBJECT") == 0)
			{
				st=search_orderedsubj;
			}
			else if (strcmp(curtoken->tokenbuf, "ARRIVAL") == 0)
			{
				st=search_arrival;
			}
			else if (strcmp(curtoken->tokenbuf, "CC") == 0)
			{
				st=search_cc;
			}
			else if (strcmp(curtoken->tokenbuf, "DATE") == 0)
			{
				st=search_date;
			}
			else if (strcmp(curtoken->tokenbuf, "FROM") == 0)
			{
				st=search_from;
			}
			else if (strcmp(curtoken->tokenbuf, "REVERSE") == 0)
			{
				st=search_reverse;
			}
			else if (strcmp(curtoken->tokenbuf, "SIZE") == 0)
			{
				st=search_size;
			}
			else if (strcmp(curtoken->tokenbuf, "TO") == 0)
			{
				st=search_to;
			}
			else
			{
				free_temp_sort_stack(ts);
				return (-1);
			}

			newts=(struct temp_sort_stack *)malloc(
				sizeof(struct temp_sort_stack));
			if (!newts)	write_error_exit(0);
			newts->next=ts;
			newts->type=st;
			ts=newts;
		}

		if (ts == 0	/* No criteria */
			|| ts->type == search_reverse)
				/* Can't end with the REVERSE keyword */
		{
			free_temp_sort_stack(ts);
			return (-1);
		}

		curtoken=nexttoken();
		if (curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			free_temp_sort_stack(ts);
			return (-1);
		}

		charset=my_strdup(curtoken->tokenbuf);
		curtoken=nexttoken();

		if ((si=alloc_parsesearch(&sihead)) == 0)
		{
			if (charset)	free(charset);
			free_temp_sort_stack(ts);
			return (-1);
		}

		while (ts)
		{
		struct temp_sort_stack *cts=ts;

			ts=cts->next;
			si=alloc_searchextra(si, &sihead, cts->type);
			free(cts);
		}

		if (currenttoken()->tokentype != IT_EOL)
		{
			if (charset)	free(charset);
			free_search(sihead);
			return (-1);
		}

		if (validate_charset(tag, &charset))
		{
			if (charset) free(charset);
			free_search(sihead);
			return (0);
		}

		writes("* SORT");
		dosortmsgs(si, sihead, charset, uid);
		writes("\r\n");
		free(charset);

		for (i=0; i<current_maildir_info.nmessages; i++)
			if (current_maildir_info.msgs[i].changedflags)
				fetchflags(i);
		writes(tag);
		writes(" OK SORT done.\r\n");
		free_search(sihead);
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "CHECK") == 0)
	{
		if (nexttoken()->tokentype != IT_EOL)	return (-1);
		doNoop(0);
		writes(tag);
		writes(" OK CHECK completed\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "EXPUNGE") == 0)
	{
		if (strchr(current_mailbox_acl, ACL_EXPUNGE[0]) == NULL)
		{
			writes(tag);
			accessdenied("EXPUNGE", "current mailbox",
				     ACL_EXPUNGE);
			return 0;
		}

		if (current_mailbox_ro)
		{
			writes(tag);
			writes(" NO Cannot expunge read-only mailbox.\r\n");
			return 0;
		}

		if (uid)
		{
			char *msgset;

			curtoken=nexttoken();
			if (!ismsgset(curtoken))	return (-1);
			msgset=my_strdup(curtoken->tokenbuf);
			if (nexttoken()->tokentype != IT_EOL)	return (-1);

			do_msgset(msgset, &uid_expunge, NULL, 1);
			free(msgset);
		}
		else
		{
			if (nexttoken()->tokentype != IT_EOL)	return (-1);
			expunge();
		}
		doNoop(0);
		writes(tag);
		writes(" OK EXPUNGE completed\r\n");
		return (0);
	}

	if (strcmp(curtoken->tokenbuf, "COPY") == 0)
	{
	struct maildirsize quotainfo;
	char	*mailbox;
	char	*msgset;
	struct copyquotainfo cqinfo;
	int	has_quota;
	int	isshared;
	struct do_copy_info copy_info;
	unsigned long copy_uidv;
	char access_rights[8];

		curtoken=nexttoken();
		if (!ismsgset(curtoken))	return (-1);
		msgset=my_strdup(curtoken->tokenbuf);

		curtoken=nexttoken_nouc();

		if (curtoken->tokentype != IT_NUMBER &&
			curtoken->tokentype != IT_ATOM &&
			curtoken->tokentype != IT_QUOTED_STRING)
		{
			free(msgset);
			writes(tag);
			writes(" BAD Invalid command\r\n");
			return (0);
		}

		mailbox=decode_valid_mailbox(curtoken->tokenbuf, 1);

		if (!mailbox)
		{
			struct maildir_info mi;

			free(msgset);

			if (maildir_info_imap_find(&mi, curtoken->tokenbuf,
						   getenv("AUTHENTICATED"))
			    == 0)
			{
				if (nexttoken()->tokentype == IT_EOL)
				{
					maildir_info_destroy(&mi);
					writes(tag);
					writes(" NO [TRYCREATE] Mailbox does not exist.\r\n");
					return (0);
				}
				maildir_info_destroy(&mi);
			}
			return (-1);
		}

		{
			char *f=imap_foldername_to_filename(enabled_utf8,
							    curtoken->tokenbuf);
			if (!f)
				return -1;
			CHECK_RIGHTSM(f,
				      append_rights,
				      ACL_INSERT ACL_DELETEMSGS
				      ACL_SEEN ACL_WRITE);

			if (strchr(append_rights, ACL_INSERT[0]) == NULL)
			{
				writes(tag);
				accessdenied("COPY",
					     f,
					     ACL_INSERT);
				free(f);
				return 0;
			}
			free(f);
			strcpy(access_rights, append_rights);
		}

		if (nexttoken()->tokentype != IT_EOL)
		{
			free(msgset);
			return (-1);
		}

		if (access(mailbox, 0))
		{
			writes(tag);
			writes(" NO [TRYCREATE] Mailbox does not exist.\r\n");
			free(msgset);
			free(mailbox);
			return (0);
		}

		fetch_free_cache();
		cqinfo.destmailbox=mailbox;
		cqinfo.acls=access_rights;

		/*
		** If the destination is a shared folder, copy it into the
		** real shared folder.
		*/

		isshared=0;
		if (is_sharedsubdir(cqinfo.destmailbox))
		{
		char	*p=malloc(strlen(cqinfo.destmailbox)+sizeof("/shared"));

			if (!p)	write_error_exit(0);
			strcat(strcpy(p, cqinfo.destmailbox), "/shared");

			free(mailbox);
			mailbox=cqinfo.destmailbox=p;
			isshared=1;
		}

		cqinfo.nbytes=0;
		cqinfo.nfiles=0;

		has_quota=0;
		if (!isshared && maildirquota_countfolder(cqinfo.destmailbox))
		{
			if (maildir_openquotafile(&quotainfo,
						  ".") == 0)
			{
				if (quotainfo.fd >= 0)
					has_quota=1;
				maildir_closequotafile(&quotainfo);
			}

			if (has_quota > 0 &&
			    do_msgset(msgset, &do_copy_quota_calc, &cqinfo,
				      uid))
				has_quota= -1;
		}

		if (has_quota > 0 && cqinfo.nfiles > 0)
		{

			if (maildir_quota_add_start(".", &quotainfo,
						    cqinfo.nbytes,
						    cqinfo.nfiles,
						    getenv("MAILDIRQUOTA")))
			{
				writes(tag);
				writes(
			" NO [ALERT] You exceeded your mail quota.\r\n");
				free(msgset);
				free(mailbox);
				return (0);
			}

			maildir_quota_add_end(&quotainfo,
					      cqinfo.nbytes,
					      cqinfo.nfiles);
		}

		if (is_outbox(mailbox))
		{
			int counter=0;

			if (do_msgset(msgset, &do_count, &counter, uid) ||
			    counter > 1)
			{
				writes(tag);
				writes(" NO [ALERT] Only one message may be sent at a time.\r\n");
				free(msgset);
				free(mailbox);
				return (0);
			}
		}

		copy_info.mailbox=mailbox;
		copy_info.uidplus_list=NULL;
		copy_info.uidplus_tail= &copy_info.uidplus_list;
		copy_info.acls=access_rights;

		if (has_quota < 0 ||
		    do_msgset(msgset, &do_copy_message, &copy_info, uid) ||
		    uidplus_fill(copy_info.mailbox, copy_info.uidplus_list,
				 &copy_uidv))
		{
			uidplus_abort(copy_info.uidplus_list);
			writes(tag);
			writes(" NO [ALERT] COPY failed - no write permission or out of disk space.\r\n");
			free(msgset);
			free(mailbox);
			return (0);
		}

		dirsync(mailbox);

		writes(tag);
		writes(" OK");

		if (copy_info.uidplus_list != NULL)
		{
			writes(" [COPYUID ");
			writen(copy_uidv);
			uidplus_writemsgset(copy_info.uidplus_list, 0);
			uidplus_writemsgset(copy_info.uidplus_list, 1);
			writes("]");
		}

		writes(" COPY completed.\r\n");
		uidplus_free(copy_info.uidplus_list);

		free(msgset);
		free(mailbox);
		return (0);
	}
	return (-1);
}

static void dogethostname()
{
char	buf[2048];
char	*p;

	if (gethostname(buf, sizeof(buf)) < 0)
		strcpy(buf, "courier-imap");
	p=malloc(strlen(buf)+sizeof("HOSTNAME="));
	if (!p)
		write_error_exit(0);
	strcat(strcpy(p, "HOSTNAME="), buf);
	putenv(p);
}

#if 0
static char *getcurdir()
{
char	*pathbuf=0;
size_t	pathlen=256;

	for (;;)
	{
		if ((pathbuf=pathbuf ? realloc(pathbuf, pathlen):
			malloc(pathlen)) == 0)
			write_error_exit(0);
		if (getcwd(pathbuf, pathlen-1))
			return (pathbuf);
		if (errno != ERANGE)
		{
			free(pathbuf);
			return (0);
		}
		pathlen += 256;
	}
}

static char *getimapscanpath(const char *argv0)
{
char	*p, *q;

	if (*argv0 != '/')
	{
		p=getcurdir();
		if (!p)
		{
			perror("getcwd");
			exit(1);
		}
		q=malloc(strlen(p)+strlen(argv0)+sizeof("//imapscan"));
		if (!q)	write_error_exit(0);
		strcat(strcat(strcpy(q, p), "/"), argv0);
	}
	else
	{
		q=malloc(strlen(argv0)+sizeof("imapscan"));
		if (!q)	write_error_exit(0);
		strcpy(q, argv0);
	}
	p=strrchr(q, '/');
	if (p && p[1] == 0)
	{
		*p=0;
		p=strrchr(q, '/');
	}

	if (p)
		p[1]=0;
	else	*q=0;

	strcat(q, "imapscan");
	return (q);
}
#endif

static void chkdisabled(const char *ip, const char *port)
{
	char *p1, *p2;

	p1 = authgetoptionenv("disableimap");
	if (p1 && atoi(p1))
	{
		writes("* BYE IMAP access disabled for this account.\r\n");
		writeflush();
		exit(0);
	}
	if (p1)
		free(p1);
	p1 = authgetoptionenv("disableinsecureimap");
	if (p1 && atoi(p1))
	{
		if (!(p2 = getenv("IMAP_TLS")) || !atoi(p2))
		{
			writes("* BYE IMAP access disabled via insecure connection.\r\n");
			writeflush();
			free(p1);
			exit(0);
		}
	}
	if (p1)
		free(p1);
	fprintf(stderr, "INFO: LOGIN, user=%s, ip=[%s], port=[%s], protocol=%s\n",
		getenv("AUTHENTICATED"), ip, port, protocol);
}

static int chk_clock_skew()
{
	static const char fn[]="tmp/courier-imap.clockskew.chk";
	struct stat stat_buf;
	int fd;
	time_t t;

	unlink(fn);
	fd=open(fn, O_RDWR|O_TRUNC|O_CREAT, 0666);
	time(&t);

	if (fd < 0)
		return 0; /* Something else is wrong */

	if (fstat(fd, &stat_buf) < 0)
	{
		close(fd);
		return -1; /* Something else is wrong */
	}
	close(fd);
	unlink(fn);

	if (stat_buf.st_mtime < t - 30 || stat_buf.st_mtime > t+30)
		return -1;
	return 0;
}

#if SMAP

static int is_smap()
{
	const char *p;

	p=getenv("PROTOCOL");

	if (p && strcmp(p, "SMAP1") == 0)
		return 1;
	return 0;
}

#else

#define is_smap() 0

#endif


int main(int argc, char **argv)
{
	const char *ip;
	const char *p;
	const char *tag;
	const char *port;
	mode_t oldumask;
	uid_t           euid, uid;

	if (!(euid = geteuid()))
	{
		if (euid != (uid = getuid()))
		{
			if (setuid(uid))
			{
				fprintf(stderr, "imapd: setuid: %s\n", strerror(errno));
				exit(1);
			}
		}
	}
#ifdef HAVE_SETVBUF_IOLBF
	setvbuf(stderr, NULL, _IOLBF, BUFSIZ);
#endif
	time(&start_time);
	if (argc > 1 && strcmp(argv[1], "--version") == 0)
	{
		printf("%s\n", PROGRAMVERSION);
		exit(0);
	}

	if ((tag=getenv("IMAPLOGINTAG")) != 0)
	{
		if (getenv("AUTHENTICATED") == NULL)
		{
			printf("* BYE AUTHENTICATED environment variable not set.\r\n");
			fflush(stdout);
			exit(0);
		}
		authmodclient();
	} else
	{
		const char *p;

		putenv("TCPREMOTEIP=127.0.0.1");
		putenv("TCPREMOTEPORT=0");

		p=getenv("AUTHENTICATED");
		if (!p || !*p)
		{
			struct passwd *pw=getpwuid(getuid());
			char *me;

			if (!pw)
			{
				fprintf(stderr,
					"ERR: uid %lu not found in passwd file\n",
					(unsigned long)getuid());
				exit(1);
			}

			me=malloc(sizeof("AUTHENTICATED=")+strlen(pw->pw_name));
			if (!me)
				write_error_exit(0);

			strcat(strcpy(me, "AUTHENTICATED="), pw->pw_name);
			putenv(me);
		}
	}

#if HAVE_SETLOCALE
	setlocale(LC_CTYPE, "C");
#endif

	ip=getenv("TCPREMOTEIP");
	if (!ip || !*ip)	exit(0);

	port=getenv("TCPREMOTEPORT");
	if (!port || !*port)	exit(0);

	protocol=getenv("PROTOCOL");

	if (!protocol || !*protocol)
		protocol="IMAP";

	putenv("IMAP_STARTTLS=NO");	/* No longer grok STARTTLS */

	/* We use select() with a timeout, so use non-blocking filedescs */

	if (fcntl(0, F_SETFL, O_NONBLOCK) ||
	    fcntl(1, F_SETFL, O_NONBLOCK))
	{
		perror("fcntl");
		exit(1);
	}

	{
	struct	stat	buf;

		if ( stat(".", &buf) < 0 || buf.st_mode & S_ISVTX)
		{
			fprintf(stderr, "INFO: LOCKED, user=%s, ip=[%s], port=[%s]\n",
				getenv("AUTHENTICATED"), ip, port);
			if (is_smap())
				writes("-ERR ");
			else
				writes("* BYE ");
			writes("* BYE Your account is temporarily unavailable (+t bit set on home directory).\r\n");
			writeflush();
			exit(0);
		}
	}

	if (argc > 1)
		p=argv[1];
	else
		p=getenv("MAILDIR");
	if (!p)
		p="./Maildir";
#if 0
	imapscanpath=getimapscanpath(argv[0]);
#endif
	if (chdir(p))
	{
		fprintf(stderr, "chdir %s: %s\n", p, strerror(errno));
		write_error_exit(strerror(errno));
	}
	maildir_loginexec();

	p=authgetoptionenv("disableshared");
	if (p && atoi(p))
	{
		maildir_acl_disabled=1;
		maildir_newshared_disabled=1;
	}
	if (p)
		free((char *) p);

	/* Remember my device/inode */

	{
		struct	stat	buf;

		if ( stat(".", &buf) < 0)
			write_error_exit("Cannot stat current directory");

		homedir_dev=buf.st_dev;
		homedir_ino=buf.st_ino;

		errno=0;

		p=getenv("IMAP_MAILBOX_SANITY_CHECK");

		if (!p || !*p) p="1";

		if (atoi(p))
		{
			if ( buf.st_uid != geteuid() ||
			     buf.st_gid != getegid())
			{
				fprintf(stderr, "uid %d euid %d\n", getuid(), geteuid());
				write_error_exit("Account's mailbox directory is not owned by the correct uid or gid");
			}
		}
	}

	p=getenv("HOSTNAME");
	if (!p)
		dogethostname();

	if ((p=getenv("IMAP_TRASHFOLDERNAME")) != 0 && *p)
	{
		trash = strdup(p);
		dot_trash = malloc(strlen(trash) + 2);
		dot_trash[0] = '.';
		strcpy(&dot_trash[1], trash); 
	}
	mdcreate(dot_trash);
	
#if 1
	mdcreate("." DRAFTS);
	mdcreate("." SENT);
	mdcreate("." "Spam");
#endif

	if ((p=getenv("IMAPDEBUGFILE")) != 0 && *p &&
	    access(p, 0) == 0)
	{
		oldumask = umask(027);
		debugfile=fopen(p, "a");
		umask(oldumask);
		if (debugfile==NULL)
			write_error_exit(0);
	}
	initcapability();

	emptytrash();
	signal(SIGPIPE, SIG_IGN);

	libmail_kwVerbotten=KEYWORD_IMAPVERBOTTEN;
	libmail_kwCaseSensitive=0;

	if (!keywords())
		libmail_kwEnabled=0;

	maildir_info_munge_complex((p=getenv("IMAP_SHAREDMUNGENAMES")) &&
				   atoi(p));

#if SMAP
	if (is_smap())
	{
		if (chk_clock_skew())
		{
			writes("-ERR Clock skew detected. Check the clock on the file server\r\n");
			writeflush();
			exit(0);
		}

		writes("+OK SMAP1 LOGIN Ok.\n");

		smapflag=1;

		libmail_kwVerbotten=KEYWORD_SMAPVERBOTTEN;
		libmail_kwCaseSensitive=1;

		chkdisabled(ip, port);
		smap();
		logoutmsg();
		emptytrash();
		return (0);
	}
#endif

	if (chk_clock_skew())
	{
		writes("* BYE Clock skew detected. Check the clock on the file server\r\n");
		writeflush();
		exit(0);
	}

	{
		struct maildirwatch *w;

		if ((w=maildirwatch_alloc(".")) == NULL)
			writes("* OK [ALERT] Filesystem notification initialization error -- contact your mail administrator (check for configuration errors with the FAM/Gamin library)\r\n");
		else
			maildirwatch_free(w);
	}

	if ((tag=getenv("IMAPLOGINTAG")) != 0)
	{
		writes(tag);
		writes(" OK LOGIN Ok.\r\n");
	}
	else
		writes("* PREAUTH Ready.\r\n");
	writeflush();
	chkdisabled(ip, port);
	imapscan_init(&current_maildir_info);
	mainloop();
	fetch_free_cached();
	bye();
	return (0);
}
