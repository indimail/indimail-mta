#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "eps_line.h"
#include "eps_buffer.h"
#include "eps_misc.h"

struct buffer_t *
buffer_alloc(void)
{
	int             ret = 0;
	struct buffer_t *b = NULL;

	if(!(b = (struct buffer_t *) mmalloc(sizeof(struct buffer_t), "buffer_alloc")))
		return NULL;
	memset((struct buffer_t *) b, 0, sizeof(struct buffer_t));
	if((b->l = line_alloc()) == NULL)
	{
		mfree(b);
		return NULL;
	}
	if(!(ret = line_init(b->l, NULL, 0)))
	{
		line_kill(b->l);
		mfree(b);
		return NULL;
	}
	return b;
}

struct buffer_t *
buffer_init(int fd, unsigned long blen)
{
	struct buffer_t *b = NULL;

	/*
	 * Need at least 2 bytes
	 */
	if (blen < 2)
		return NULL;
	if(!(b = buffer_alloc()))
		return NULL;
	if(!(b->b = (char *) mmalloc(blen + 1, "buffer_init")))
	{
		buffer_kill(b);
		return NULL;
	}
	b->fd = fd;
	b->blen = blen;
	memset((char *) b->b, 0, blen + 1);
	return b;
}

void
buffer_restart(struct buffer_t *b, int fd)
{
	b->fd = fd;
	b->eof = 0;
	b->cin = 0;
	b->restart = 0;
	b->ulen = 0;
	b->clen = 0;
	b->fbrk = 0;

	line_restart(b->l);
	if (b->fd == -1)
		b->bp = b->b;
	else
	{
		b->bp = NULL;
		memset((char *) b->b, 0, b->blen);
	}
}

void
buffer_kill(struct buffer_t *b)
{
	if (b->l)
		line_kill(b->l);
	if ((b->b) && (b->fd != -1))
		mfree(b->b);
	mfree(b);
}

char           *
buffer_next_line(struct buffer_t *bb)
{
	int             ret = 0;
	char           *p = NULL, *t = NULL, *bp = NULL, fn = 0;

	if (bb->eof)
		return NULL;
	/*
	 * Call line_restart?
	 */
	if (bb->restart)
	{
#ifdef DEBUG
		printf("restarting\n");
#endif
		line_restart(bb->l);
		bb->restart = 0;
	}
	/*
	 * Buffer is empty
	 */
	if (bb->cin == 0)
	{
		if (bb->fd != -1)
		{
			ret = read(bb->fd, bb->b, (bb->blen - 1));
			if (!ret)
			{
#ifdef DEBUG
				printf("read returns NULL %lu[%s]\n", bb->l->bytes, bb->l->data);
#endif
				bb->eof = 1;
				return NULL;
			}

			else
				bb->cin = ret;
		} else
		{
			bb->eof = 1;
			return NULL;
		}
	}
#ifdef DEBUG
	printf("BUFFER: [%s] RESTART:%d\n", bb->b, bb->restart);
#endif
	while (1)
	{
		fn = 0;
		if (bb->fd == -1)
			bp = p = t = bb->bp;
		else
			bp = p = t = bb->b;
		for (bb->clen = bb->ulen = 0;; p++)
		{
			if (*p == '\0')
			{
				p = NULL;
				break;
			} else
			if (*p == '\n')
			{
				if (bb->ulen && *(p - 1) == '\r')
					bb->ulen--;
				fn = 1;
				bb->clen++;
				break;
			} else
			{
				bb->ulen++;
				bb->clen++;
			}
		}
#ifdef DEBUG
		printf("EXAMINING: [%s]\n", bp);
#endif
		/*
		 * No newline found.
		 */
		if (!fn)
		{
#ifdef DEBUG
			printf("No newline\n");
			printf("Injecting from newline: [");
			line_print(bp, bb->ulen);
			printf("]\n");
#endif
			ret = line_inject(bb->l, bp, bb->ulen);
			if (!ret)
				return NULL;
			if (bb->fd == -1)
				bb->bp += bb->clen;
			bb->fn = 1;
			bb->cin = 0;
			bb->restart = 0;
			bb->ulen = 0;
			bb->clen = 0;
			if (bb->fd != -1)
				memset((char *) bb->b, 0, bb->blen);
			return buffer_next_line(bb);
		}
#ifdef DEBUG
		printf("Injecting from base: [");
		line_print(bp, bb->ulen);
		printf("]\n");
#endif
		if (bb->ulen)
		{
			ret = line_inject(bb->l, bp, bb->ulen);
			if (!ret)
				return NULL;
		}
		bb->restart = 1;
		if (bb->fd != -1)
			memcpy((char *) bb->b, (char *) (bb->b + bb->clen), (bb->blen - bb->clen));
		else
			bb->bp += bb->clen;
		bb->cin -= bb->clen;
		if (bb->fd != -1)
			memset((char *) (bb->b + bb->cin + 1), 0, (bb->blen - bb->cin));
		bb->ulen = 0;
		bb->clen = 0;
#ifdef DEBUG
		printf("bottom returns line\n");
#endif
		return bb->l->data;
	}
	return NULL;
}
