/*
** Copyright 1998 - 2010 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
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
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif

#include	"maildirquota.h"
#include	"maildirmisc.h"
#include	"maildircreate.h"
#include	"quotawarnmsg.h"
#include	"../rfc822/rfc822.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<errno.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<time.h>
#include	<ctype.h>
#include	<numlib/numlib.h>


static void parsequotastr(const char *, struct maildirquota *);

#define DIG(c) ( (c) >= '0' && (c) <= '9')

/* Read the maildirsize file */

static int maildir_openquotafile_init(struct maildirsize *info,
				      const char *maildir,
				      const char *newquota);

static int do_maildir_openquotafile(struct maildirsize *info,
				    const char *filename,
				    const char *newquota);

int maildir_openquotafile(struct maildirsize *info, const char *maildir)
{
	return (maildir_openquotafile_init(info, maildir, NULL));
}

static int maildir_openquotafile_init(struct maildirsize *info,
				      const char *maildir,
				      const char *newquota)
{
	int rc;

	char	*buf=(char *)malloc(strlen(maildir)+sizeof("/maildirfolder"));

	memset(info, 0, sizeof(*info));

	info->fd= -1;

	if (!buf)
		return (-1);

	strcat(strcpy(buf, maildir), "/maildirfolder");
	if (stat(buf, &info->statbuf) == 0)	/* Go to parent */
	{
		strcat(strcpy(buf, maildir), "/..");

		rc=maildir_openquotafile_init(info, buf, newquota);
		free(buf);
		return rc;
	}

	info->maildir=strdup(maildir);

	if (!info->maildir)
	{
		free(buf);
		return (-1);
	}

	strcat(strcpy(info->maildirsizefile=buf, maildir), "/maildirsize");

	rc=do_maildir_openquotafile(info, buf, newquota);

	if (rc == 0)
		return (0);

	free(buf);
	free(info->maildir);
	return (rc);
}

static int do_maildir_openquotafile(struct maildirsize *info,
				    const char *filename,
				    const char *newquota)
{
	char buf[5120];
	char *p;
	unsigned l;
	int n;
	int first;

	/*
	** When setting a new quota, we don't care about the existing
	** maildirsize.
	*/

	if ((info->fd=(newquota ? open("/dev/null", O_RDWR):
		       maildir_safeopen(filename,
					O_RDWR|O_APPEND, 0))) < 0)
		return (0);	/* No quota */

	if (newquota)
	{
		parsequotastr(newquota, &info->quota);

		if (info->quota.nbytes == 0 &&
		    info->quota.nmessages == 0)
		{
			close(info->fd);
			info->fd= -1;
			errno=EINVAL;
			return (-1);
		}
		info->recalculation_needed=1;
		return (0);
	}

	p=buf;
	l=sizeof(buf);

	while (l)
	{
		n=read(info->fd, p, l);
		if (n < 0)
		{
			close(info->fd);
			info->fd= -1;
			return (-1);
		}
		if (n == 0)	break;
		p += n;
		l -= n;
	}

	if (fstat(info->fd, &info->statbuf))	/* maildir too big */
	{
		close(info->fd);
		info->fd= -1;
		return (-1);
	}

	if (l == 0)	/*
			** maildirsize overflowed, still need to read its
			** quota
			*/
	{
		p[-1]=0;
		p=strchr(buf, '\n');
		if (p)
			*p=0;
		parsequotastr(buf, &info->quota);
		info->recalculation_needed=1;
		return (0);
	}


	info->size.nbytes=0;
	info->size.nmessages=0;
	info->nlines=0;
	*p=0;
	p=buf;
	first=1;
	while (*p)
	{
		int64_t n=0;
		int c=0;
		char	*q=p;
		int	neg;

		while (*p)
			if (*p++ == '\n')
			{
				p[-1]=0;
				break;
			}

		if (first)
		{
			parsequotastr(q, &info->quota);
			first=0;
			continue;
		}

		while (*q && isspace((int)(unsigned char)*q))
			++q;

		neg=0;
		if (*q == '-')
		{
			neg=1;
			++q;
		}

		if (DIG(*q))
		{
			while (DIG(*q))
			{
				n=n*10 + (*q++ - '0');
			}

			if (neg)
				n= -n;

			neg=0;
			while (*q && isspace((int)(unsigned char)*q))
				++q;

			if (*q == '-')
			{
				neg=1;
				++q;
			}
			if (DIG(*q))
			{
				while (DIG(*q))
				{
					c=c*10 + (*q++ - '0');
				}

				if (neg)
					c= -c;

				info->size.nbytes += n;
				info->size.nmessages += c;
			}
		}

		++ info->nlines;
	}
	if (info->size.nbytes < 0 ||
	    info->size.nmessages < 0)
		info->recalculation_needed=1;
	return (0);
}

