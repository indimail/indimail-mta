/*
** Copyright 2002 Double Precision, Inc.
** See COPYING for distribution information.
*/
#include	"config.h"
#include	"numlib/numlib.h"
#include	"liblock/config.h"
#include	"liblock/liblock.h"
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<ctype.h>

#if HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#if HAVE_FCNTL_H
#include	<fcntl.h>
#endif

#include	"tlscache.h"


/*
** The cache file begins with the following record:
*/

struct hdr {
	off_t filesize;		/* Size of the file, don't trust fstat */
	off_t head, tail;	/* Head and tail ptrs */
	off_t first;		/* See below */
};

#ifndef TLSCACHEMINSIZE
#define TLSCACHEMINSIZE 16384
#endif

#define BORK(p) BORK2(__LINE__, (p))

static void BORK2(int l, const char *p)
{
	fprintf(stderr, "ALERT: tlscache.c(%d): corruption detected in %s\n",
		(l), (p));
}

/*
** Cached SSL session objects are written starting at the end of the file
** the file, growing to the beginning of the file.  The head pointer
** points to the most recently added cached object.  When the beginning of
** the file is reached, it's wrapped around.
**
** After the wraparound, old SSL session objects are freed. starting with
** the tail ptr, to make room for newer object.  There may be unused space
** between struct hdr, and the first cached object, the 'first' pointer
** helps me find the first cached object, when searching.
** When searching for a cached object, begin at the head ptr (most recent,
** and continue until we find an object referenced by the tail ptr).
**
** Each cached object carries the following header.
*/

struct obj {
	size_t prev_size;	/* Size of the previous cached object,
				** this must be the first member of this
				** struct.
				*/

	size_t my_size;		/* Size of this cached object */
};

/* Read cnt number of bytes, or else */

static int my_read(int fd, void *buffer, size_t cnt)
{
	char *p=(char *)buffer;

	while (cnt > 0)
	{
		int n=read(fd, p, cnt);

		if (n <= 0)
			return n;
		p += n;
		cnt -= n;
	}
	return 1;
}

/* Write cnt number of bytes, or else */

static int my_write(int fd, const void *buffer, size_t cnt)
{
	const char *p=(const char *)buffer;

	while (cnt > 0)
	{
		int n=write(fd, p, cnt);

		if (n <= 0)
			return -1;
		p += n;
		cnt -= n;
	}
	return 0;
}

static int init(struct CACHE *, off_t s);

void tls_cache_close(struct CACHE *p)
{
	if (p->filename != NULL)
		free(p->filename);
	if (p->fd >= 0)
		close(p->fd);
	free(p);
}

/*
** Open a cache file, creating one if necessary
*/

struct CACHE *tls_cache_open(const char *filename, off_t req_size)
{
	struct CACHE *p=malloc(sizeof(struct CACHE));
	struct hdr h;
	int rc;

	if (!p) return NULL;

	if ((p->fd=open(filename, O_RDWR|O_CREAT, 0600)) < 0)
	{
		free(p);
		return NULL;
	}

	if ((p->filename=strdup(filename)) == NULL)
	{
		close(p->fd);
		free(p);
		return (NULL);
	}

	rc=my_read(p->fd, &h, sizeof(h));

	if (rc < 0)
	{
		tls_cache_close(p);
		return (NULL);
	}

	if (rc == 0 || h.filesize == 0)
	{
		/* Once again, but this time lock it */

		if (ll_lock_ex(p->fd) < 0 ||
		    lseek(p->fd, 0, SEEK_SET) < 0)
		{
			tls_cache_close(p);
			return (NULL);
		}

		rc=my_read(p->fd, &h, sizeof(h));

		if (rc < 0)
		{
			tls_cache_close(p);
			return (NULL);
		}

		if (rc == 0 || h.filesize == 0)
		{
			if (init(p, req_size))
			{
				tls_cache_close(p);
				return (NULL);
			}
		}
		ll_unlock_ex(p->fd);
	}
	return p;
}

static int doadd(struct CACHE *p, const char *val, size_t vallen);

