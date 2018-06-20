#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eps.h"

int
int_buffer_init(struct eps_t *e, struct line_t *l)
{
	if (e->u->b->b == NULL)
		return 0;

	if (l == NULL)
		return 0;

	if (l->data == NULL)
		return 0;

	if (l->bytes == 0)
		return 0;

	mfree(e->u->b->b);

	e->u->b->fd = -1;

	e->u->b->b = l->data;
	e->u->b->blen = l->bytes;
	e->u->b->cin = l->bytes;
	e->u->b->bp = e->u->b->b;

	return 1;
}

void
int_buffer_restart(struct eps_t *e, struct line_t *l)
{
	unroll_restart(e->u, -1);

	e->u->b->fd = -1;

	e->u->b->b = l->data;
	e->u->b->blen = l->bytes;
	e->u->b->cin = l->bytes;
	e->u->b->bp = e->u->b->b;
}
