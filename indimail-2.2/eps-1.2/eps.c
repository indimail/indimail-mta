#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eps.h"

struct eps_t   *
eps_alloc(void)
{
	struct eps_t   *e = NULL;

	if(!(e = (struct eps_t *) mmalloc(sizeof(struct eps_t), "eps_alloc")))
		return NULL;
	memset((struct eps_t *) e, 0, sizeof(struct eps_t));
	return e;
}

struct eps_t   *
eps_begin(int interface, void *args)
{
	int             ret = 0, fd = 0;
	struct eps_t   *e = NULL;

	if(!(e = eps_alloc()))
		return NULL;
	e->interface = interface;
	/*
	 * Just allow EPS calls to be made
	 * (this can be risky as far as stability)
	 */
	if (interface == INTERFACE_NONE)
		return e;
	else
	if (interface & INTERFACE_STREAM)
	{
		if(!(ret = int_stream_init(e, (int *) args)))
		{
			eps_end(e);
			return NULL;
		}
		return e;
	} else
	if (interface & INTERFACE_BUFFER)
	{
		fd = -1;
		if(!(ret = int_stream_init(e, (int *) &fd)))
		{
			eps_end(e);
			return NULL;
		}
		if(!(ret = int_buffer_init(e, (struct line_t *) args)))
		{
			eps_end(e);
			return NULL;
		}
		return e;
	}
	return NULL;
}

void
eps_restart(struct eps_t *e, void *args)
{
	if (e->interface & INTERFACE_STREAM)
		int_stream_restart(e, (int *) args);
	else
	if (e->interface & INTERFACE_BUFFER)
		int_buffer_restart(e, (struct line_t *) args);
	e->content_type = CON_NONE;
	if (e->h)
	{
		header_kill(e->h);
		e->h = NULL;
	}
	if (e->b)
	{
		boundary_kill(e->b);
		e->b = NULL;
	}
}

struct header_t *
eps_next_header(struct eps_t *e)
{
	unsigned char  *l = NULL;

	if(!(l = (unsigned char *) unroll_next_line(e->u)))
		return NULL;
	e->h = header_parse(l);
	email_header_internal(e);
	return e->h;
}

unsigned char  *
eps_next_line(struct eps_t *e)
{
	int             ret = 0;
	unsigned char  *l = NULL;

	if (e->u->b->eof)
		return NULL;
	if(!(l = (unsigned char *) buffer_next_line(e->u->b)))
	{
		e->u->b->eof = 1;
		return NULL;
	}
	if (e->content_type & CON_MULTI)
	{
		if ((*l == '-') && (*(l + 1) == '-'))
		{
			if((ret = boundary_is(e, (char *) (l + 2))))
				return NULL;
		}
	}
	return l;
}

void
eps_header_free(struct eps_t *e)
{
	if (e->h == NULL)
		return;
	header_kill(e->h);
	e->h = NULL;
}

void
eps_end(struct eps_t *e)
{
	if (e->u)
		unroll_kill(e->u);
	if (e->b)
		boundary_kill(e->b);
	if (e->m)
		mime_kill(e->m);
	if (e->h)
		header_kill(e->h);
	mfree(e);
}

int
eps_is_eof(struct eps_t *e)
{
	if (e->u->b->eof)
		return 1;
	return 0;
}
