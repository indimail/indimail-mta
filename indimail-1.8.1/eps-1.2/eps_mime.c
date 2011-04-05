#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "eps.h"

struct i_header_t _m_i_headers[] = {
	{(unsigned char *) "Content-Type", mime_content_type},
	{(unsigned char *) "Content-Transfer-Encoding", mime_transfer_encoding},
	{(unsigned char *) "Content-Disposition", mime_content_disposition},
	{NULL, NULL}
};

/*
 * Allocate, and initialize a new mime_t
 * structure to return.
 */
struct mime_t  *
mime_new_instance(void)
{
	struct mime_t  *m = NULL;

	m = (struct mime_t *) mmalloc(sizeof(struct mime_t), "mime_new_instance");
	if (m == NULL)
		return NULL;

	memset((struct mime_t *) m, 0, sizeof(struct mime_t));

	m->content_type = CON_TEXT;
	m->encoding = ENC_TEXT;
	m->disposition = DIS_INLINE;

	return m;
}

int
mime_header(struct eps_t *eps, struct mime_t *m, struct header_t *h)
{
	int             i = 0;

	for (i = 0; _m_i_headers[i].name; i++)
	{
		if (!(strcasecmp((const char *) _m_i_headers[i].name, (const char *) h->name)))
		{
			_m_i_headers[i].func(eps, h, (void *) m);
			return 1;
		}
	}

	return 0;
}

void
mime_content_type(struct eps_t *eps, struct header_t *h, void *mx)
{
	char           *p = NULL;
	struct mime_t  *m = (struct mime_t *) mx;

	if ((!h) || (!h->atoms) || (!h->atoms->next) || (!h->atoms->next->data))
		m->content_type = CON_TEXT;

	else
		m->content_type = content_parse((char *) h->atoms->next->data, TYP_CON);

	if (!(m->filename))
	{
		p = (char *) header_fetch_atom(h, (unsigned char *) "name");
		if (p)
			m->filename = (char *) mstrdup((unsigned char *) p);
	}

	if (m->content_type & CON_MULTI)
	{
		p = (char *) header_fetch_atom(h, (unsigned char *) "boundary");
		if (p)
			boundary_add(eps, p);
	}
}

void
mime_transfer_encoding(struct eps_t *eps, struct header_t *h, void *mx)
{
	struct mime_t  *m = (struct mime_t *) mx;

	if ((!h) || (!h->atoms) || (!h->atoms->next) || (!h->atoms->next->data))
		m->encoding = ENC_TEXT;

	else
		m->encoding = content_parse((char *) h->atoms->next->data, TYP_ENC);
}

void
mime_content_disposition(struct eps_t *eps, struct header_t *h, void *mx)
{
	char           *p = NULL;
	struct mime_t  *m = (struct mime_t *) mx;

	if ((!h) || (!h->atoms) || (!h->atoms->next) || (!h->atoms->next->data))
		m->disposition = DIS_INLINE;

	else
		m->disposition = content_parse((char *) h->atoms->next->data, TYP_DIS);

	if (!(m->filename))
	{
		p = (char *) header_fetch_atom(h, (unsigned char *) "filename");
		if (p)
			m->filename = (char *) mstrdup((unsigned char *) p);
	}
}

void
mime_kill(struct mime_t *m)
{
	if (m->filename)
		mfree(m->filename);

	if (m->boundary)
		mfree(m->boundary);

	if (m->orig)
		mfree(m->orig);

	mfree(m);
}

int
mime_init_stream(struct eps_t *eps)
{
	char           *p = NULL;

	if (eps->m)
		mime_kill(eps->m);

	eps->u->eof = 0;
	eps->m = mime_new_instance();

	p = boundary_fetch(eps, eps->b->cdepth);
	if (p)
	{
		eps->m->boundary = (char *) mstrdup((unsigned char *) p);
		eps->m->depth = eps->b->cdepth;
	}

	else
		eps->m->depth = -1;

#ifdef MIME_DEBUG
	printf("New MIME: [%s](%d)\n", p ? p : "NONE", eps->m->depth);
#endif

	return 1;
}

struct header_t *
mime_next_header(struct eps_t *eps)
{
	unsigned char  *l = NULL;
	struct header_t *h = NULL;

	l = (unsigned char *) unroll_next_line(eps->u);
	if (l == NULL)
	{
#ifdef MIME_DEBUG
		printf("Unroll ends\n");
#endif
		return NULL;
	}

	h = header_parse(l);
	if (h)
	{
		if ((h->name) && (h->data))
			mime_header(eps, eps->m, h);
	}

	return h;
}

unsigned char  *
mime_next_line(struct eps_t *eps)
{
	int             ret = 0;
	unsigned char  *l = NULL;

	l = (unsigned char *) buffer_next_line(eps->u->b);
	if (l == NULL)
		return NULL;

	if ((*l == '-') && (*(l + 1) == '-'))
	{
		ret = boundary_is(eps, (char *) (l + 2));
		if (ret == 1)
		{
			if (eps->m->orig)
				mfree(eps->m->orig);

			eps->m->orig = (char *) mmalloc(strlen((const char *) l) + 1, "mime_next_line");
			if (eps->m->orig)
			{
				memset((char *) eps->m->orig, 0, strlen((const char *) l) + 1);
				memcpy((char *) eps->m->orig, (char *) l, strlen((const char *) l));
			}

			return NULL;
		}

		else
		if (ret == 2)
		{
#ifdef MIME_DEBUG
			printf("Boundary [%s] terminates; removing\n", eps->m->boundary);
#endif
			eps->m->depth = -1;

			boundary_remove_last(eps);

			if (eps->b->cdepth == 0)
			{
#ifdef MIME_DEBUG
				printf("Reached 0 depth: EOF\n");
#endif
				eps->u->b->eof = 1;
			}

			if (eps->m->orig)
				mfree(eps->m->orig);

			eps->m->orig = (char *) mmalloc(strlen((const char *) l) + 1, "mime_next_line");
			if (eps->m->orig)
			{
				memset((char *) eps->m->orig, 0, strlen((const char *) l) + 1);
				memcpy((char *) eps->m->orig, (char *) l, strlen((const char *) l));
			}

			return NULL;
		}
	}

	if (eps->m->orig)
	{
		mfree(eps->m->orig);
		eps->m->orig = NULL;
	}

	return l;
}
