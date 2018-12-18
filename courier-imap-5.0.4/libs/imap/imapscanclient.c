/*
** Copyright 1998 - 2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<fcntl.h>
#include	<time.h>
#include	<errno.h>
#include	"numlib/numlib.h"

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<sys/types.h>
#include	<sys/stat.h>
#if HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#ifndef WEXITSTATUS
#define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
#define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
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

#include	"liblock/config.h"
#include	"liblock/liblock.h"
#include	"maildir/config.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirwatch.h"
#include	"liblock/mail.h"

#include	"imapscanclient.h"
#include	"imaptoken.h"
#include	"imapwrite.h"
#include	"imapd.h"

static const char rcsid[]="$Id: imapscanclient.c,v 1.47 2009/06/27 16:32:38 mrsam Exp $";

/*
** RFC 2060: "A good value to use for the unique identifier validity value is a
** 32-bit representation of the creation date/time of the mailbox."
**
** Well, Y2038k is on the horizon, time to push to reset button.
**
*/

#ifndef IMAP_EPOCH
#define IMAP_EPOCH	1000000000
#endif

static int do_imapscan_maildir2(struct imapscaninfo *, const char *,
				int, int, struct uidplus_info *);
void imapscanfail(const char *p);

#if SMAP
extern int smapflag;
#endif

extern int keywords();
extern void set_time(const char *tmpname, time_t timestamp);

static void imapscan_readKeywords(const char *maildir,
				  struct imapscaninfo *scaninfo);

void imapscan_init(struct imapscaninfo *i)
{
	memset(i, 0, sizeof(*i));

	if ((i->keywordList=malloc(sizeof(*i->keywordList))) == NULL)
		write_error_exit(0);

	libmail_kwhInit(i->keywordList);
}

void imapscan_copy(struct imapscaninfo *a,
		   struct imapscaninfo *b)
{
	imapscan_free(a);
	*a=*b;
	imapscan_init(b);
}

struct libmail_kwMessage *imapscan_createKeyword(struct imapscaninfo *a,
					      unsigned long n)
{
	if (n >= a->nmessages)
		return NULL;

	if (a->msgs[n].keywordMsg == NULL)
	{
		struct libmail_kwMessage *m=libmail_kwmCreate();

		if (!m)
			write_error_exit(0);

		m->u.userNum=n;
		a->msgs[n].keywordMsg=m;
	}

	return a->msgs[n].keywordMsg;
}

static int uselocks()
{
	const	char *p;

	if ((p=getenv("IMAP_USELOCKS")) != 0 && *p != '1')
		return 0;

	return 1;
}

int imapmaildirlock(struct imapscaninfo *scaninfo,
		    const char *maildir,
		    int (*func)(void *),
		    void *void_arg)
{
	char *newname;
	int tryAnyway;
	int rc;

	if (!uselocks())
		return (*func)(void_arg);

	if (scaninfo->watcher == NULL &&
	    (scaninfo->watcher=maildirwatch_alloc(maildir)) == NULL)
		imapscanfail("maildirwatch");

	if ((newname=maildir_lock(maildir, scaninfo->watcher, &tryAnyway))
	    == NULL)
	{
		if (!tryAnyway)
			return -1;

		imapscanfail("maildir_lock");
	}

	rc=(*func)(void_arg);

	if (newname)
	{
		unlink(newname);
		free(newname);
		newname=NULL;
	}
	return rc;
}


struct imapscan_info {
	struct imapscaninfo *scaninfo;
	const char *dir;
	int leavenew;
	int ro;
	struct uidplus_info *uidplus;
};


static int imapscan_maildir_cb(void *);

int imapscan_maildir(struct imapscaninfo *scaninfo,
		     const char *dir, int leavenew, int ro,
		     struct uidplus_info *uidplus)
{
	struct imapscan_info ii;

	ii.scaninfo=scaninfo;
	ii.dir=dir;
	ii.leavenew=leavenew;
	ii.ro=ro;
	ii.uidplus=uidplus;

	return imapmaildirlock(scaninfo, dir, imapscan_maildir_cb, &ii);
}

int imapscan_maildir_cb(void *void_arg)
{
	struct imapscan_info *ii=(struct imapscan_info *)void_arg;
	int rc=do_imapscan_maildir2(ii->scaninfo,
				   ii->dir,
				   ii->leavenew,
				   ii->ro,
				   ii->uidplus);

	if (rc)
		rc= -1;
	return rc;
}