static void parsequotastr(const char *quota, struct maildirquota *q)
{
	int64_t i;
	const char *quota_start = quota;

	q->nbytes=0;
	q->nmessages=0;

	while (quota && *quota)
	{
		if (!DIG(*quota))
		{
			++quota;
			continue;
		}
		i=0;
		while (DIG(*quota))
			i=i*10 + (*quota++ - '0');
		switch (*quota)	{
		case MDQUOTA_SIZE:
			q->nbytes=i;
			break;
		case MDQUOTA_COUNT:
			q->nmessages=i;
			break;
		default:
			fprintf(stderr, "WARN: quota string '%s' not parseable\n",
				quota_start);
			break;
		}
	}
}


void maildir_closequotafile(struct maildirsize *info)
{
	if (info->maildir)
		free (info->maildir);
	info->maildir=NULL;

	if (info->maildirsizefile)
		free (info->maildirsizefile);
	info->maildirsizefile=NULL;

	if (info->fd >= 0)
		close(info->fd);
	info->fd= -1;
}

/**
 ** Check if size > quota, and calculate by how much
 */

static int checkOneQuota(int64_t size, int64_t quota, int *percentage);

static int checkQuota(struct maildirquota *size,
		      struct maildirquota *quota, int *percentage)
{
	int b_quota;
	int n_quota;

	if (checkOneQuota(size->nbytes, quota->nbytes, &b_quota) ||
	    checkOneQuota(size->nmessages, quota->nmessages,
			  &n_quota))
	{
		if (percentage)
			*percentage= 100;
		errno=ENOSPC;
		return -1;
	}

	if (b_quota < n_quota)
		b_quota=n_quota;

	if (percentage)
		*percentage=b_quota;
	return (0);
}

static int checkOneQuota(int64_t size, int64_t quota, int *percentage)
{
	int x=1;

	if (quota == 0) /* No quota */
	{
		*percentage=0;
		return (0);
	}

	if (size > quota)
	{
		*percentage=100;
		return (-1);
	}

	if (quota > 20000000)
		x=1024;

	*percentage= quota > 0 ? (size/x) * 100 / (quota/x):0;
	return 0;
}

static char *makenewmaildirsizename(const char *, int *);
static int countcurnew(const char *, time_t *, int64_t *, unsigned *);
static int countsubdir(const char *, const char *,
		time_t *, int64_t *, unsigned *);
static int statcurnew(const char *, time_t *);
static int statsubdir(const char *, const char *, time_t *);

static int	doaddquota(struct maildirsize *, int, int64_t, int, int);

static int docheckquota(struct maildirsize *info,
			int64_t xtra_size,
			int xtra_cnt, int *percentage);

int maildir_checkquota(struct maildirsize *info,
		       int64_t xtra_size,
		       int xtra_cnt)
{
	int	dummy;

	return (docheckquota(info, xtra_size, xtra_cnt, &dummy));
}

