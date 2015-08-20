/*
** Copyright 2003 Double Precision, Inc.
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
#if	HAVE_UNISTD_H
#include	<unistd.h>
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

#include	<sys/types.h>
#include	<sys/stat.h>

#include	"mysignal.h"
#include	"imapd.h"
#include	"imapscanclient.h"
#include	"imapwrite.h"

#include	"maildir/config.h"
#include	"maildir/maildircreate.h"
#include	"maildir/maildirrequota.h"
#include	"maildir/maildirgetquota.h"
#include	"maildir/maildirquota.h"
#include	"maildir/maildirmisc.h"
#include	"maildir/maildirwatch.h"

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


extern int keywords();

/*
** Implement SMAP snapshots.  A snapshot is implemented, essentially, by
** saving the current folder index, restoring it, then doing a noop().
**
** The snapshot file saves uids, not complete filenames.  The complete
** filenames are already in courierimapuiddb.  Filenames are long, and saving
** them can result in huge snapshot files for large folders.  So only uids
** are saved, and when the snapshot is restored the courierimapuiddb file is
** read to obtain the filenames.
*/

extern char *current_mailbox;
extern struct imapscaninfo current_maildir_info;
extern char *readline(unsigned i, FILE *);

static char *snapshot_dir; /* Directory with snapshots */

static char *snapshot_last; /* Last snapshot */
static char *snapshot_cur;  /* Current snapshot */

static int index_dirty;
static int snapshots_enabled;

extern void set_time(const char *tmpname, time_t timestamp);
extern void smapword(const char *);

struct snapshot_list {
	struct snapshot_list *next;

	char *filename;
	char *prev;
	time_t mtime;
};

/*
** When cleaning up a snapshot directory, we need to know whether there's
** a later snapshot that claims that this snapshot is the previous
** snapshot (we can safely dump snapshots that were previous snapshots
** of previous snapshots).
*/

static struct snapshot_list *find_next_snapshot(struct snapshot_list *s,
						const char *n)
{
	const char *p, *q;

	p=strrchr(n, '/');

	if (p)
		n=p+1;

	while (s)
	{
		p=s->prev;
		q=p ? strrchr(p, '/'):NULL;

		if (q && strcmp(q+1, n) == 0)
			return s;

		s=s->next;
	}
	return NULL;
}

/*
** Delete a snapshot structure, and the actual file
*/

static void delete_snapshot(struct snapshot_list *snn)
{
	char *p=malloc(strlen(snapshot_dir)+strlen(snn->filename)+2);

	if (p)
	{
		strcat(strcat(strcpy(p, snapshot_dir), "/"), snn->filename);
		unlink(p);
	}

	free(snn->filename);
	free(snn->prev);
	free(snn);
}

/*
** Restore a snapshot
*/

static int restore_snapshot2(const char *snapshot_dir,
			     FILE *fp,
			     struct imapscaninfo *new_index);

/*
** Part 1: process the first header line of a snapshot file, and allocate a
** new folder index list.
*/

static int restore_snapshot(const char *dir, FILE *snapshot_fp,
			    char **last_snapshot)
{
	int format;
	unsigned long s_nmessages, s_uidv, s_nextuid;
	char *p;
	char *buf;
	struct imapscaninfo new_index;

	if ((buf=readline(0, snapshot_fp)) == NULL)
		return 0;

	p=strchr(buf, ':');
	if (p)
		*p++=0;

	*last_snapshot=NULL;

	if (sscanf(buf, "%d %lu %lu %lu", &format, &s_nmessages, &s_uidv,
		   &s_nextuid) != 4 || format != SNAPSHOTVERSION)
		return 0; /* Don't recognize the header */

	/* Save the previous snapshot ID */

	if (p)
	{
		*last_snapshot=malloc(strlen(dir)+strlen(p)+2);

		if (!last_snapshot)
		{
			write_error_exit(0);
			return 0;
		}

		strcat(strcat(strcpy(*last_snapshot, dir), "/"), p);
	}

	imapscan_init(&new_index);

	if (s_nmessages && (new_index.msgs=(struct imapscanmessageinfo *)
			    malloc(s_nmessages * sizeof(*new_index.msgs)))
	    == 0)
	{
		write_error_exit(0);
		return (0);
	}
	memset(new_index.msgs, 0, s_nmessages * sizeof(*new_index.msgs));

	new_index.nmessages=s_nmessages;
	new_index.uidv=s_uidv;
	new_index.nextuid=s_nextuid;

	if (restore_snapshot2(dir, snapshot_fp, &new_index))
	{
		imapscan_copy(&current_maildir_info, &new_index);
		imapscan_free(&new_index);
		return 1;
	}
	imapscan_free(&new_index);

	if (*last_snapshot)
	{
		free(*last_snapshot);
		*last_snapshot=0;
	}

	return 0;
}

