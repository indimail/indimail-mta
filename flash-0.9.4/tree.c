/*
 * $Log: tree.c,v $
 * Revision 1.3  2011-07-29 09:24:22+05:30  Cprogrammer
 * fixed gcc warnings
 *
 * Revision 1.2  2008-06-09 15:33:15+05:30  Cprogrammer
 * added GPL copyright notice
 *
 * Revision 1.1  2002-12-16 01:55:22+05:30  Manny
 * Initial revision
 *
 *
 * Generalised Binary Search Tree (BST) routines. 
 *
 * Copyright (C) Stephen Fegan 1995, 1996
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 * 
 * please send patches or advice on flash to: `flash@netsoc.ucd.ie'
 */

#include <stdlib.h>
#include <stdio.h>

#include"tree.h"
#include"misc.h"

struct node_stack
{
	struct node    *n;
	struct node_stack *next;
};

static struct node *
new_node(void *data)
{
	struct node    *n;

	n = xmalloc(sizeof(*n));

	n->l = n->r = NULL;
	n->data = data;
	n->next = NULL;

	return n;
}

struct dict    *
new_dict(int    (*compare) (void *a, void *b))
{
	struct dict    *d;
	struct node    *head;

	d = xmalloc(sizeof(*d));

	head = new_node(NULL);

	d->head = head;
	d->order = O_NONE;
	d->compare = compare;

	return d;
}

void
free_dict(struct dict *dict)
{
	struct node    *head, *x;

	head = dict->head;
	while (head != NULL)
	{
		x = head->next;
		free(head);
		head = x;
	}

	free(dict);
}

void           *
find_node(struct dict *d, void *k)
{
	struct node    *x = d->head->r;
	int             (*compare) (void *a, void *b) = d->compare;
	int             kcmp;

	while ((x != NULL) && ((kcmp = (*compare) (x->data, k)) != 0))
		x = kcmp > 0 ? x->l : x->r;

	if (x == NULL)
		return NULL;
	else
		return x->data;
}

int
add_unique_node(struct dict *d, void *data)
{
	struct node    *head = d->head;
	struct node    *p = head, *x = head->r;
	int             (*compare) (void *a, void *b) = d->compare;
	int             kcmp;

	while (x != NULL)
	{
		p = x;
		kcmp = (*compare) (x->data, data);
		if (kcmp == 0)
			break;
		x = kcmp > 0 ? x->l : x->r;
	}

	if (x != NULL)
		return 0;

	x = new_node(data);

	if ((p != head) && ((*compare) (p->data, data) > 0))
		p->l = x;
	else
		p->r = x;

	x->next = head->next;
	head->next = x;
	d->order = O_NONE;

	return 1;
}

void
add_node(struct dict *d, void *data)
{
	struct node    *head = d->head;
	struct node    *p = head, *x = head->r;
	int             (*compare) (void *a, void *b) = d->compare;
	int             kcmp;

	while (x != NULL)
	{
		p = x;
		kcmp = (*compare) (x->data, data);
		x = kcmp > 0 ? x->l : x->r;
	}

	x = new_node(data);

	if ((p != head) && ((*compare) (p->data, data) > 0))
		p->l = x;
	else
		p->r = x;

	x->next = head->next;
	head->next = x;
	d->order = O_NONE;

	return;
}

void           *
change_node(struct dict *d, void *data)
{
	struct node    *head = d->head;
	struct node    *x = head->r;
	void           *old_data;
	int             (*compare) (void *a, void *b) = d->compare;
	int             kcmp;

	while (x != NULL)
	{
		kcmp = (*compare) (x->data, data);
		if (kcmp == 0)
			break;
		x = kcmp > 0 ? x->l : x->r;
	}

	if (x == NULL)
		return NULL;

	old_data = x->data;
	x->data = data;

	return old_data;
}