/* This structure is a temporary home for the filenames */

struct tempinfo {
	struct tempinfo *next;
	char *filename;
	unsigned long uid;
	int	found;
	int	isrecent;
	} ;

static char *imapscan_namedir(const char *dir, const char *new_or_cur)
{
char	*p=malloc(strlen(dir)+strlen(new_or_cur)+2);

	if (!p)	write_error_exit(0);
	strcat(strcat(strcpy(p, dir), "/"), new_or_cur);
	return (p);
}

static int fnamcmp(const char *a, const char *b)
{
	long ai, bi;
	char ca, cb;

	ai = atol(a);
	bi = atol(b);
	if(ai - bi)
		return ai - bi;

	do
	{
		ca= *a++;
		cb= *b++;

		if (ca == ':') ca=0;
		if (cb == ':') cb=0;
	} while (ca && cb && ca == cb);


	return ( (int)(unsigned char)ca - (int)(unsigned char)cb);
}

static int sort_by_filename(struct tempinfo **a, struct tempinfo **b)
{
	return (fnamcmp( (*a)->filename, (*b)->filename));
}

static int sort_by_filename_status(struct tempinfo **a, struct tempinfo **b)
{
	if ( (*a)->found && (*b)->found )
	{
		if ( (*a)->uid < (*b)->uid )
			return (-1);
		if ( (*a)->uid > (*b)->uid )
			return (1);
		return (0);	/* What the fuck??? */
	}
	if ( (*a)->found )	return (-1);
	if ( (*b)->found )	return (1);

	return (fnamcmp( (*a)->filename, (*b)->filename));
}

/* Binary search on an array of tempinfos which is sorted by filenames */

static int search_by_filename(struct tempinfo **a, unsigned s, unsigned *i,
	const char *filename)
{
unsigned lo=0, hi=s, mid;
int	rc;

	while (lo < hi)
	{
		mid=(hi+lo)/2;
		rc=fnamcmp( a[mid]->filename, filename);
		if (rc < 0)
		{
			lo=mid+1;
			continue;
		}
		if (rc > 0)
		{
			hi=mid;
			continue;
		}
		*i=mid;
		return (0);
	}
	return (-1);
}

void imapscanfail(const char *p)
{
	fprintf(stderr, "ERR: Failed to create cache file: %s (%s)\n", p,
		getenv("AUTHENTICATED"));
#if	HAVE_STRERROR
	fprintf(stderr, "ERR: Error: %s\n", strerror(errno));
#endif

#if	HAVE_FAM
	if (errno == EIO)
	{
		fprintf(stderr,
			"ERR: Check for proper operation and configuration\n"
			"ERR: of the File Access Monitor daemon (famd).\n");
	}
#endif
}

static char *readbuf;
static unsigned readbufsize=0;

char *readline(unsigned i, FILE *fp)
{
int	c;

	for (;;)
	{
		if (i >= 10000)
			--i;	/* DOS check */

		if (i >= readbufsize)
		{
		char	*p= readbuf ? realloc(readbuf, readbufsize+256):
					malloc(readbufsize+256);

			if (!p)	write_error_exit(0);
			readbuf=p;
			readbufsize += 256;
		}

		c=getc(fp);
		if (c == EOF || c == '\n')
		{
			readbuf[i]=0;
			return (c == EOF ? 0:readbuf);
		}
		readbuf[i++]=c;
	}
}

static int do_imapscan_maildir2(struct imapscaninfo *scaninfo,
				const char *dir, int leavenew, int ro,
				struct uidplus_info *uidplus)
{
char	*dbfilepath, *newdbfilepath;
struct	tempinfo *tempinfo_list=0, **tempinfo_array=0, *tempinfoptr;
struct	tempinfo *newtempinfo_list=0;
unsigned	tempinfo_cnt=0, i;
FILE	*fp;
char	*p, *q;
unsigned long uidv, nextuid;
int	version;
struct	stat	stat_buf;
DIR	*dirp;
struct	dirent *de;
unsigned long left_unseen=0;
int	dowritecache=0;

	if (is_sharedsubdir(dir))
		maildir_shared_sync(dir);


	/* Step 0 - purge the tmp directory */

	maildir_purgetmp(dir);

