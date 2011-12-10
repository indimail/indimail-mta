/*
** Copyright 2003-2007 Double Precision, Inc.
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

#include	"maildir/config.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirwatch.h"
#include	"numlib/numlib.h"

#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	"maildirkeywords.h"
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


int libmail_kwEnabled=1;

struct keywordUpdateInfo {
	size_t highestN;
	size_t highestT;
	size_t totalCnt;
	int foundNewest;
};

static void scan_updates(const char *dir,
			 time_t t,
			 time_t tn,
			 struct maildir_kwReadInfo *info,
			 struct keywordUpdateInfo **updateInfo);

static void read_updates(const char *dir,
			 struct maildir_kwReadInfo *info,
			 struct keywordUpdateInfo *updateInfo);

static void purge_old_updates(const char *dir,
			      time_t tn,
			      struct maildir_kwReadInfo *info,
			      struct keywordUpdateInfo *updateInfo);

static int save_updated_keywords(const char *maildir, const char *kwname,
				 struct maildir_kwReadInfo *info,
				 struct keywordUpdateInfo *updateInfo);

static void doReadKeywords2(const char *maildir, const char *dir,
			    struct maildir_kwReadInfo *rki);


struct readLine {
	char *lineBuf;
	size_t lineBufSize;
	int errflag;
};

static void rl_init(struct readLine *p)
{
	p->lineBuf=NULL;
	p->lineBufSize=0;
	p->errflag=0;
}

static void rl_free(struct readLine *p)
{
	if (p->lineBuf)
		free(p->lineBuf);
}

static char *rl_read(struct readLine *p, FILE *f)
{
	size_t n=0;
	int ch;

	for (;;)
	{
		if (n >= p->lineBufSize)
		{
			size_t o= n + 1024;
			char *b= p->lineBuf ? realloc(p->lineBuf, o):malloc(o);

			if (!b)
			{
				p->errflag=1;
				return NULL;
			}

			p->lineBuf=b;
			p->lineBufSize=o;
		}

		if ((ch=getc(f)) == EOF || ch == '\n')
			break;
		p->lineBuf[n++]=(char)ch;
	}

	p->lineBuf[n]=0;

	return n == 0 && ch == EOF ? NULL:p->lineBuf;
}

int maildir_kwRead(const char *maildir,
			  struct maildir_kwReadInfo *rki)
{
	char *p=malloc(strlen(maildir)+sizeof("/" KEYWORDDIR));

	if (!p)
		return -1;

	strcat(strcpy(p, maildir), "/" KEYWORDDIR);

	rki->errorOccured=0;
	doReadKeywords2(maildir, p, rki);
	free(p);

	return rki->errorOccured;
}

static void doReadKeywords2(const char *maildir, const char *dir,
			   struct maildir_kwReadInfo *rki)
{
	FILE *fp;
	char *kwname=malloc(strlen(dir)+sizeof("/:list"));
	struct keywordUpdateInfo *updateInfo;

	time_t t=time(NULL);
	time_t tn=t/300;

	rki->updateNeeded=0;
	rki->tryagain=0;

	if (!kwname)
	{
		rki->errorOccured= -1;
		return;
	}

	strcat(strcpy(kwname, dir), "/:list");

	fp=fopen(kwname, "r");

	if (!fp) /* Maybe the keyword directory needs creating? */
	{
		struct stat stat_buf;

		mkdir(dir, 0700);

		/* Give it same mode as the maildir */

		if (stat(maildir, &stat_buf) == 0)
			chmod(dir, stat_buf.st_mode & 0777);
	}

	/*
	** If keywords are disabled, we still need to create the keyword
	** directory, otherwise FAM-based IDLE will not work.
	*/

	if (!libmail_kwEnabled)
	{
		if (fp)
			fclose(fp);
		free(kwname);
		return;
	}

	if (fp && maildir_kwImport(fp, rki))
		rki->updateNeeded=1;
	if (fp)
		fclose(fp);

	updateInfo=malloc(sizeof(struct keywordUpdateInfo)
			  *( 1+(*rki->getMessageCount)(rki->voidarg)));

	if (!updateInfo)
	{
		free(kwname);
		return;
	}

	scan_updates(dir, t, tn, rki, &updateInfo);
	read_updates(dir, rki, updateInfo);

	if (!rki->tryagain && !rki->errorOccured)
	{
		if (save_updated_keywords(maildir, kwname, rki, updateInfo)
		    == 0 && !rki->errorOccured)
			purge_old_updates(dir, tn, rki, updateInfo);
	}

	free(updateInfo);
	free(kwname);
}