int maildir_readquota(struct maildirsize *info)
{
	int	percentage=0;

	(void)docheckquota(info, 0, 0, &percentage);
	return (percentage);
}

static int docheckquota(struct maildirsize *info,
			int64_t xtra_size,
			int xtra_cnt,
			int *percentage)
{
	char	*newmaildirsizename;
	int	maildirsize_fd;
	int64_t	maildirsize_size;
	unsigned maildirsize_cnt;

	time_t	tm;
	time_t	maxtime;
	DIR	*dirp;
	struct dirent *de;

	struct maildirquota new_quota;

	*percentage=0;

	if (info->fd < 0)	/* No quota */
		return (0);

	new_quota=info->size;

	new_quota.nbytes += xtra_size;
	new_quota.nmessages += xtra_cnt;

	if (!info->recalculation_needed &&
	    checkQuota(&new_quota, &info->quota, percentage) == 0)
		return (0);	/* New size is under quota */

	/*
	** Overquota, see if it's time to recalculate the quota anyway
	*/

	time(&tm);
	if (!info->recalculation_needed &&
	    info->nlines == 1 && tm < info->statbuf.st_mtime + 15*60)
		return (-1);


	maxtime=0;
	maildirsize_size=0;
	maildirsize_cnt=0;

	if (countcurnew(info->maildir,
			&maxtime, &maildirsize_size, &maildirsize_cnt))
	{
		errno=EIO;
		return (-1);
	}

	dirp=opendir(info->maildir);
	while (dirp && (de=readdir(dirp)) != 0)
	{
#ifdef DT_LNK
		if (de->d_type == DT_LNK) continue;
#endif
		if (countsubdir(info->maildir, de->d_name,
				&maxtime, &maildirsize_size,
			&maildirsize_cnt))
		{
			errno=EIO;
			closedir(dirp);
			return (-1);
		}
	}
	if (dirp)
	{
#if	CLOSEDIR_VOID
		closedir(dirp);
#else
		if (closedir(dirp))
		{
			errno=EIO;
			return (-1);
		}
#endif
	}

	newmaildirsizename=makenewmaildirsizename(info->maildir,
						  &maildirsize_fd);
	if (!newmaildirsizename)
	{
		errno=EIO;
		return (-1);
	}

	if (doaddquota(info, maildirsize_fd, maildirsize_size,
		maildirsize_cnt, 1))
	{
		unlink(newmaildirsizename);
		free(newmaildirsizename);
		close(maildirsize_fd);
		errno=EIO;
		return (-1);
	}

	if (rename(newmaildirsizename, info->maildirsizefile))
	{
		unlink(newmaildirsizename);
		close(maildirsize_fd);
		errno=EIO;
		return (-1);
	}

	info->recalculation_needed=0;
	info->size.nbytes=maildirsize_size;
	info->size.nmessages=maildirsize_cnt;
	info->nlines=1;
	close(info->fd);
	info->fd=maildirsize_fd;

	tm=0;

	if (statcurnew(info->maildir, &tm))
	{
		errno=EIO;
		return (-1);
	}

	dirp=opendir(info->maildir);
	while (dirp && (de=readdir(dirp)) != 0)
	{
#ifdef DT_LNK
		if (de->d_type == DT_LNK) continue;
#endif
		if (statsubdir(info->maildir, de->d_name, &tm))
		{
			errno=EIO;
			closedir(dirp);
			return (-1);
		}
	}
	if (dirp)
	{
#if	CLOSEDIR_VOID
		closedir(dirp);
#else
		if (closedir(dirp))
		{
			errno=EIO;
			return (-1);
		}
#endif
	}

	if (tm != maxtime)	/* Race condition, someone changed something */
	{
		info->recalculation_needed=1;
		info->nlines=0;
		errno=EAGAIN;
		return (-1);
	}

	*percentage=0;

	new_quota=info->size;

	new_quota.nbytes += xtra_size;
	new_quota.nmessages += xtra_cnt;

	return checkQuota(&new_quota, &info->quota, percentage);
}

