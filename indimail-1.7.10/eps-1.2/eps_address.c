#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "eps.h"

struct group_t *
address_evaluate(char *data)
{
	struct group_t *g = NULL;
	char           *d = NULL, *h = NULL, *t = NULL;
	struct address_t *alist = NULL, *a = NULL, *a_tail = NULL;

	if(!(g = (struct group_t *) mmalloc(sizeof(struct group_t), "address_evaluate")))
		return NULL;
	if(!(alist = (struct address_t *) mmalloc(sizeof(struct address_t), "address_evaluate")))
	{
		mfree(g);
		return NULL;
	}
	a_tail = alist;
	alist->next = NULL;
	g->group = NULL;
	g->members = a_tail;
	g->nmembers = 0;
	/*
	 * address_evaluate_one mangles data
	 */
	if(!(d = (char *) mstrdup((unsigned char *) data)))
	{
		mfree(g);
		mfree(alist);
		return NULL;
	}
	t = d;
	h = (char *) rfc2822_next_token((unsigned char *) d, ':', (unsigned char *) "<>;");
	if (*h == ':')
	{
		*h++ = '\0';
		g->group = (char *) mstrdup((unsigned char *) t);

		while ((*h == ' ') || (*h == '\t'))
			h++;
		t = h;
	} else
		h = t = d;
	while (1)
	{
		h = (char *) rfc2822_next_token((unsigned char *) t, ',', (unsigned char *) NULL);
		if (!(*h))
			break;
		*h = '\0';
		a = address_evaluate_one(t);
		if (!a)
			break;
		a_tail->next = a;
		a->next = NULL;
		a_tail = a;
		g->nmembers++;
		t = (h + 1);
	}
	if (*t)
	{
		if((a = address_evaluate_one(t)))
		{
			a_tail->next = a;
			a->next = NULL;
			a_tail = a;
			g->nmembers++;
		}
	}
	if (g->group)
	{
		h = (char *) rfc2822_convert_literals((unsigned char *) g->group);
		mfree(g->group);
		g->group = h;
	}
	mfree(d);
	return g;
}

struct address_t *
address_evaluate_one(char *data)
{
	struct address_t *a = NULL;
	char           *p = NULL, *n = NULL, *u = NULL, *h = NULL, *d = NULL;

	n = u = d = NULL;
	if(!(a = (struct address_t *) mmalloc(sizeof(struct address_t), "address_evaluate_one")))
		return NULL;
	memset((struct address_t *) a, 0, sizeof(struct address_t));
	p = data;
	/*- Name/User */
	h = (char *) rfc2822_next_token((unsigned char *) data, '<', (unsigned char *) NULL);
	if (*h == '<')
	{
		*h++ = '\0';
		if (*p)
			n = data;
		if (*h)
			u = h;
		else
			return a;
		for (p = (h - 2); ((*p == ' ') || (*p == '\t')); p--);
		*(++p) = '\0';
		p = h;
		if ((n) && (*n))
			a->name = (char *) mstrdup((unsigned char *) n);
	}
	/*- User/Domain */
	h = (char *) rfc2822_next_token((unsigned char *) p, '@', (unsigned char *) ">");
	if (*h == '@')
	{
		*h++ = '\0';
		if (!(*h))
			return a;
		if (!u)
		{
			while ((*p == ' ') || (*p == '\t'))
				p++;
			u = p;
		}
		d = p = h;
	} else
		return a;
	a->user = (char *) mstrdup((unsigned char *) u);
	/*- End */
	h = (char *) rfc2822_next_token((unsigned char *) p, '>', (unsigned char *) " ");
	if (*h == '>')
		*h = '\0';
	a->domain = (char *) mstrdup((unsigned char *) d);
	address_fixup(a);
	return a;
}

void
address_kill(struct group_t *g)
{
	struct address_t *a = NULL, *ao = NULL;

	if (g->members)
	{
		a = g->members;
		while (a->next)
		{
			ao = a->next;
			a->next = a->next->next;

			address_kill_one(ao);
		}
		mfree(g->members);
	}
	mfree(g);
}

void
address_kill_one(struct address_t *a)
{
	if (!a)
		return;
	if (a->name)
		mfree(a->name);
	if (a->user)
		mfree(a->user);
	if (a->domain)
		mfree(a->domain);
	mfree(a);
}

void
address_fixup(struct address_t *a)
{
	char           *p = NULL;

	if (!a)
		return;
	if (a->name)
	{
		p = (char *) rfc2822_convert_literals((unsigned char *) a->name);
		mfree(a->name);
		a->name = p;
	}
}