	dbfilepath=malloc(strlen(dir)+sizeof("/" IMAPDB));
	if (!dbfilepath)	write_error_exit(0);
	strcat(strcpy(dbfilepath, dir), "/" IMAPDB);

	/*
	** We may need to rebuild the UID cache file.  Create the new cache
	** file in the tmp subdirectory.
	*/

	{
		char	uniqbuf[80];
		static  unsigned tmpuniqcnt=0;
		struct maildir_tmpcreate_info createInfo;
		int fd;

		maildir_tmpcreate_init(&createInfo);

		createInfo.maildir=dir;
		createInfo.hostname=getenv("HOSTNAME");
		sprintf(uniqbuf, "imapuid_%u", tmpuniqcnt++);
		createInfo.uniq=uniqbuf;
		createInfo.doordie=1;

		if ((fd=maildir_tmpcreate_fd(&createInfo)) < 0)
		{
			write_error_exit(0);
		}
		close(fd);

		newdbfilepath=createInfo.tmpname;
		createInfo.tmpname=NULL;
		maildir_tmpcreate_free(&createInfo);
	}

	/* Step 1 - read the cache file */

	if ((fp=fopen(dbfilepath, "r")) != 0 &&
		(p=readline(0, fp)) != 0 &&
		sscanf(p, "%d %lu %lu", &version, &uidv, &nextuid) == 3 &&
		version == IMAPDBVERSION)
	{
		while ((p=readline(0, fp)) != 0)
		{
		char	*q=strchr(p, ' ');
		unsigned long uid;
		struct	tempinfo	*newtmpl;

			if (!q)	continue;
			*q++=0;
			if (sscanf(p, "%lu", &uid) != 1)	continue;
			if ((newtmpl=(struct tempinfo *)
				malloc(sizeof(struct tempinfo))) == 0
				|| (newtmpl->filename=strdup(q)) == 0)
			{
				unlink(newdbfilepath);
				write_error_exit(0);
			}
			newtmpl->next=tempinfo_list;
			tempinfo_list=newtmpl;
			newtmpl->uid=uid;
			newtmpl->found=0;
			newtmpl->isrecent=0;
			++tempinfo_cnt;
		}
		fclose(fp);
		fp=0;
	}
	else if(!ro)
	{

	/* First time - create the cache file */

		if (fp)	fclose(fp);
		nextuid=1;
		if ((fp=fopen(newdbfilepath, "w")) == 0 ||
			fstat(fileno(fp), &stat_buf) != 0)
		{
			if (fp)	fclose(fp);
			imapscanfail(newdbfilepath);

			/* bk: ignore error */
			unlink(newdbfilepath);
			unlink(dbfilepath);
			fp = 0;
			/*
			free(dbfilepath);
			unlink(newdbfilepath);
			free(newdbfilepath);
			return (-1);
			*/
		}
		uidv=stat_buf.st_mtime - IMAP_EPOCH;
		dowritecache=1;
	}
	else
	{
		nextuid=1;
		uidv=time(0) - IMAP_EPOCH;
	}

	while (uidplus)
	{
		struct	tempinfo	*newtmpl;

		if (uidplus->tmpkeywords)
			if (rename(uidplus->tmpkeywords,
				   uidplus->newkeywords) < 0)
			{
				struct libmail_kwGeneric g;

				/*
				** Maybe courierimapkeywords needs to be
				** created.
				*/

				libmail_kwgInit(&g);
				libmail_kwgReadMaildir(&g, dir);
				libmail_kwgDestroy(&g);

				rename(uidplus->tmpkeywords,
				       uidplus->newkeywords);
			}

		maildir_movetmpnew(uidplus->tmpfilename,
				   uidplus->curfilename);

		if (uidplus->mtime)
			set_time (uidplus->curfilename, uidplus->mtime);

		if ((newtmpl=(struct tempinfo *)
		     malloc(sizeof(struct tempinfo))) == 0
		    || (newtmpl->filename=strdup(strrchr(uidplus->curfilename,
							 '/')+1)) == 0)
		{
			unlink(newdbfilepath);
			write_error_exit(0);
		}

		if ((p=strrchr(newtmpl->filename, MDIRSEP[0])) != 0)
			*p=0;

		newtmpl->next=tempinfo_list;
		tempinfo_list=newtmpl;
		newtmpl->uid=nextuid;
		uidplus->uid=nextuid;
		nextuid++;
		newtmpl->found=0;
		newtmpl->isrecent=0;
		++tempinfo_cnt;

		uidplus=uidplus->next;
		dowritecache=1;
	}