int	maildir_addquota(struct maildirsize *info,
			 int64_t maildirsize_size, int maildirsize_cnt)
{
	if (info->fd < 0)
		return (0);

	return (doaddquota(info, info->fd, maildirsize_size,
			   maildirsize_cnt, 0));
}

static int doaddquota(struct maildirsize *info, int maildirsize_fd,
		      int64_t maildirsize_size, int maildirsize_cnt,
		      int isnew)
{
	char	n[NUMBUFSIZE];
	char	buf[NUMBUFSIZE * 4 + 32 ];
	char *p;
	int cnt;

	buf[0]=0;

	if (isnew)
	{
		if (info->quota.nbytes > 0)
		{
			char b[2];

			b[0]=MDQUOTA_SIZE;
			b[1]=0;

			strcat(strcat(buf, libmail_str_int64_t(info->quota.nbytes, n)),
			       b);
		}

		if (info->quota.nmessages > 0)
		{
			char b[2];

			b[0]=MDQUOTA_COUNT;
			b[1]=0;

			if (buf[0] != 0)
				strcat(buf, ",");

			strcat(strcat(buf,
				      libmail_str_size_t(info->quota.nmessages, n)),
			       b);
		}
		strcat(buf, "\n");
	}

	sprintf(buf + strlen(buf),
		"%12s ", libmail_str_int64_t(maildirsize_size, n));

	sprintf(buf + strlen(buf),
		"%12s\n", libmail_str_int64_t(maildirsize_cnt, n));

	p=buf;
	cnt=strlen(buf);

	while (cnt > 0)
	{
		int c=write( maildirsize_fd, p, cnt);

		if (c < 0)
			return (-1);

		cnt -= c;
		p += c;
	}

	return (0);
}


/* New maildirsize is built in the tmp subdirectory */

static char *makenewmaildirsizename(const char *dir, int *fd)
{
char	hostname[256];
struct	stat stat_buf;
time_t	t;
char	*p;

	hostname[0]=0;
	hostname[sizeof(hostname)-1]=0;
	gethostname(hostname, sizeof(hostname)-1);
	p=(char *)malloc(strlen(dir)+strlen(hostname)+130);
	if (!p)	return (0);

	for (;;)
	{
	char	tbuf[NUMBUFSIZE];
	char	pbuf[NUMBUFSIZE];

		time(&t);
		strcat(strcpy(p, dir), "/tmp/");
		sprintf(p+strlen(p), "%s.%s_NeWmAiLdIrSiZe.%s",
			libmail_str_time_t(t, tbuf),
			libmail_str_pid_t(getpid(), pbuf), hostname);

		if (stat( (const char *)p, &stat_buf) < 0 &&
			(*fd=maildir_safeopen(p,
				O_CREAT|O_RDWR|O_APPEND, 0644)) >= 0)
			break;
		sleep(3);
	}
	return (p);
}

static int statcurnew(const char *dir, time_t *maxtimestamp)
{
char	*p=(char *)malloc(strlen(dir)+5);
struct	stat	stat_buf;

	if (!p)	return (-1);
	strcat(strcpy(p, dir), "/cur");
	if ( stat(p, &stat_buf) == 0 && stat_buf.st_mtime > *maxtimestamp)
		*maxtimestamp=stat_buf.st_mtime;
	strcat(strcpy(p, dir), "/new");
	if ( stat(p, &stat_buf) == 0 && stat_buf.st_mtime > *maxtimestamp)
		*maxtimestamp=stat_buf.st_mtime;
	free(p);
	return (0);
}