/*
** Part 2: combine the snapshot and courierimapuiddb, create a halfbaked
** index from the combination.
*/

static int restore_snapshot2(const char *snapshot_dir,
			     FILE *fp,
			     struct imapscaninfo *new_index)
{
	unsigned long i;
	char *p=malloc(strlen(snapshot_dir) + sizeof("/../" IMAPDB));
	FILE *courierimapuiddb;
	int version;
	unsigned long uidv;
	unsigned long nextuid;
	char *uid_line;
	unsigned long uid=0;

	if (!p)
	{
		write_error_exit(0);
		return 0;
	}

	strcat(strcpy(p, snapshot_dir), "/../" IMAPDB);

	courierimapuiddb=fopen(p, "r");
	free(p);

	if (!courierimapuiddb)
		return 0; /* Can't open the uiddb file, no dice */

	if ((p=readline(0, courierimapuiddb)) == NULL ||
	    sscanf(p, "%d %lu %lu", &version, &uidv, &nextuid) != 3 ||
	    version != IMAPDBVERSION /* Do not recognize the uiddb file */

	    || uidv != new_index->uidv /* Something major happened, abort */ )
	{
		fclose(courierimapuiddb);
		return 0;
	}

	uid_line=readline(0, courierimapuiddb);

	if (uid_line)
	{
		if (sscanf(uid_line, "%lu", &uid) != 1)
		{
			fclose(courierimapuiddb);
			return 0;
		}
	}

	/*
	** Both the snapshot file and courierimapuiddb should be in sorted
	** order, by UIDs, rely on that and do what amounts to a merge sort.
	*/

	for (i=0; i<new_index->nmessages; i++)
	{
		unsigned long s_uid;
		char flag_buf[128];

		p=fgets(flag_buf, sizeof(flag_buf)-1, fp);

		if (p == NULL || (p=strchr(p, '\n')) == NULL ||
		    (*p = 0, sscanf(flag_buf, "%lu", &s_uid)) != 1 ||
		    (p=strchr(flag_buf, ':')) == NULL) /* Corrupted file */
		{
			fclose(courierimapuiddb);
			return 0;
		}

		new_index->msgs[i].uid=s_uid;

		/* Try to fill in the filenames to as much of an extent as
		** possible.  If IMAPDB no longer has a particular uid listed,
		** that's ok, because the message is now gone, so we just
		** insert an empty filename, which will be expunged by
		** noop() processing, after the snapshot is restored.
		*/

		while (uid_line && uid <= s_uid)
		{
			if (uid == s_uid &&
			    (uid_line=strchr(uid_line, ' ')) != NULL)
				/* Jackpot */
			{
				new_index->msgs[i].filename=
					malloc(strlen(uid_line)+
					       strlen(flag_buf)+2);

				if (!new_index->msgs[i].filename)
				{
					fclose(courierimapuiddb);
					write_error_exit(0);
					return 0;
				}

				strcpy(new_index->msgs[i].filename,
				       uid_line+1);

				if (p)
				{
					strcat(strcat(new_index->msgs[i]
						      .filename, MDIRSEP),
					       p+1);
				}
			}

			uid_line=readline(0, courierimapuiddb);

			if (uid_line)
			{
				if (sscanf(uid_line, "%lu", &uid) != 1)
				{
					fclose(courierimapuiddb);
					return 0;
				}
			}
		}


		if (new_index->msgs[i].filename == 0)
		{
			new_index->msgs[i].filename=strdup("");
			/* A noop should get rid of this entry anyway */

			if (!new_index->msgs[i].filename)
			{
				fclose(courierimapuiddb);
				write_error_exit(0);
				return 0;
			}
		}
	}

	fclose(courierimapuiddb);
	if (keywords())
		imapscan_restoreKeywordSnapshot(fp, new_index);
	return 1;
}

void snapshot_select(int flag)
{
	snapshots_enabled=flag;
}