	if (!fp && (fp=fopen(newdbfilepath, "w")) == 0)
	{
		imapscanfail(newdbfilepath);

		/* bk: ignore error */
		unlink(newdbfilepath);
		unlink(dbfilepath);
	}

	/*
	** Convert the link list of cached files to an array, then
	** sort it by filename.
	*/

	if ((tempinfo_array=(struct tempinfo **)malloc(
		(tempinfo_cnt+1)*sizeof(struct tempinfo *))) == 0)
	{
		unlink(newdbfilepath);
		write_error_exit(0);
	}

	for (i=0, tempinfoptr=tempinfo_list; tempinfoptr;
		tempinfoptr=tempinfoptr->next, i++)
		tempinfo_array[i]=tempinfoptr;

	if (tempinfo_cnt)
		qsort(tempinfo_array, tempinfo_cnt,
			sizeof(tempinfo_array[0]),
			( int (*)(const void *, const void *))
				&sort_by_filename);

	/* Step 2 - read maildir/cur.  Search the array.  Mark found files. */

	p=imapscan_namedir(dir, "cur");
	dirp=opendir(p);
	free(p);
	while (dirp && (de=readdir(dirp)) != 0)
	{
	int	rc;
	struct	tempinfo	*newtmpl;

		if (de->d_name[0] == '.')	continue;

		p=my_strdup(de->d_name);

		/* IMAPDB doesn't store the filename flags, so strip them */
		q=strrchr(p, MDIRSEP[0]);
		if (q)	*q=0;
		rc=search_by_filename(tempinfo_array, tempinfo_cnt, &i, p);
		if (q)	*q=MDIRSEP[0];
		if (rc == 0)
		{
			tempinfo_array[i]->found=1;
			free(tempinfo_array[i]->filename);
			tempinfo_array[i]->filename=p;
				/* Keep the full filename */
			continue;
		}

		if ((newtmpl=(struct tempinfo *)
			malloc(sizeof(struct tempinfo))) == 0)
		{
			unlink(newdbfilepath);
			write_error_exit(0);
		}
		newtmpl->filename=p;
		newtmpl->next=newtempinfo_list;
		newtmpl->found=0;
		newtmpl->isrecent=1;
		/** begin change by mbhangui */
		for(;*q;q++);
		q--;
		if(*q == ',')
			newtmpl->isrecent=1;
		else
			newtmpl->isrecent=0;
		/** end change by mbhangui */
		newtempinfo_list=newtmpl;
		dowritecache=1;
	}
	if (dirp)	closedir(dirp);

	/* Step 3 - purge messages that no longer exist in the maildir */

	free(tempinfo_array);

	for (tempinfo_array= &tempinfo_list; *tempinfo_array; )
	{
		if ( (*tempinfo_array)->found )
		{
			tempinfo_array= & (*tempinfo_array)->next;
			continue;
		}
		tempinfoptr= *tempinfo_array;
		*tempinfo_array=tempinfoptr->next;
		free(tempinfoptr->filename);
		free(tempinfoptr);
		--tempinfo_cnt;
		dowritecache=1;
	}

	/* Step 4 - add messages in cur that are not in the cache file */

	while (newtempinfo_list)
	{
		tempinfoptr=newtempinfo_list;
		newtempinfo_list=tempinfoptr->next;

		tempinfoptr->next=tempinfo_list;
		tempinfo_list=tempinfoptr;
		++tempinfo_cnt;
	}

	/* Step 5 - read maildir/new.  */

	p=imapscan_namedir(dir, "new");