static int statsubdir(const char *dir, const char *subdir, time_t *maxtime)
{
char	*p;
int	n;

	if ( *subdir != '.' || strcmp(subdir, ".") == 0 ||
		strcmp(subdir, "..") == 0
#ifndef TRASHQUOTA
				|| strcmp(subdir, "." TRASH) == 0
#endif
		)
		return (0);

	p=(char *)malloc(strlen(dir)+strlen(subdir)+2);
	if (!p)	return (-1);
	strcat(strcat(strcpy(p, dir), "/"), subdir);
	n=statcurnew(p, maxtime);
	free(p);
	return (n);
}

static int docount(const char *, time_t *, int64_t *, unsigned *);

static int countcurnew(const char *dir, time_t *maxtime,
	int64_t *sizep, unsigned *cntp)
{
char	*p=(char *)malloc(strlen(dir)+5);
int	n;

	if (!p)	return (-1);
	strcat(strcpy(p, dir), "/new");
	n=docount(p, maxtime, sizep, cntp);
	if (n == 0)
	{
		strcat(strcpy(p, dir), "/cur");
		n=docount(p, maxtime, sizep, cntp);
	}
	free(p);
	return (n);
}

static int countsubdir(const char *dir, const char *subdir, time_t *maxtime,
	int64_t *sizep, unsigned *cntp)
{
char	*p;
int	n;

	if ( *subdir != '.' || strcmp(subdir, ".") == 0 ||
	     strcmp(subdir, "..") == 0 ||
	     ! maildirquota_countfolder(subdir))
		return (0);

	p=(char *)malloc(strlen(dir)+strlen(subdir)+2);
	if (!p)	return (2);
	strcat(strcat(strcpy(p, dir), "/"), subdir);
	n=countcurnew(p, maxtime, sizep, cntp);
	free(p);
	return (n);
}

static int docount(const char *dir, time_t *dirstamp,
	int64_t *sizep, unsigned *cntp)
{
struct	stat	stat_buf;
char	*p;
DIR	*dirp;
struct dirent *de;
unsigned long s;

	if (stat(dir, &stat_buf))	return (0);	/* Ignore */
	if (stat_buf.st_mtime > *dirstamp)	*dirstamp=stat_buf.st_mtime;
	if ((dirp=opendir(dir)) == 0)	return (0);
	while ((de=readdir(dirp)) != 0)
	{
	const char *n=de->d_name;

		if (*n == '.')	continue;

		if (!maildirquota_countfile(n))
			continue;

		if (maildir_parsequota(n, &s) == 0)
			stat_buf.st_size=s;
		else
		{
			p=(char *)malloc(strlen(dir)+strlen(n)+2);
			if (!p)
			{
				closedir(dirp);
				return (-1);
			}
			strcat(strcat(strcpy(p, dir), "/"), n);
			if (stat(p, &stat_buf))
			{
				free(p);
				continue;
			}
			free(p);
		}
		*sizep += stat_buf.st_size;
		++*cntp;
	}

#if	CLOSEDIR_VOID
	closedir(dirp);
#else
	if (closedir(dirp))
		return (-1);
#endif
	return (0);
}

int maildirquota_countfolder(const char *folder)
{
#ifdef TRASHQUOTA

#else

	if (strcmp(folder, "." TRASH) == 0 ||
	    strcmp(folder, "." TRASH "/") == 0)
		return (0);

	for ( ; *folder; folder++)
		if (*folder == '/' &&
		    (strcmp(folder+1, "." TRASH) == 0 ||
		     strcmp(folder+1, "." TRASH "/") == 0))
			return (0);
#endif
	return (1);
}

int maildirquota_countfile(const char *n)
{
#ifdef TRASHQUOTA

#else
	const char *nn=strrchr(n, '/');

	if (nn != NULL)
		n=nn+1;

	/* do not count msgs marked as deleted */

	for ( ; *n; n++)
	{
		if (n[0] != MDIRSEP[0] || n[1] != '2' ||
		    n[2] != ',')	continue;
		n += 3;
		while (*n >= 'A' && *n <= 'Z')
		{
			if (*n == 'T')	return (0);
			++n;
		}
		break;
	}
#endif
	return (1);
}