int tls_cache_add(struct CACHE *p, const char *val, size_t vallen)
{
	int rc;

	if (p->fd < 0)
		return (0);	/* Previous error invalidated obj */

	if (ll_lock_ex(p->fd) < 0)
	{
		close(p->fd);
		p->fd= -1;
		return (-1);
	}

	rc=doadd(p, val, vallen);

	if (rc < 0 && p->fd >= 0)
	{
		close(p->fd);
		p->fd= -1;
		unlink(p->filename);	/* Blow it away, something's wrong */
		perror("ALERT: tlscache.c: ");
		fprintf(stderr, "ALERT: tlscache.c: removing %s\n",
			p->filename);
	}

	if (p->fd >= 0 && ll_unlock_ex(p->fd) < 0)
	{
		close(p->fd);
		p->fd= -1;
		rc= -1;
	}

	if (rc != 0)
		rc= -1;

	return rc;
}

/*
** Read the header, and do a simple sanity check
*/

static int readhdr(struct CACHE *p, struct hdr *h)
{
	if (lseek(p->fd, 0, SEEK_SET) < 0 ||
	    my_read(p->fd, h, sizeof(*h)) <= 0)
	{
		BORK(p->filename);
		return (-1);
	}

	if (h->filesize < TLSCACHEMINSIZE || h->head >= h->filesize ||
	    h->tail >= h->filesize || h->first >= h->filesize ||
	    h->head < 0 || h->tail < 0 || h->first < 0 ||
	    h->first > h->head || h->first > h->tail)
	{
		BORK(p->filename);
		return (-1);	/* Sanity check */
	}
	return (0);
}

static int writehdr(struct CACHE *p, const struct hdr *h)
{
	if (lseek(p->fd, 0, SEEK_SET) < 0 ||
	    my_write(p->fd, h, sizeof(*h)) < 0)
	{
		BORK(p->filename);
		return -1;
	}
	return 0;
}

/*
** Read the header of a cached object, and do a sanity check
*/

static int readobj(struct CACHE *p, off_t w,
		   const struct hdr *h, struct obj *o)
{
	if (lseek(p->fd, w, SEEK_SET) < 0 ||
	    my_read(p->fd, o, sizeof(*o)) <= 0)
	{
		BORK(p->filename);
		return (-1);
	}

	if (o->prev_size < sizeof(*o) || o->my_size < sizeof(*o) ||
	    o->prev_size >= h->filesize || o->my_size >= h->filesize ||
	    h->filesize - w < o->my_size)
	{
		BORK(p->filename);
		errno=EIO;
		return (-1);	/* Sanity check */
	}
	return 0;
}


