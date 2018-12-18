/*
** Copyright 2006 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	"liblock.h"
#include	"mail.h"
#include	"../numlib/numlib.h"
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<errno.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#if HAVE_FCNTL_H
#include	<fcntl.h>
#endif


struct ll_mail *ll_mail_alloc(const char *filename)
{
	struct ll_mail *p=(struct ll_mail *)malloc(sizeof(struct ll_mail));

	if (!p)
		return NULL;

	if ((p->file=strdup(filename)) == NULL)
	{
		free(p);
		return NULL;
	}

	p->cclientfd= -1;
	p->cclientfile=NULL;

	p->dotlock=NULL;

	return p;
}

#define IDBUFSIZE 512

/*
** For extra credit, we mark our territory.
*/

static void getid(char *idbuf)
{
	libmail_str_pid_t(getpid(), idbuf);

	while (*idbuf)
		idbuf++;

	*idbuf++=':';

	idbuf[IDBUFSIZE-NUMBUFSIZE-10]=0;

	if (gethostname(idbuf, IDBUFSIZE-NUMBUFSIZE-10) < 0)
		strcpy(idbuf, "localhost");
}

static int writeid(char *idbuf, int fd)
{
	int l=strlen(idbuf);

	while (l)
	{
		int n=write(fd, idbuf, l);

		if (n <= 0)
			return (-1);

		l -= n;
		idbuf += n;
	}
	return 0;
}

static int readid(char *p, int fd)
{
	int l=IDBUFSIZE-1;

	while (l)
	{
		int n=read(fd, p, l);

		if (n < 0)
			return (-1);

		if (n == 0)
			break;

		p += n;
		l -= n;
	}
	*p=0;
	return 0;
}

static pid_t getpidid(char *idbuf, char *myidbuf)
{
	pid_t p=atol(idbuf);

	if ((idbuf=strchr(idbuf, ':')) == NULL ||
	    (myidbuf=strchr(myidbuf, ':')) == NULL ||
	    strcmp(idbuf, myidbuf))
		return 0;

	return p;
}

int ll_mail_lock(struct ll_mail *p)
{
	struct stat stat_buf;
	char idbuf[IDBUFSIZE];
	char idbuf2[IDBUFSIZE];

	char fn[NUMBUFSIZE*2 + 20];
	char *f;
	int fd;

	getid(idbuf);

	if (p->cclientfd >= 0)
		return 0;

	if (stat(p->file, &stat_buf) < 0)
		return -1;

	if (snprintf(fn, sizeof(fn), "/tmp/.%lx.%lx",
		     (unsigned long)stat_buf.st_dev,
		     (unsigned long)stat_buf.st_ino) < 0)
	{
		errno=ENOSPC;
		return (-1);
	}

	if ((f=strdup(fn)) == NULL)
		return (-1);

	/* We do things a bit differently.  First, try O_EXCL */

	if ((fd=open(f, O_RDWR|O_CREAT|O_EXCL, 0644)) >= 0)
	{
		struct stat stat_buf2;

		if (ll_lockfd(fd, ll_writelock, ll_whence_start, 0) < 0 ||
		    fcntl(fd, F_SETFD, FD_CLOEXEC) < 0 ||
		    writeid(idbuf, fd) < 0)
		{
			/* This shouldn't happen */

			close(fd);
			free(f);
			return (-1);
		}

		/* Rare race condition: */

		if (fstat(fd, &stat_buf) < 0 ||
		    lstat(f, &stat_buf2) < 0 ||
		    stat_buf.st_dev != stat_buf2.st_dev ||
		    stat_buf.st_ino != stat_buf2.st_ino)
		{
			errno=EAGAIN;
			close(fd);
			free(f);
			return (-1);
		}

		p->cclientfd=fd;
		p->cclientfile=f;
		return 0;
	}

	/*
	** An existing lockfile.  See if it's tagged with another
	** pid on this server, which no longer exists.
	*/

	if ((fd=open(f, O_RDONLY)) >= 0)
	{
		pid_t p=-1;

		if (readid(idbuf2, fd) == 0 &&
		    (p=getpidid(idbuf2, idbuf)) != 0 &&
		    kill(p, 0) < 0 && errno == ESRCH)
		{
			errno=EAGAIN;
			close(fd);
			unlink(f); /* Don't try again right away */
			free(f);
			return (-1);
		}

		/* If we can't lock, someone must have it open, game over. */

		if (p == getpid() /* It's us! */

		    || ll_lockfd(fd, ll_readlock, ll_whence_start, 0) < 0)
		{
			errno=EEXIST;
			close(fd);
			free(f);
			return (-1);
		}

		close(fd);
	}

	/* Stale 0-length lockfiles are blown away after 5 mins */

	if (lstat(f, &stat_buf) == 0 && stat_buf.st_size == 0 &&
	    stat_buf.st_mtime + 300 < time(NULL))
	{
		errno=EAGAIN;
		unlink(f);
		free(f);
		return (-1);
	}

	errno=EAGAIN;
	free(f);
	return (-1);
}