/*
** Prepare to add something to the maildir
*/

int maildir_quota_add_start(const char *maildir,
			    struct maildirsize *info,
			    int64_t msgsize, int nmsgs,
			    const char *newquota)
{
	struct maildirquota mq;
	int i;

	if ( maildir_openquotafile(info, maildir))
		info->fd= -1;

	if (newquota != NULL)
	{
		parsequotastr(newquota, &mq);

		if ((mq.nbytes > 0 || mq.nmessages > 0) &&
		    (info->fd < 0 || info->quota.nbytes != mq.nbytes ||
		     info->quota.nmessages != mq.nmessages))
		{
			if (info->fd < 0)
			{
				maildir_quota_set(maildir, newquota);
				if (maildir_openquotafile(info, maildir))
					info->fd= -1;
			}
			else
			{
				info->quota=mq;
				info->recalculation_needed=1;
			}
		}
	}
	if (info->fd < 0)
		return (0);	/* No quota set on this maildir */

	for (i=0; i<5; i++)
	{
		int rc;

		rc=maildir_checkquota(info, msgsize, nmsgs);
		if (rc == 0)
			return (0);

		if (errno != EAGAIN)
		{
			maildir_closequotafile(info);
			return (-1);
		}
	}
	maildir_closequotafile(info);

	/* Cannot recover from a race condition, just punt */

	return (0);
}

void maildir_quota_add_end(struct maildirsize *info,
			   int64_t msgsize, int nmsgs)
{
	maildir_addquota(info, msgsize, nmsgs);
	maildir_closequotafile(info);
}

void maildir_quota_deleted(const char *maildir,
			   int64_t nbytes,	/* Must be negative */
			   int nmsgs)	/* Must be negative */
{
	struct maildirsize info;

	if ( maildir_openquotafile(&info, maildir))
		return;

	maildir_checkquota(&info, nbytes, nmsgs); /* Cleanup */
	maildir_addquota(&info, nbytes, nmsgs);
	maildir_closequotafile(&info);
}

void maildir_quota_recalculate(const char *maildir)
{
	struct maildirsize info;

	if (maildir_openquotafile(&info, maildir))
		return;
	info.recalculation_needed=1;

	maildir_readquota(&info);
	maildir_closequotafile(&info);
}

int maildir_quota_delundel_start(const char *maildir,
				 struct maildirsize *info,
				 int64_t msgsize, int nmsgs)
{
#if TRASHQUOTA
	return (0);
#else
	if (nmsgs < 0)
	{
		maildir_quota_deleted(maildir, msgsize, nmsgs);
		return (0);	/* Always allowed */
	}

	return maildir_quota_add_start(maildir, info, msgsize, nmsgs, NULL);
#endif
}

void maildir_quota_delundel_end(struct maildirsize *info,
				int64_t msgsize, int nmsgs)
{
#if TRASHQUOTA
	return;
#else
	if (nmsgs < 0)
		return;

	maildir_quota_add_end(info, msgsize, nmsgs);
#endif
}


void maildir_quota_set(const char *dir, const char *quota)
{
	struct maildirsize info;

	if (maildir_openquotafile_init(&info, dir, quota) == 0)
	{
		maildir_checkquota(&info, 0, 0);
		maildir_closequotafile(&info);
	}
}