void           *
delete_node(struct dict *d, void *data)
{
	struct node    *head = d->head;
	struct node    *p = head, *x = head->r, *t, *c;
	void           *old_data;
	int             (*compare) (void *a, void *b) = d->compare;
	int             kcmp;

	while ((x != NULL) && ((kcmp = (*compare) (x->data, data)) != 0))
	{
		p = x;
		x = kcmp > 0 ? x->l : x->r;
	}

	t = x;

	if (t->r == NULL)
		x = x->l;
	else
	if (t->r->l == NULL)
	{
		x = x->r;
		x->l = t->l;
	} else
	{
		c = x->r;
		while (c->l->l != NULL)
			c = c->l;
		x = c->l;
		c->l = x->r;
		x->l = t->l;
		x->r = t->r;
	}

	old_data = t->data;

	if ((p != head) && ((*compare) (p->data, data) > 0))
		p->l = x;
	else
		p->r = x;

	p = head;
	x = head->next;
	while (x != t)
	{
		p = x;
		x = x->next;
	}

	p->next = t->next;
	d->order = O_NONE;

	free(t);

	return (old_data);
}

#define ns_push(x) \
{\
   struct node_stack *ns_x;\
\
   if(ns_empties!=NULL)\
   {\
      ns_x=ns_empties;\
      ns_empties=ns_empties->next;\
  }\
   else\
   {\
      ns_x=(struct node_stack *)xmalloc(sizeof(*ns_x));\
  }\
   ns_x->next=ns_head;\
   ns_head=ns_x;\
   ns_head->n=x;\
}

#define ns_pop(x)\
if(ns_head!=NULL)\
{\
   struct node_stack *ns_x;\
\
   x=ns_head->n;\
   ns_x=ns_head;\
   ns_head=ns_head->next;\
   ns_x->next=ns_empties;\
   ns_empties=ns_x;\
}\
else x=NULL;

#define add_link(x)\
link->next=x;\
link=link->next;

void
relink_inorder(struct dict *d)
{
	struct node    *head = d->head;
	struct node    *link = head, *x = head->r;
	struct node_stack *ns_head = NULL;
	struct node_stack *ns_empties = NULL;

	if (d->order == O_IN)
		return;

	while (x != NULL)
	{
		while (x->l != NULL)
		{
			ns_push(x);
			x = x->l;
		}

		add_link(x);

		while (x->r == NULL)
		{
			ns_pop(x);
			if (x == NULL)
				break;;
			add_link(x);
		}

		if (x != NULL)
			x = x->r;
	}

	add_link(NULL);
	d->order = O_IN;

	if (ns_head != NULL)
	{
		fprintf(stderr, "Node Stack error !!\n");
		exit(EXIT_FAILURE);
	}

	while (ns_empties)
	{
		ns_head = ns_empties->next;
		free(ns_empties);
		ns_empties = ns_head;
	}

	return;
}

void
visit_nodes(struct dict *d, void (*visit) (void *data))
{
	struct node    *n = d->head->next;

	while (n != NULL)
	{
		(*visit) (n->data);
		n = n->next;
	}
}

#define median_divide(x)\
{\
   struct node *y,*m,*mp;\
\
   y=m=mp=x;\
\
   while((y)&&(y->next))\
   {\
      y=y->next->next;\
      mp=m;\
      m=m->next;\
  }\
\
   mp->next=NULL;\
   if(m==x)m->l=NULL;\
   else m->l=x;\
   m->r=m->next;\
\
   x=m;\
}


void
balance_tree(struct dict *d)
{
	struct node    *head = d->head;
	struct node    *link = head, *x;
	struct node_stack *ns_head = NULL;
	struct node_stack *ns_empties = NULL;

	relink_inorder(d);
	head->r = head->next;

	ns_push(head);

	while (ns_head != NULL)
	{
		ns_pop(x);

		if (x->l)
		{
			median_divide(x->l);
			add_link(x->l);
			ns_push(x->l);
		}

		if (x->r)
		{
			median_divide(x->r);
			add_link(x->r);
			ns_push(x->r);
		}
	}

	d->order = O_NONE;

	while (ns_empties)
	{
		ns_head = ns_empties->next;
		free(ns_empties);
		ns_empties = ns_head;
	}
}