int maildir_kwImport(FILE *fp, struct maildir_kwReadInfo *rki)
{
	struct keywordIndex {
		struct keywordIndex *next;
		struct libmail_keywordEntry *kw;
	} *firstKw=NULL, *lastKw=NULL, **index;
	size_t numKw=0;
	struct libmail_kwMessage *tmpMsg;
	char *p;
	int rc=0;

	struct readLine rl;

	if ((tmpMsg=libmail_kwmCreate()) == NULL)
	{
		rki->errorOccured=-1;
		return 0;
	}

	rl_init(&rl);

	while ((p=rl_read(&rl, fp)) != NULL && *p)
	{
		struct keywordIndex *ki=malloc(sizeof(*firstKw));

		if (!ki)
		{
			rki->errorOccured=-1;
			rl_free(&rl);
			libmail_kwmDestroy(tmpMsg);
			return 0;
		}

		if (lastKw)
			lastKw->next=ki;
		else
			firstKw=ki;

		lastKw=ki;
		ki->next=NULL;
		++numKw;

		ki->kw=libmail_kweFind((*rki->getKeywordHashtable)
					(rki->voidarg), p, 1);
		if (libmail_kwmSet(tmpMsg, ki->kw) < 0)
		{
			rki->errorOccured= -1;
			break;
		}
	}

	if (rki->errorOccured ||
	    (index=malloc(sizeof(*firstKw)*(numKw+1))) == NULL)
	{
		while ((lastKw=firstKw) != NULL)
		{
			firstKw=firstKw->next;
			free(lastKw);
		}
		libmail_kwmDestroy(tmpMsg);
		rki->errorOccured= -1;
		rl_free(&rl);
		return 0;
	}

	numKw=0;

	while (firstKw)
	{
		index[numKw]=firstKw;
		++numKw;
		firstKw=firstKw->next;
	}
	index[numKw]=0;

	if (p)
		while ((p=rl_read(&rl, fp)) != NULL)
		{
			char *q=strchr(p, ':');
			struct libmail_kwMessage **i;
			size_t l;
			size_t n;

			if (!q)
			{
				rc=1;
				continue; /* Crap */
			}

			*q++=0;

			i= (*rki->findMessageByFilename)(p, 0, &n,
							 rki->voidarg);

			if (!i)
			{
				rc=1;
				continue; /* Stale data */
			}

			if (*i) /* Already have it */
				libmail_kwmDestroy(*i);
			(*i)=NULL;

			i=NULL;

			while ((q=strtok(q, " ")) != NULL)
			{
				l=atoi(q);
				q=NULL;

				if (l < numKw)
				{
					if (!i)
						i= (*rki->
						    findMessageByFilename)
							(p, 1, &n,
							 rki->voidarg);
					/* Autocreate it */

					if (*i == NULL) /* ENOMEM */
					{
						rc= -1;
						break;
					}

					if (libmail_kwmSet(*i, index[l]->kw)
					    < 0)
						rki->errorOccured= -1;
				}
			}
		}

	rl_free(&rl);
	for (numKw=0; index[numKw]; numKw++)
		free(index[numKw]);

	free(index);
	libmail_kwmDestroy(tmpMsg);
	return rc;
}

/* See the README */

static void scan_updates(const char *dir,
			 time_t t,
			 time_t tn,
			 struct maildir_kwReadInfo *info,
			 struct keywordUpdateInfo **updateInfo)
{
	DIR *dirp;
	struct dirent *de;
	unsigned long i;

	size_t n= (*info->getMessageCount)(info->voidarg);

	for (i=0; i<n; i++)
	{
		(*updateInfo)[i].highestN=0;
		(*updateInfo)[i].highestT=0;
		(*updateInfo)[i].totalCnt=0;
		(*updateInfo)[i].foundNewest=0;
	}