/* Try to create a dot-lock */

static int try_dotlock(const char *tmpfile,
		       const char *dotlock,
		       char *idbuf);

static int try_mail_dotlock(const char *dotlock, char *idbuf)
{
	char timebuf[NUMBUFSIZE];
	char pidbuf[NUMBUFSIZE];
	char *tmpname;
	int rc;

	libmail_str_time_t(time(NULL), timebuf);
	libmail_str_pid_t(getpid(), pidbuf);

	tmpname=malloc(strlen(dotlock) + strlen(timebuf) + strlen(pidbuf) +
		       strlen(idbuf) + 10);

	if (!tmpname)
		return -1;

	strcpy(tmpname, dotlock);
	strcat(tmpname, ".");
	strcat(tmpname, timebuf);
	strcat(tmpname, ".");
	strcat(tmpname, pidbuf);
	strcat(tmpname, ".");
	strcat(tmpname, strchr(idbuf, ':')+1);

	rc=try_dotlock(tmpname, dotlock, idbuf);
	free(tmpname);
	return (rc);
}

static int try_dotlock(const char *tmpname,
		       const char *dotlock,
		       char *idbuf)
{
	struct stat stat_buf;

	int fd;

	fd=open(tmpname, O_RDWR | O_CREAT, 0644);

	if (fd < 0)
		return (-1);

	if (writeid(idbuf, fd))
	{
		close(fd);
		unlink(tmpname);
		return (-1);
	}
	close(fd);

	if (link(tmpname, dotlock) < 0 || stat(tmpname, &stat_buf) ||
	    stat_buf.st_nlink != 2)
	{
		if (errno != EEXIST)
			errno=EIO;

		unlink(tmpname);
		return (-1);
	}
	unlink(tmpname);
	return (0);
}

static void dotlock_exists(const char *dotlock, char *myidbuf,
			   int timeout)
{
	char idbuf[IDBUFSIZE];
	struct stat stat_buf;
	int fd;

	if ((fd=open(dotlock, O_RDONLY)) >= 0)
	{
		pid_t p;

		/*
		** Where the locking process is on the same server,
		** the decision is easy: does the process still exist,
		** or not?
		*/

		if (readid(idbuf, fd) == 0 && (p=getpidid(idbuf, myidbuf)))
		{
			if (p == getpid() /* Possibly recycled PID */
			    || (kill(p, 0) < 0 && errno == ESRCH))
			{
				close(fd);
				if (unlink(dotlock) == 0)
					errno=EAGAIN;
				else
					errno=EEXIST;
				return;
			}
		}
		else if (timeout > 0 && fstat(fd, &stat_buf) >= 0 &&
			 stat_buf.st_mtime < time(NULL) - timeout)
		{
			close(fd);

			if (unlink(dotlock) == 0)
				errno=EAGAIN;
			else
				errno=EEXIST;
			return;
		}

		close(fd);
	}

	errno=EEXIST;
}