/*
** Initialize snapshots for an opened folder.
**
** Parameters:
**
**     folder - the path to a folder that's in the process of opening.
**
**     snapshot - not NULL if the client requested a snapshot restore.
**
** Exit code:
**
** When a snapshot is requested, a non-zero exit code means that the
** snapshot has been succesfully restored, and current_mailbox is now
** initialized based on the snapshot.  A zero exit code means that the
** snapshot has not been restored, and snapshot_init() needs to be called
** again with snapshot=NULL in order to initialize the snapshot structures.
**
** When a snapshot is not requested, the exit code is always 0
*/

int snapshot_init(const char *folder, const char *snapshot)
{
	struct snapshot_list *sl=NULL;
	DIR *dirp;
	struct dirent *de;
	struct snapshot_list *snn, **ptr;
	int cnt;
	char *new_dir;
	int rc=0;
	char *new_snapshot_cur=NULL;
	char *new_snapshot_last=NULL;

	if ((new_dir=malloc(strlen(folder)+sizeof("/" SNAPSHOTDIR)))
	    == NULL)
	{
		write_error_exit(0);
		return rc;
	}

	strcat(strcpy(new_dir, folder), "/" SNAPSHOTDIR);
	mkdir(new_dir, 0755); /* Create, if doesn't exist */

	if (snapshot)
	{
		FILE *fp;

		if (*snapshot == 0 || strchr(snapshot, '/') ||
		    *snapshot == '.') /* Monkey business */
		{
			free(new_dir);
			return 0;
		}

		new_snapshot_cur=malloc(strlen(new_dir) +
					strlen(snapshot) + 2);

		if (!new_snapshot_cur)
		{
			free(new_dir);
			write_error_exit(0);
			return rc;
		}

		strcat(strcat(strcpy(new_snapshot_cur, new_dir), "/"),
		       snapshot);

		if ((fp=fopen(new_snapshot_cur, "r")) != NULL &&
		    restore_snapshot(new_dir, fp, &new_snapshot_last))
		{
			set_time(new_snapshot_cur, time(NULL));
			rc=1; /* We're good to go.  Finish everything else */
		}

		if (fp)
		{
			fclose(fp);
			fp=NULL;
		}

		if (!rc) /* Couldn't get the snapshot, abort */
		{
			free(new_snapshot_cur);
			free(new_dir);
			return 0;
		}
	}

	if (snapshot_dir) free(snapshot_dir);
	if (snapshot_last) free(snapshot_last);
	if (snapshot_cur) free(snapshot_cur);

	snapshot_dir=NULL;
	snapshot_last=new_snapshot_last;
	snapshot_cur=new_snapshot_cur;

	snapshot_dir=new_dir;

	index_dirty=1;

	/* Get rid of old snapshots as follows */

	/* Step 1, compile a list of snapshots, sorted in mtime order */

	dirp=opendir(snapshot_dir);

	while (dirp && (de=readdir(dirp)) != NULL)
	{
		FILE *fp;
		struct stat stat_buf;

		char *n;

		if (de->d_name[0] == '.') continue;

		n=malloc(strlen(snapshot_dir)+strlen(de->d_name)+2);
		if (!n) break; /* Furrfu */

		strcat(strcat(strcpy(n, snapshot_dir), "/"), de->d_name);

		fp=fopen(n, "r");

		if (fp)
		{
			char buf[1024];

			if (fgets(buf, sizeof(buf)-1, fp) != NULL &&
			    fstat(fileno(fp), &stat_buf) == 0)
			{
				char *p=strchr(buf, '\n');
				int fmt;

				if (p) *p=0;

				p=strchr(buf, ':');

				if (p)
					*p++=0;


				if (sscanf(buf, "%d", &fmt) == 1 &&
				    fmt == SNAPSHOTVERSION)
				{
					snn=malloc(sizeof(*sl));

					if (snn) memset(snn, 0, sizeof(*snn));

					if (snn == NULL ||
					    (snn->filename=strdup(de->d_name))
					    == NULL ||
					    (snn->prev=strdup(p ? p:""))
					    == NULL)
					{
						if (snn && snn->filename)
							free(snn->filename);
						if (snn)
							free(snn);

						snn=NULL;
					}

					if (snn)
					{
						snn->mtime=stat_buf.st_mtime;

						for (ptr= &sl; *ptr;
						     ptr=&(*ptr)->next)
						{
							if ( (*ptr)->mtime >
							     snn->mtime)
								break;
						}

						snn->next= *ptr;
						*ptr=snn;
					}
					free(n);
					n=NULL;
				}

			}
			fclose(fp);
		}
		if (n)
		{
			unlink(n);
			free(n);
		}
	}
	if (dirp)
		closedir(dirp);

	/* Step 2: drop snapshots that are definitely obsolete */

	for (ptr= &sl; *ptr; )
	{
		if ((snn=find_next_snapshot(sl, (*ptr)->filename)) &&
		    find_next_snapshot(sl, snn->filename))
		{
			snn= *ptr;

			*ptr=snn->next;

			delete_snapshot(snn);
		}
		else
			ptr=&(*ptr)->next;

	}

	/* If there are more than 10 snapshots, drop older snapshots */

	cnt=0;
	for (snn=sl; snn; snn=snn->next)
		++cnt;

	if (cnt > 10)
	{
		time_t now=time(NULL);

		while (sl && sl->mtime < now &&
		       (now - sl->mtime) > 60 * 60 * 24 * (7 + (cnt-10)*2))
		{
			snn=sl;
			sl=sl->next;
			delete_snapshot(snn);
			--cnt;
		}
	}

	/* All right, put a lid on 50 snapshots */

	while (cnt > 50)
	{
		snn=sl;
		sl=sl->next;
		delete_snapshot(snn);
		--cnt;
	}

	return rc;
}