	if (leavenew)
	{
		dirp=opendir(p);
		while (dirp && (de=readdir(dirp)) != 0)
		{
			if (de->d_name[0] == '.')	continue;
			++left_unseen;
		}
		if (dirp)	closedir(dirp);
	}
	else
		/*
		** Some filesystems keel over if we delete files while
		** reading the directory where the files are.
		** Accomodate them by processing 20 files at a time.
		*/
	{
		char *new_buf[20];
		char *cur_buf[20];
		int keepgoing;
		int n;

		do
		{
			n=0;
			keepgoing=0;

			dirp=opendir(p);
			while (dirp && (de=readdir(dirp)) != 0)
			{
				struct	tempinfo	*newtmpl;
				char	*newname, *curname;
				char	*z;

				if (de->d_name[0] == '.')	continue;

				z=de->d_name;

				newname=imapscan_namedir(p, z);
				curname=malloc(strlen(newname)
					       +sizeof(MDIRSEP "2,"));
				if (!curname)
				{
					unlink(newdbfilepath);
					write_error_exit(0);
				}
				strcpy(curname, newname);
				z=strrchr(curname, '/');

				memcpy(z-3, "cur", 3);
				/* Mother of all hacks */
				if (strchr(z, MDIRSEP[0]) == 0)
					strcat(z, MDIRSEP "2,");

				new_buf[n]=newname;
				cur_buf[n]=curname;

				if ((newtmpl=(struct tempinfo *)
				     malloc(sizeof(struct tempinfo))) == 0)
				{
					unlink(newdbfilepath);
					write_error_exit(0);
				}
				newtmpl->filename=my_strdup(z+1);
				newtmpl->next=tempinfo_list;
				tempinfo_list=newtmpl;
				++tempinfo_cnt;
				newtmpl->found=0;
				newtmpl->isrecent=1;
				dowritecache=1;

				if (++n >= sizeof(cur_buf)/
				    sizeof(cur_buf[0]))
				{
					keepgoing=1;
					break;
				}
			}

			if (dirp)	closedir(dirp);

			while (n)
			{
				char *newname, *curname;

				--n;

				newname=new_buf[n];
				curname=cur_buf[n];

				if (rename(newname, curname))
				{
					fprintf(stderr,
						"ERR: rename(%s,%s) failed:"
						" %s\n",
						newname, curname,
						strerror(errno));
					keepgoing=0;
					/* otherwise we could have infinite loop */
				}

				free(newname);
				free(curname);
			}
		} while (keepgoing);
	}
	free(p);

	/*
	** Step 6: sort existing messages by UIDs, new messages will
	** sort after all messages with UIDs, and new messages are
	** sorted by filename, so that they end up roughly in the order
	** they were received.
	*/

	if ((tempinfo_array=(struct tempinfo **)malloc(
		(tempinfo_cnt+1)*sizeof(struct tempinfo *))) == 0)
	{
		unlink(newdbfilepath);
		write_error_exit(0);
	}

	for (i=0, tempinfoptr=tempinfo_list; tempinfoptr;
		tempinfoptr=tempinfoptr->next, i++)
		tempinfo_array[i]=tempinfoptr;

	if (tempinfo_cnt)
		qsort(tempinfo_array, tempinfo_cnt,
			sizeof(tempinfo_array[0]),
			( int (*)(const void *, const void *))
				&sort_by_filename_status);

	/* Assign new UIDs */

	for (i=0; i<tempinfo_cnt; i++)
		if ( !tempinfo_array[i]->found )
		{
			tempinfo_array[i]->uid= nextuid++;
			dowritecache=1;
		}

	/* bk: ignore if failed to open file */
	if (!ro && dowritecache && fp)
	{
		int need_fclose;
	/* Step 7 - write out the new cache file */

		version=IMAPDBVERSION;
		fprintf(fp, "%d %lu %lu\n", version, uidv, nextuid);

		for (i=0; i<tempinfo_cnt; i++)
		{
			q=strrchr(tempinfo_array[i]->filename, MDIRSEP[0]);
			if (q)  *q=0;
			fprintf(fp, "%lu %s\n", tempinfo_array[i]->uid,
				tempinfo_array[i]->filename);
			if (q)	*q=MDIRSEP[0];
		}
		need_fclose=1;
		if (fflush(fp) || ferror(fp) || ((need_fclose=0), fclose(fp)))
		{
			imapscanfail(dir);
			if (need_fclose)
				fclose(fp);
			/* bk: ignore if failed */
			unlink(newdbfilepath);
			unlink(dbfilepath);
		}
		/* bk */
		else
			rename(newdbfilepath, dbfilepath);
	}
	else
	{
		if (fp)
			fclose(fp);
		unlink(newdbfilepath);
	}
	free(dbfilepath);
	free(newdbfilepath);

	/* Step 8 - create the final scaninfo array */