static int doadd(struct CACHE *p, const char *val, size_t vallen)
{
	struct hdr h;
	struct obj o;
	int timer=0;
	int first=0;
	char *buf;

	if (readhdr(p, &h))
		return -1;

	/* Keep trying to allocate sufficient space in the cache file */

	for (;;)
	{
		if (++timer > 100)
		{
			BORK(p->filename);
			errno=EIO;
			return (-1);	/* Sanity check */
		}

		if (h.head == 0 && h.tail == 0)	/* First time */
		{
			if (vallen + sizeof(struct obj) +
			    sizeof(struct hdr) > h.filesize)
			{
				errno=ENOSPC;
				return 1;
			}

			/* First cached object goes at the end */

			h.head=h.tail=h.filesize - vallen - sizeof(struct obj);
			first=1;
			break;
		}

		if (h.head <= h.tail)	/* Not wrapped around */
		{
			if (h.head >= sizeof(struct hdr) +
			    sizeof(struct obj) + vallen)
			{
				h.head -= sizeof(struct obj) + vallen;
				break;
				/* Room earlier in the file */
			}

			/*
			** No room before, we must now wrap around.  Find
			** where the last object ends, and see if there's
			** enough room between the end of the last object,
			** and the end of the file, to save the new object.
			*/

			if (readobj(p, h.tail, &h, &o) < 0)
				return -1;

			if (h.filesize - h.tail - o.my_size >=
			    sizeof(struct obj) + vallen)
			{
				h.first=h.head;
				h.head=h.filesize - vallen -
					sizeof(struct obj);
				/* Room to wrap around */

				break;
			}
		}
		else	/* We're currently wrapped around, so all the free
			** space is from tail to head.
			*/
		{
			if (readobj(p, h.tail, &h, &o) < 0)
				return -1;

			if (h.head >= h.tail + o.my_size +
			    sizeof(struct obj) + vallen)
			{
				h.head -= sizeof(struct obj) + vallen;
				break;
			}
		}

		if (h.head == h.tail)	/* Sanity check */
		{
			errno=ENOSPC;
			return 1;
		}

		/* Pop one off tail */

		if (readobj(p, h.tail, &h, &o))
			return -1;

		if (sizeof(h) + o.prev_size <= h.tail)
		{
			h.tail -= o.prev_size;

			if (writehdr(p, &h))
				return (-1);
			continue;
		}

		if (h.tail != h.first)
		{
			BORK(p->filename);
			errno=EIO;
			return (-1);	/* Sanity check */
		}

		h.first=0;
		h.tail=h.filesize - o.prev_size;

		if (h.tail < h.first)
		{
			BORK(p->filename);
			errno=EIO;
			return (-1);	/* Sanity check */
		}

		if (writehdr(p, &h))
			return (-1);
	}

	buf=malloc(vallen + sizeof(o) + sizeof(o.prev_size));

	if (!buf)
		return (1);

	o.prev_size=0;
	o.my_size=vallen + sizeof(o);
	memcpy(buf, &o, sizeof(o));
	memcpy(buf + sizeof(o), val, vallen);
	o.prev_size=o.my_size;
	memcpy(buf + sizeof(o) + vallen, &o.prev_size, sizeof(o.prev_size));

	if (lseek(p->fd, h.head, SEEK_SET) < 0)
		return (-1);

	if (h.head + sizeof(o) + vallen < h.filesize)
	{
		if (my_write(p->fd, buf, sizeof(o)+vallen+sizeof(o.prev_size)) < 0)
			return -1;
	}
	else
	{
		if (my_write(p->fd, buf, sizeof(o)+vallen) < 0 ||
		    (!first && (lseek(p->fd, h.first, SEEK_SET) < 0 ||
				my_write(p->fd, &o.prev_size,
					 sizeof(o.prev_size)) < 0)))
			return -1;
	}
				
	return writehdr(p, &h);
}

static int init(struct CACHE *p, off_t size)
{
	char buffer[BUFSIZ];
	off_t c;
	struct hdr h;

	if (size < TLSCACHEMINSIZE)
	{
		errno=EINVAL;
		return -1;
	}

	if (lseek(p->fd, 0, SEEK_SET) < 0)
		return -1;

	memset(buffer, 0, sizeof(buffer));

	c=size;

	while (c > 0)
	{
		int i;

		off_t n=c;

		if (n > sizeof(buffer))
			n=sizeof(buffer);

		i=write(p->fd, buffer, n);

		if (i <= 0)
			return -1;

		c -= i;
	}

	memset(&h, 0, sizeof(h));
	h.filesize=size;

	if (lseek(p->fd, 0, SEEK_SET) < 0 ||
	    my_write(p->fd, &h, sizeof(h)))
		return (-1);
	return (0);
}

static int dowalk(struct CACHE *cache,
		  int (*walk_func)(void *rec, size_t recsize,
				   int *doupdate,
				   void *arg),
		  void *arg);

int tls_cache_walk(struct CACHE *p,
		   int (*walk_func)(void *rec, size_t recsize,
				    int *doupdate,
				    void *arg),
		   void *arg)
{
	int rc;

	if (p->fd < 0)
		return (0);	/* Previous error invalidated obj */

	if (ll_lockfd(p->fd, ll_readlock||ll_whence_start|ll_wait, 0, 0) < 0)
	{
		/* Some locking methods don't support readonly locks */

		if (ll_lock_ex(p->fd) < 0)
		{
			close(p->fd);
			p->fd= -1;
			return (-1);
		}
	}

	rc=dowalk(p, walk_func, arg);

	if (rc < 0 && p->fd >= 0)
	{
		close(p->fd);
		p->fd= -1;
		unlink(p->filename);
		perror("ALERT: tlscache.c: ");
		fprintf(stderr, "ALERT: tlscache.c: removing %s\n",
			p->filename);
	}

	if (p->fd >= 0 && ll_unlock_ex(p->fd) < 0)
	{
		close(p->fd);
		p->fd= -1;
		rc= -1;
	}

	return rc;
}

