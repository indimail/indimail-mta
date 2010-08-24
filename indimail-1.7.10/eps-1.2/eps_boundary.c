#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eps.h"

struct boundary_t *
boundary_alloc(void)
{
	struct boundary_t *b = NULL;

	b = (struct boundary_t *) mmalloc(sizeof(struct boundary_t), "boundary_alloc");
	if (b == NULL)
		return NULL;

	memset((struct boundary_t *) b, 0, sizeof(struct boundary_t));

	b->boundaries = member_alloc();
	if (b->boundaries == NULL)
	{
		boundary_kill(b);
		return NULL;
	}

	b->last = NULL;
	return b;
}

struct member_t *
member_alloc(void)
{
	struct member_t *m = NULL;

	m = (struct member_t *) mmalloc(sizeof(struct boundary_t), "member_alloc");
	if (m == NULL)
		return NULL;

	memset((struct member_t *) m, 0, sizeof(struct member_t));
	return m;
}

int
boundary_add(struct eps_t *e, char *boundary)
{
	struct member_t *m = NULL, *ms = NULL;

	if (e->b == NULL)
		return 0;

	m = member_alloc();
	if (m == NULL)
		return 0;

	m->boundary = (char *) mstrdup((unsigned char *) boundary);
	if (m->boundary == NULL)
	{
		member_kill(e, m);
		return 0;
	}

	for (ms = e->b->boundaries; ms->next; ms = ms->next);

	ms->next = m;

	m->next = NULL;
	m->depth = (e->b->cdepth + 1);

	e->b->cdepth++;
	e->b->last = m;

	return 1;
}

void
boundary_kill(struct boundary_t *b)
{
	struct member_t *m = NULL, *om = NULL;

	if (b->boundaries)
	{
		m = b->boundaries;

		while (m->next)
		{
			om = m->next;
			m->next = m->next->next;

			if (om->boundary)
				mfree(om->boundary);

			mfree(om);
		}

		mfree(b->boundaries);
	}

	mfree(b);
}

void
member_kill(struct eps_t *e, struct member_t *m)
{
	struct member_t *ms = NULL;

	if (m->boundary)
		mfree(m->boundary);

	for (ms = e->b->boundaries; ms->next; ms = ms->next)
	{
		if (ms->next == m)
		{
			ms->next = m->next;
			e->b->cdepth--;
			break;
		}
	}

	if (e->b->last == m)
		e->b->last = NULL;

	mfree(m);
}

int
boundary_is(struct eps_t *e, char *boundary)
{
	char           *p = NULL;
	struct member_t *m = NULL;
	unsigned long   len = 0, blen = 0;

	if (e->b == NULL)
		return 0;

	blen = strlen(boundary);

	for (m = e->b->boundaries; m->next; m = m->next)
	{
		if (!(strcasecmp(m->next->boundary, boundary)))
		{
			if (m->next->depth != e->b->cdepth)
				return 0;

			e->b->last = m->next;

#ifdef MIME_DEBUG
			printf("boundary_is[%s]: 1(%d)\n", boundary, m->next->depth);
#endif
			return 1;
		}

		len = strlen(m->next->boundary);
		if (blen == (len + 2))
		{
			p = (boundary + len);
			if ((*p == '-') && (*(p + 1) == '-'))
			{
				if (!(strncasecmp(m->next->boundary, boundary, len)))
				{
#ifdef MIME_DEBUG
					printf("boundary_is[%s]: 2(%d)\n", boundary, m->next->depth);
#endif
					e->b->last = m->next;
					return 2;
				}
			}
		}
	}

	return 0;
}

int
boundary_remove_last(struct eps_t *e)
{
	if (e->b->last == NULL)
		return 0;

	member_kill(e, e->b->last);
	return 1;
}

char           *
boundary_fetch(struct eps_t *e, char depth)
{
	struct member_t *b = NULL;

	if (e->b == NULL)
		return NULL;

	if (e->b->boundaries == NULL)
		return NULL;

	if ((depth > e->b->cdepth) || (depth < 1))
		return NULL;

	for (b = e->b->boundaries; b->next; b = b->next)
	{
		if (b->next->depth == depth)
		{
			e->b->last = b->next;
			return b->next->boundary;
		}
	}

	return NULL;
}

void
boundary_debug(struct eps_t *e)
{
	struct member_t *m = NULL;

	printf("BOUNDARY_DEBUG:\nCurrent depth: %d\n", e->b->cdepth);
	printf("  Current boundaries:\n");

	for (m = e->b->boundaries; m->next; m = m->next)
		printf("    [%s](Depth: %d)\n", m->next->boundary, m->next->depth);
}