	scaninfo->msgs=0;
	if (tempinfo_cnt && (scaninfo->msgs=(struct imapscanmessageinfo *)
		malloc(tempinfo_cnt * sizeof(*scaninfo->msgs))) == 0)
		write_error_exit(0);
	scaninfo->nmessages=tempinfo_cnt;
	scaninfo->uidv=uidv;
	scaninfo->left_unseen=left_unseen;
	scaninfo->nextuid=nextuid+left_unseen;

	for (i=0; i<tempinfo_cnt; i++)
	{
		scaninfo->msgs[i].uid=tempinfo_array[i]->uid;
		scaninfo->msgs[i].filename=tempinfo_array[i]->filename;
		scaninfo->msgs[i].keywordMsg=NULL;
		scaninfo->msgs[i].copiedflag=0;
		scaninfo->msgs[i].err8bitflag=0;

#if SMAP
		if (smapflag)
			scaninfo->msgs[i].recentflag=0;
		else
#endif
			scaninfo->msgs[i].recentflag=
				tempinfo_array[i]->isrecent;
		scaninfo->msgs[i].changedflags=0;

		free(tempinfo_array[i]);
	}
	free(tempinfo_array);

	imapscan_readKeywords(dir, scaninfo);


	return (0);
}

static int try_maildir_open(const char *dir, struct imapscanmessageinfo *n)
{
int	fd;
char	*filename=maildir_filename(dir, 0, n->filename);
char	*p;

	if (!filename)
	{
		return (0);
	}

	p=strrchr(filename, '/')+1;

	if (strcmp(p, n->filename))
	{
		n->changedflags=1;
		free(n->filename);
		n->filename=malloc(strlen(p)+1);
		if (!n->filename)	write_error_exit(0);
		strcpy(n->filename, p);
	}

	fd=maildir_semisafeopen(filename, O_RDONLY, 0);
	free(filename);
	return (fd);
}

int imapscan_openfile(const char *dir, struct imapscaninfo *i, unsigned j)
{
struct imapscanmessageinfo *n;

	if (j >= i->nmessages)
	{
		errno=EINVAL;
		return (-1);
	}

	n=i->msgs+j;

	return (try_maildir_open(dir, n));
}

void imapscan_free(struct imapscaninfo *i)
{
	unsigned	n;

	if (i->watcher)
	{
		maildirwatch_free(i->watcher);
		i->watcher=NULL;
	}

	if (i->msgs)
	{
		for (n=0; n<i->nmessages; n++)
		{
			if (i->msgs[n].filename)
				free(i->msgs[n].filename);

			if (i->msgs[n].keywordMsg)
				libmail_kwmDestroy(i->msgs[n].keywordMsg);

		}
		free(i->msgs);
		i->msgs=0;
	}

	if (i->keywordList)
	{
		if (libmail_kwhCheck(i->keywordList) < 0)
			write_error_exit("INTERNAL ERROR: Keyword hashtable "
					 "memory corruption.");

		free(i->keywordList);
		i->keywordList=NULL;
	}
}

/*
 * Keyword-related stuff  See README.imapkeywords.html for more information.
 */

extern char *current_mailbox;

int imapscan_updateKeywords(const char *filename,
			    struct libmail_kwMessage *newKeyword)
{
	char *tmpname, *newname;
	int rc;

	if (maildir_kwSave(current_mailbox, filename, newKeyword,
			   &tmpname, &newname, 0))
	{
		perror("maildir_kwSave");
		return -1;
	}

	rc=rename(tmpname, newname);

	if (rc)
	{
		perror(tmpname);
		unlink(tmpname);
	}
	free(tmpname);
	free(newname);
	return rc;
}

static unsigned long hashFilename(const char *fn, struct imapscaninfo *info)
{
	unsigned long hashBucket=0;

	while (*fn && *fn != MDIRSEP[0])
	{
		hashBucket=(hashBucket << 1) ^ (hashBucket & 0x8000 ? 0x1301:0)
			^ (unsigned char)*fn++;
	}
	hashBucket=hashBucket & 0xFFFF;

	return hashBucket % info->nmessages; /* Cannot get here if its zero */
}

struct imapscanReadKeywordInfo {
	struct maildir_kwReadInfo ri;

	struct imapscaninfo *messages;
	int hashedFilenames;
};