	dirp=opendir(dir);
	while (dirp && (de=readdir(dirp)) != NULL)
	{
		struct libmail_kwMessage **i;
		unsigned long x=0;
		char *p, *q;
		size_t in;

		if (de->d_name[0] == ':')
			continue;

		if (de->d_name[0] != '.')
			i=(*info->findMessageByFilename)
				(de->d_name, 0, &in, info->voidarg);

		else if ((x=libmail_atotime_t(de->d_name+1)) == 0)
		{
			if (strncmp(de->d_name, ".tmp.", 5) == 0)
			{
				if ((x=libmail_atotime_t(de->d_name+5)) != 0
				    && x >= t - 120)
					continue; /* New scratch file */


				p=malloc(strlen(dir)+strlen(de->d_name)+2);

				if (!p)
				{
					closedir(dirp);
					info->errorOccured= -1;
					return;
				}

				strcat(strcat(strcpy(p, dir), "/"),
				       de->d_name);
				unlink(p);
				free(p);
			}
			continue;
		}
		else
		{
			const char *p=strchr(de->d_name+1, '.');

			if (!p)
				continue;

			i=(*info->findMessageByFilename)
				(p+1, 0, &in, info->voidarg);
		}

		p=malloc(strlen(dir)+strlen(de->d_name)+2);

		if (!p)
		{
			closedir(dirp);
			info->errorOccured= -1;
			return;
		}

		strcat(strcat(strcpy(p, dir), "/"), de->d_name);

		if (!i)
		{
			struct stat stat_buf;

			if (stat(p, &stat_buf) == 0 &&
			    stat_buf.st_mtime < t - 15 * 60)
				unlink(p);
			free(p);
			continue;
		}
		free(p);


		if (in >= n)
		{
			/* libmail_kwgReadMaildir autocrerate */
			
			struct keywordUpdateInfo *u=
				realloc(*updateInfo,
					sizeof(**updateInfo) * (in+1));

			if (!u)
			{
				closedir(dirp);
				info->errorOccured= -1;
				return;
			}

			*updateInfo=u;

			while (n <= in)
			{

				(*updateInfo)[n].highestN=0;
				(*updateInfo)[n].highestT=0;
				(*updateInfo)[n].totalCnt=0;
				(*updateInfo)[n].foundNewest=0;
				++n;
			}
		}


		if (de->d_name[0] != '.')
		{
			x=tn+1;

			(*updateInfo)[in].foundNewest=1;
		}

		++(*updateInfo)[in].totalCnt;

		if (x >= tn)
		{
			if (x > (*updateInfo)[in].highestT)
				(*updateInfo)[in].highestT=x;
		}
		else if ((*updateInfo)[in].highestN < x)
		{
			if ((*updateInfo)[in].highestN > 0)
			{
				char b[NUMBUFSIZE];
				char *r;

				libmail_str_size_t((*updateInfo)[in].highestN, b);

				r=de->d_name;
				if (*r == '.')
					r=strchr(r+1, '.')+1;

				q=malloc(strlen(dir)+strlen(r)+
					 strlen(b)+4);

				if (!q)
				{
					closedir(dirp);
					info->errorOccured= -1;
					return;
				}

				sprintf(q, "%s/.%s.%s", dir, b, r);
				unlink(q);
				free(q);
			}

			(*updateInfo)[in].highestN=x;
		}
	}

	if (dirp)
		closedir(dirp);
}

static void read_updates(const char *dir,
			 struct maildir_kwReadInfo *info,
			 struct keywordUpdateInfo *updateInfo)
{
	unsigned long i;
	size_t msgCnt= (*info->getMessageCount)(info->voidarg);
	struct readLine rl;

	struct libmail_kwHashtable *h=
		(*info->getKeywordHashtable)(info->voidarg);

	rl_init(&rl);
	for (i=0; i<msgCnt; i++)
	{
		size_t n=updateInfo[i].highestN;
		char *p, *q;
		char b[NUMBUFSIZE];
		FILE *fp;
		struct libmail_kwMessage *km, **oldKm;
		const char *fn;

		if (n < updateInfo[i].highestT)
			n=updateInfo[i].highestT;

		if (n == 0)
			continue;

		libmail_str_size_t(n, b);

		fn= (*info->getMessageFilename)(i, info->voidarg);

		if (!fn)
			continue; /* Shouldn't happen */

		p=malloc(strlen(dir)+strlen(b)+strlen(fn)+4);

		if (!p)
		{
			info->errorOccured= -1;
			rl_free(&rl);
			return;
		}

		if (updateInfo[i].foundNewest)
			sprintf(p, "%s/%s", dir, fn);
		else
			sprintf(p, "%s/.%s.%s", dir, b, fn);


		q=strrchr(strrchr(p, '/'), MDIRSEP[0]);
		if (q)
			*q=0;

		fp=fopen(p, "r");
		free(p);

		if (!fp)
		{
			if (errno == ENOENT)
			{
				info->tryagain=1;
				rl_free(&rl);
				return;
			}

			continue;
		}

		if ((km=libmail_kwmCreate()) == NULL)
		{
			fclose(fp);
			info->errorOccured= -1;
			rl_free(&rl);
			return;
		}

		while ((p=rl_read(&rl, fp)) != NULL && *p)
			if (libmail_kwmSetName(h, km, p) < 0)
			{
				fclose(fp);
				info->errorOccured= -1;
				rl_free(&rl);
				return;
			}

		if (km->firstEntry == NULL)
		{
			oldKm= (*info->findMessageByIndex)(i,
							   0, info->voidarg);
			if (oldKm && *oldKm)
			{
				info->updateNeeded=1;
				libmail_kwmDestroy(*oldKm);
				*oldKm=NULL;
			}
			libmail_kwmDestroy(km);
		}
		else
		{
			oldKm= (*info->findMessageByIndex)(i, 1,
							   info->voidarg);

			if (oldKm && *oldKm == NULL)
			{
				info->updateNeeded=1;
				(*info->updateKeywords)(i, km, info->voidarg);
			}
			else if (!oldKm /* Shouldn't happen */
			    || libmail_kwmCmp(*oldKm, km) == 0)
			{
				libmail_kwmDestroy(km);
			}
			else
			{
				info->updateNeeded=1;
				(*info->updateKeywords)(i, km, info->voidarg);
			}
		}

		fclose(fp);
	}
	rl_free(&rl);
}