/* Buffered reads when searching, for speed */

struct walkbuf {
	char buffer[BUFSIZ];
	char *bufptr;
	int left;
};

static int buf_read(int fd, struct walkbuf *w,
		    const void *buffer, size_t cnt)
{
	char *p=(char *)buffer;

	while (cnt > 0)
	{
		if (w->left <= 0)
		{
			w->left=read(fd, w->buffer, sizeof(w->buffer));

			if (w->left <= 0)
				return -1;
			w->bufptr=w->buffer;
		}

		*p++ = *w->bufptr++;
		--w->left;
		--cnt;
	}
	return 1;
}

static int dowalk(struct CACHE *p,
		  int (*walk_func)(void *rec, size_t recsize,
				   int *doupdate, void *arg),
		  void *arg)
{
	struct hdr h;
	struct obj o;
	char *buf=NULL;
	size_t bufsize=0;
	int rc;
	int counter;
	struct walkbuf wb;
	int updateflag;

	off_t pos;
	off_t lastpos;

	if (readhdr(p, &h))
		return -1;

	if (h.head == 0 && h.tail == 0)	/* First time */
		return (0);

	pos=h.head;
	if (lseek(p->fd, pos, SEEK_SET) < 0)
		return (-1);

	counter=0;
	wb.left=0;
	for (;;)
	{
		if (++counter > h.filesize / sizeof(o))
		{
			BORK(p->filename);
			return (-1);
		}

		if (h.filesize - pos < sizeof(o))
		{
			BORK(p->filename);
			errno=EIO;
			if (buf)
				free(buf);
			return (-1);	/* Sanity check */
		}

		if (buf_read(p->fd, &wb, &o, sizeof(o)) <= 0)
		{
			if (buf)
				free(buf);
			return (-1);
		}

		if (h.filesize - pos < o.my_size || o.my_size < sizeof(o))
		{
			BORK(p->filename);
			errno=EIO;
			if (buf)
				free(buf);
			return (-1);	/* Sanity check */
		}

		if (buf == NULL || bufsize < o.my_size - sizeof(o)+1)
		{
			char *newbuf;

			bufsize=o.my_size - sizeof(o)+1;

			if ((newbuf=buf ? realloc(buf, bufsize):
			     malloc(bufsize)) == NULL)
			{
				free(buf);
				return (-1);
			}
			buf=newbuf;
		}

		if (buf_read(p->fd, &wb, buf, o.my_size - sizeof(o)) <= 0)
		{
			free(buf);
			return (-1);
		}

		updateflag=0;

		rc= (*walk_func)(buf, o.my_size - sizeof(o), &updateflag, arg);

		if (updateflag && rc >= 0)
		{
			if (lseek(p->fd, pos + sizeof(o), SEEK_SET) < 0 ||
			    my_write(p->fd, buf, o.my_size - sizeof(o)) < 0)
			{
				free(buf);
				return (-1);
			}
			wb.left=0;
		}

		if (rc != 0)
		{
			free(buf);
			if (rc < 0)
				rc=1;
			return (rc);
		}

		if (pos == h.tail)
			break;

		lastpos=pos;
		pos += o.my_size;

		if (pos < h.filesize)
		{
			if (lastpos < h.tail && pos > h.tail)
			{
				BORK(p->filename);
				errno=EIO;
				free(buf);
				return (-1);
			}
		}
		else
		{
			pos=h.first;
			if (h.first < sizeof(h))
			{
				BORK(p->filename);
				free(buf);
				errno=EIO;
				return (-1);
			}

			if (lseek(p->fd, pos, SEEK_SET) < 0)
			{
				free(buf);
				return (-1);
			}
			wb.left=0;
		}
	}
	if (buf)
		free(buf);
	return (0);
}