static struct libmail_kwMessage **findMessageByFilename(const char *filename,
						     int autocreate,
						     size_t *indexNum,
						     void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;

	size_t l;
	struct imapscanmessageinfo *i;

	struct imapscaninfo *scaninfo=info->messages;

	if (!info->hashedFilenames)
	{
		unsigned long n;

		for (n=0; n<scaninfo->nmessages; n++)
			scaninfo->msgs[n].firstBucket=NULL;

		for (n=0; n<scaninfo->nmessages; n++)
		{
			unsigned long bucket=hashFilename(scaninfo->msgs[n]
							  .filename,
							  scaninfo);

			scaninfo->msgs[n].nextBucket=
				scaninfo->msgs[bucket].firstBucket;

			scaninfo->msgs[bucket].firstBucket=scaninfo->msgs+n;
		}
		info->hashedFilenames=1;
	}

	l=strlen(filename);

	for (i= scaninfo->nmessages ?
		     scaninfo->msgs[hashFilename(filename, scaninfo)]
		     .firstBucket:NULL; i; i=i->nextBucket)
	{
		if (strncmp(i->filename, filename, l))
			continue;

		if (i->filename[l] == 0 ||
		    i->filename[l] == MDIRSEP[0])
			break;
	}

	if (!i)
		return NULL;

	if (indexNum)
		*indexNum= i-scaninfo->msgs;

	if (!i->keywordMsg && autocreate)
		imapscan_createKeyword(info->messages, i-scaninfo->msgs);

	return &i->keywordMsg;
}

static size_t getMessageCount(void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;

	return info->messages->nmessages;
}

static const char *getMessageFilename(size_t n, void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;

	if (n >= info->messages->nmessages)
		return NULL;

	return info->messages->msgs[n].filename;
}

static void updateKeywords(size_t n, struct libmail_kwMessage *kw,
			   void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;

	if (n >= info->messages->nmessages)
		return;

	if (info->messages->msgs[n].keywordMsg)
		libmail_kwmDestroy(info->messages->msgs[n].keywordMsg);

	kw->u.userNum=n;
	info->messages->msgs[n].keywordMsg=kw;
}

static struct libmail_kwHashtable * getKeywordHashtable(void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;

	return info->messages->keywordList;
}

static struct libmail_kwMessage **findMessageByIndex(size_t indexNum,
						  int autocreate,
						  void *voidarg)
{
	struct imapscanReadKeywordInfo *info=
		(struct imapscanReadKeywordInfo *)voidarg;
	struct imapscanmessageinfo *i;

	if (indexNum >= info->messages->nmessages)
		return NULL;

	i= &info->messages->msgs[indexNum];

	if (!i->keywordMsg && autocreate)
		imapscan_createKeyword(info->messages, indexNum);

	return &i->keywordMsg;
}

static void initri(struct imapscanReadKeywordInfo *rki)
{
	memset(rki, 0, sizeof(*rki));

	rki->ri.findMessageByFilename= &findMessageByFilename;
	rki->ri.getMessageCount= &getMessageCount;
	rki->ri.findMessageByIndex= &findMessageByIndex;
	rki->ri.getKeywordHashtable= &getKeywordHashtable;
	rki->ri.getMessageFilename= &getMessageFilename;
	rki->ri.updateKeywords= &updateKeywords;
	rki->ri.voidarg= rki;
}

void imapscan_readKeywords(const char *maildir,
			   struct imapscaninfo *scaninfo)
{
	struct imapscanReadKeywordInfo rki;

	initri(&rki);

	do
	{
		unsigned long i;

		for (i=0; i<scaninfo->nmessages; i++)
			if (scaninfo->msgs[i].keywordMsg)
			{
				libmail_kwmDestroy(scaninfo->msgs[i]
						      .keywordMsg);
				scaninfo->msgs[i].keywordMsg=NULL;
			}

		rki.messages=scaninfo;

		if (maildir_kwRead(maildir, &rki.ri) < 0)
			write_error_exit(0);

	} while (rki.ri.tryagain);
}

int imapscan_restoreKeywordSnapshot(FILE *fp, struct imapscaninfo *scaninfo)
{
	struct imapscanReadKeywordInfo rki;

	initri(&rki);

	rki.messages=scaninfo;
	return maildir_kwImport(fp, &rki.ri);
}

int imapscan_saveKeywordSnapshot(FILE *fp, struct imapscaninfo *scaninfo)
{
	struct imapscanReadKeywordInfo rki;

	initri(&rki);

	rki.messages=scaninfo;
	return maildir_kwExport(fp, &rki.ri);
}