static int ll_mail_open_do(struct ll_mail *p, int ro)
{
	char *dotlock;
	char myidbuf[IDBUFSIZE];
	int save_errno;
	int fd;

	getid(myidbuf);

	if (p->dotlock) /* Already locked */
	{
		fd=open(p->file, ro ? O_RDONLY:O_RDWR);

		if (fd >= 0 &&
		    (ll_lockfd(fd, ro ? ll_readlock:ll_writelock, 0, 0) < 0 ||
		     fcntl(fd, F_SETFD, FD_CLOEXEC) < 0))
		{
			close(fd);
			fd= -1;
		}
		return fd;
	}

	if ((dotlock=malloc(strlen(p->file)+sizeof(".lock"))) == NULL)
		return -1;

	strcat(strcpy(dotlock, p->file), ".lock");

	if (try_mail_dotlock(dotlock, myidbuf) == 0)
	{
		fd=open(p->file, ro ? O_RDONLY:O_RDWR);

		if (fd >= 0 &&
		    (ll_lockfd(fd, ro ? ll_readlock:ll_writelock, 0, 0) ||
		     fcntl(fd, F_SETFD, FD_CLOEXEC) < 0))
		{
			close(fd);
			fd= -1;
		}

		p->dotlock=dotlock;
		return fd;
	}

	save_errno=errno;

	/*
	** Last fallback: for EEXIST, a read-only lock should suffice
	** In all other instances, we'll fallback to read/write or read-only
	** flock as last resort.
	*/

	if ((errno == EEXIST && ro) || errno == EPERM || errno == EACCES)
	{
		fd=open(p->file, ro ? O_RDONLY:O_RDWR);

		if (fd >= 0)
		{
			if (ll_lockfd(fd, ro ? ll_readlock:ll_writelock,
				      0, 0) == 0 &&
			    fcntl(fd, F_SETFD, FD_CLOEXEC) == 0)
			{
				free(dotlock);
				return fd;
			}
			close(fd);
		}
	}

	/*
	** If try_dotlock blew up for anything other than EEXIST, we don't
	** know what the deal is, so punt.
	*/

	if (save_errno != EEXIST)
	{
		free(dotlock);
		return (-1);
	}

	dotlock_exists(dotlock, myidbuf, 300);
	free(dotlock);
	return (-1);
}

int ll_mail_open_ro(struct ll_mail *p)
{
	return ll_mail_open_do(p, 1);
}

int ll_mail_open(struct ll_mail *p)
{
	return ll_mail_open_do(p, 0);
}

void ll_mail_free(struct ll_mail *p)
{
	char myid[IDBUFSIZE];
	char idbuf[IDBUFSIZE];

	getid(myid);

	if (p->cclientfd >= 0)
	{
		if (lseek(p->cclientfd, 0L, SEEK_SET) == 0 &&
		    readid(idbuf, p->cclientfd) == 0 &&
		    strcmp(myid, idbuf) == 0)
		{
			if (ftruncate(p->cclientfd, 0) >= 0)
				unlink(p->cclientfile);
		}
		close(p->cclientfd);
		free(p->cclientfile);
	}

	if (p->dotlock)
	{
		int fd=open(p->dotlock, O_RDONLY);

		if (fd >= 0)
		{
			if (readid(idbuf, fd) == 0 &&
			    strcmp(myid, idbuf) == 0)
			{
				close(fd);
				unlink(p->dotlock);
				free(p->dotlock);
				free(p->file);
				free(p);
				return;
			}
			close(fd);
		}

		free(p->dotlock);
	}
	free(p->file);
	free(p);
}

int ll_dotlock(const char *dotlock, const char *tmpfile,
		int timeout)
{
	char myidbuf[IDBUFSIZE];

	getid(myidbuf);

	if (try_dotlock(tmpfile, dotlock, myidbuf))
	{
		if (errno == EEXIST)
			dotlock_exists(dotlock, myidbuf, timeout);
		return -1;
	}
	return 0;
}