/*
** Something changed in the folder, so next time snapshot_save() was called,
** take a snapshot.
*/

void snapshot_needed()
{
	index_dirty=1;
}

/*
** Save a snapshot, if the folder was changed.
*/

void snapshot_save()
{
	int rc;
	struct maildir_tmpcreate_info createInfo;
	FILE *fp;
	unsigned long i;
	const char *q;

	if (!index_dirty || !snapshots_enabled)
		return;

	index_dirty=0;

	maildir_tmpcreate_init(&createInfo);

	createInfo.maildir=current_mailbox;
	createInfo.uniq="snapshot";
	createInfo.hostname=getenv("HOSTNAME");
	createInfo.doordie=1;

	if ((rc=maildir_tmpcreate_fd(&createInfo)) < 0)
	{
		perror("maildir_tmpcreate_fd");
		return;
	}
	close(rc);

	q=strrchr(createInfo.tmpname, '/'); /* Always there */

	free(createInfo.newname);
	createInfo.newname=malloc(strlen(snapshot_dir)+strlen(q)+2);

	if (!createInfo.newname)
	{
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		perror("malloc");
		return;
	}

	strcat(strcat(strcpy(createInfo.newname, snapshot_dir), "/"), q);

	if ((fp=fopen(createInfo.tmpname, "w")) == NULL)
	{
		perror(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return;
	}

	fprintf(fp, "%d %lu %lu %lu", SNAPSHOTVERSION,
		current_maildir_info.nmessages,
		current_maildir_info.uidv,
		current_maildir_info.nextuid);
	if (snapshot_cur)
		fprintf(fp, ":%s", strrchr(snapshot_cur, '/')+1);
	fprintf(fp, "\n");

	for (i=0; i<current_maildir_info.nmessages; i++)
	{
		struct imapscanmessageinfo *p=current_maildir_info.msgs + i;
		q=strrchr(p->filename, MDIRSEP[0]);

		fprintf(fp, "%lu:%s\n", p->uid, q ? q+1:"");
	}

	if (keywords())
		imapscan_saveKeywordSnapshot(fp, &current_maildir_info);

	if (fflush(fp) < 0 || ferror(fp) < 0)
	{
		fclose(fp);
		perror(createInfo.tmpname);
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return;
	}
	fclose(fp);
	if (rename(createInfo.tmpname, createInfo.newname) < 0)
	{
		perror(createInfo.tmpname);
		unlink(createInfo.tmpname);
		maildir_tmpcreate_free(&createInfo);
		return;
	}
	if (snapshot_last)
	{
		unlink(snapshot_last); /* Obsolete snapshot */
		free(snapshot_last);
	}

	snapshot_last=snapshot_cur;
	snapshot_cur=createInfo.newname;
	createInfo.newname=NULL;
	maildir_tmpcreate_free(&createInfo);

	writes("* SNAPSHOT ");
	smapword(strrchr(snapshot_cur, '/')+1);
	writes("\n");
}