struct saveUpdateInfo {
	FILE *fp;
	unsigned long counter;
};

static int saveKeywordList(struct libmail_keywordEntry *ke,
			   void *vp)
{
	struct saveUpdateInfo *sui=(struct saveUpdateInfo *)vp;

	fprintf(sui->fp, "%s\n", keywordName(ke));

	ke->u.userNum= sui->counter;

	++sui->counter;
	return 0;
}

int maildir_kwExport(FILE *fp, struct maildir_kwReadInfo *info)
{
	struct saveUpdateInfo sui;
	size_t i, n;

	sui.fp=fp;
	sui.counter=0;
	libmail_kwEnumerate((*info->getKeywordHashtable)(info->voidarg),
			 saveKeywordList, &sui);

	if (sui.counter == 0) /* No keywords set for any message */
		return 0;

	fprintf(fp, "\n");

	n= (*info->getMessageCount)(info->voidarg);

	for (i=0; i<n; i++)
	{
		const char *p;

		struct libmail_kwMessage **km=
			(*info->findMessageByIndex)(i, 0, info->voidarg);
		struct libmail_kwMessageEntry *kme;

		if (!km || !*km || (*km)->firstEntry == NULL)
			continue;

		for (p= (*info->getMessageFilename)(i, info->voidarg);
		     *p && *p != MDIRSEP[0]; p++)
			putc(*p, fp);

		putc(':', fp);
		p="";

		for (kme=(*km)->firstEntry; kme; kme=kme->next)
		{
			fprintf(fp, "%s%lu", p, kme->libmail_keywordEntryPtr
				->u.userNum);
			p=" ";
		}
		fprintf(fp, "\n");
	}

	return 1;
}

static int save_updated_keywords(const char *maildir, const char *kwname,
				 struct maildir_kwReadInfo *info,
				 struct keywordUpdateInfo *updateInfo)
{
	struct maildir_tmpcreate_info createInfo;
	FILE *fp;