static void do_deliver_warning(const char *msgfile, const char *dir)
{
	int	fdin, fd, ret;
	FILE *fpout;
	time_t	t;
	size_t	l, msg_len;
	char	*qname = 0;
	struct stat	sb;
	char	hostname[256];
	char	buf[4096];
	size_t	n;
	struct	maildirsize info;
	struct maildir_tmpcreate_info createInfo;

	fdin=-1;

	if (msgfile && *msgfile)
		fdin=open(msgfile, O_RDONLY);

	if (fdin < 0)
	{
		msgfile=QUOTAWARNMSG;
		fdin=open(msgfile, O_RDONLY);
	}

	if (fdin < 0)
		return;

	l = strlen(dir)+sizeof("/quotawarn");

	/* Send only one warning every 24 hours */
	if ((qname = malloc(l)) == 0)
	{
		close(fdin);
		return;
	}

	strcat(strcpy(qname, dir), "/quotawarn");
	time(&t);
	ret = stat(qname, &sb);
	if ((ret == -1 && errno != ENOENT) ||
	    (ret == 0 && (sb.st_mtime + 86400) > t))
	{
		free(qname);
		close(fdin);
		return;
	}

	fd = open(qname, O_WRONLY|O_CREAT|O_TRUNC, 0644);
	free(qname);
	if (fd == -1)
	{
		close(fdin);
		return;
	}
	if (write(fd, buf, 1) < 0)
		perror(msgfile);

	close(fd);

	strcpy(buf, "Date: ");
	rfc822_mkdate_buf(t, buf+strlen(buf));
	strcat(buf, "\n");

	hostname[0]=0;
	hostname[sizeof(hostname)-1]=0;
	gethostname(hostname, sizeof(hostname)-1);
	sprintf(buf+strlen(buf), "Message-Id: <%lu.overquota@%-1.256s>\n",
		(unsigned long)t, hostname);

	if (fstat(fdin, &sb) < 0) {
		close(fdin);
		return;
	}
	msg_len=strlen(buf)+sb.st_size;



	maildir_tmpcreate_init(&createInfo);
	createInfo.maildir=dir;
	createInfo.uniq="warn";
	createInfo.msgsize=msg_len;
	createInfo.doordie=1;

	if ((fpout=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		close(fdin);
		return;
	}

	fprintf(fpout, "%s", buf);

	while ((n=read(fdin, buf, sizeof(buf))) > 0)
	{
		if (fwrite(buf, n, 1, fpout) != 1)
		{
			perror(createInfo.tmpname);
			break;
		}
	}
	close(fdin);

	if (fflush(fpout) || ferror(fpout))
	{
		fclose(fpout);
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return;
	}

	if (fseek(fpout, 0L, SEEK_SET) >= 0)
	{
		/* Make sure the quota message's size itself is factored into
		** the quota. Deliver the message regardless of whether the
		** user is over quota.
		*/
		if (maildirquota_countfolder(dir))
		{
			maildir_quota_add_start(dir, &info, msg_len, 1, NULL);
			maildir_quota_add_end(&info, msg_len, 1);
		}
	}

	fclose(fpout);
	if (maildir_movetmpnew(createInfo.tmpname,
			       createInfo.newname))
	{
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return;
	}
	maildir_tmpcreate_free(&createInfo);
}

void maildir_deliver_quota_warning(const char *dir, const int percent,
				   const char *msgquotafile)
{
	size_t l;
	char *p;
	struct stat	sb;

	/* If we delivered to a folder, dump the warning message into INBOX */

	l = strlen(dir)+sizeof("/maildirfolder");
	if ((p = malloc(l)) == 0)
		return;

	strcat(strcpy(p, dir), "/maildirfolder");

	/* If delivering to a folder, find quotawarn in its parent directory */

	if (stat(p, &sb) == 0)
	{
		strcat(strcpy(p, dir), "/..");
		maildir_deliver_quota_warning(p, percent, msgquotafile);
		free(p);
		return;
	}
	free(p);

	if (percent >= 0)
	{
		struct maildirsize info;

		if (maildir_openquotafile(&info, dir) == 0)
		{
			if (maildir_readquota(&info) >= percent)
			{
				maildir_closequotafile(&info);
				do_deliver_warning(msgquotafile, dir);
				return;
			}
			maildir_closequotafile(&info);
		}
	}
}