	if (!info->updateNeeded)
		return 0;

	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=maildir;
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((fp=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		perror("maildir_tmpcreate_fp");
		return -1;
	}

	if (maildir_kwExport(fp, info) == 0)
	{
		fclose(fp);
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		unlink(kwname);
		return 0;
	}
	if (fflush(fp) < 0 || ferror(fp))
	{
		fclose(fp);
		unlink(createInfo.tmpname);
		perror(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return -1;
	}
	fclose(fp);

	if (rename(createInfo.tmpname, kwname))
	{
		perror(createInfo.tmpname);
		unlink(createInfo.tmpname);
		perror(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return -1;
	}
	maildir_tmpcreate_free(&createInfo);
	return 0;
}

static void purge_old_updates(const char *dir,
			      time_t tn,
			      struct maildir_kwReadInfo *info,
			      struct keywordUpdateInfo *updateInfo)
{
	size_t i;

	time_t x;

	size_t n= (*info->getMessageCount)(info->voidarg);

	for (i=0; i<n; i++)
	{
		char *p, *q, *c;
		char b[NUMBUFSIZE];

		const char *fn= (*info->getMessageFilename)(i, info->voidarg);

		if (!fn)
			continue; /* Shouldn't happen */

		if (updateInfo[i].foundNewest)
		{
			size_t l;

			x=tn+1;

			libmail_str_size_t(x, b);

			l=strlen(dir)+strlen(b)+strlen(fn)+4;

			p=malloc(l);
			if (!p)
			{
				info->errorOccured= -1;
				return;
			}

			q=malloc(l);
			if (!q)
			{
				free(p);
				info->errorOccured= -1;
				return;
			}

			sprintf(p, "%s/%s", dir, fn);

			sprintf(q, "%s/.%s.%s", dir, b, fn);


			c=strrchr(strrchr(p, '/'), MDIRSEP[0]);

			if (c) *c=0;
			c=strrchr(strrchr(q, '/'), MDIRSEP[0]);

			if (c) *c=0;

			rename(p, q);
			free(q);
			free(p);
			continue;
		}

		if (! (updateInfo[i].totalCnt == 1 &&
		       updateInfo[i].highestN))
			continue;

		libmail_str_size_t(updateInfo[i].highestN, b);

		p=malloc(strlen(dir)+strlen(b)+strlen(fn)+4);

		if (!p)
		{
			info->errorOccured= -1;
			return;
		}

		sprintf(p, "%s/.%s.%s", dir, b, fn);

		q=strrchr(strrchr(p, '/'), MDIRSEP[0]);
		if (q) *q=0;
		unlink(p);
		free(p);
	}
}


/***************/

static int maildir_kwSaveCommon(const char *maildir,
				const char *filename,
				struct libmail_kwMessage *newKeyword,
				const char **newKeywordArray,
				char **tmpname,
				char **newname,
				int tryAtomic);

int maildir_kwSave(const char *maildir,
		   const char *filename,
		   struct libmail_kwMessage *newKeyword,
		   char **tmpname,
		   char **newname,
		   int tryAtomic)
{
	return maildir_kwSaveCommon(maildir, filename, newKeyword, NULL,
				    tmpname, newname, tryAtomic);
}

int maildir_kwSaveArray(const char *maildir,
			const char *filename,
			const char **flags,
			char **tmpname,
			char **newname,
			int tryAtomic)
{
	return maildir_kwSaveCommon(maildir, filename, NULL, flags,
				    tmpname, newname, tryAtomic);
}

static int maildir_kwSaveCommon(const char *maildir,
				const char *filename,
				struct libmail_kwMessage *newKeyword,
				const char **newKeywordArray,
				char **tmpname,
				char **newname,
				int tryAtomic)
{
	struct maildir_tmpcreate_info createInfo;
	FILE *fp;
	char *n=malloc(strlen(maildir)+strlen(filename)+10+
		       sizeof(KEYWORDDIR)), *p;
	struct libmail_kwMessageEntry *kme;

	if (!n)
		return -1;

	strcat(strcat(strcpy(n, maildir),
		      "/" KEYWORDDIR "/"), filename);

	p=strrchr(strrchr(n, '/'), MDIRSEP[0]);
	if (p)
		*p=0;

	
	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=maildir;
	createInfo.msgsize=0;
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((fp=maildir_tmpcreate_fp(&createInfo)) == NULL)
	{
		free(n);
		return -1;
	}

	if (newKeywordArray)
	{
		size_t i;

		for (i=0; newKeywordArray[i]; i++)
			fprintf(fp, "%s\n", newKeywordArray[i]);
	}
	else for (kme=newKeyword ? newKeyword->firstEntry:NULL;
		  kme; kme=kme->next)
		fprintf(fp, "%s\n", keywordName(kme->libmail_keywordEntryPtr));

	errno=EIO;

	if (fflush(fp) < 0 || ferror(fp))
	{
		fclose(fp);
		free(n);
		return -1;
	}

	fclose(fp);

	*tmpname=createInfo.tmpname;
	createInfo.tmpname=NULL;
	*newname=n;
	maildir_tmpcreate_free(&createInfo);

	if (tryAtomic)
	{
		char timeBuf[NUMBUFSIZE];

		char *n=malloc(strlen(*tmpname)
			       + sizeof(KEYWORDDIR) + NUMBUFSIZE+10);

		if (!n)
		{
			free(*tmpname);
			free(*newname);
			return -1;
		}

		strcat(strcat(strcat(strcpy(n, maildir),
				     "/" KEYWORDDIR "/.tmp."),
			      libmail_str_time_t(time(NULL), timeBuf)),
		       strrchr( *tmpname, '/')+1);


		if (rename( *tmpname, n) < 0)
		{
			free(n);
			free(*tmpname);
			free(*newname);
			return -1;
		}

		free (*tmpname);
		*tmpname=n;
	}
	return 0;
}
