#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eps.h"

struct i_header_t _i_headers[] = {
	{(unsigned char *) "Content-type", email_content_type},
	{NULL, NULL}
};

void
email_content_type(struct eps_t *eps, struct header_t *h, void *x)
{
	int             type = 0;
	char           *p = NULL;

	if ((!eps->h) || (!eps->h->atoms) || (!eps->h->atoms->next) || (!eps->h->atoms->next->data))
	{

		type = eps->content_type = CON_TEXT;
		return;
	}

	else
	{
		type = content_parse((char *) eps->h->atoms->next->data, TYP_CON);
		eps->content_type = type;
	}

	if (type & CON_MULTI)
	{
		p = (char *) header_fetch_atom(eps->h, (unsigned char *) "boundary");
		if (p)
		{
			if (eps->b == NULL)
			{
				eps->b = boundary_alloc();
				if (eps->b == NULL)
				{
					eps->content_type = CON_TEXT;
					return;
				}
			}

			boundary_add(eps, p);
		}
	}
}

void
email_header_internal(struct eps_t *eps)
{
	int             i = 0;

	if ((!eps->h) || (!(eps->h->name)) || (!(eps->h->data)))
		return;

	for (i = 0; _i_headers[i].name; i++)
	{
		if (!(strcasecmp((const char *) _i_headers[i].name, (const char *) eps->h->name)))
			_i_headers[i].func(eps, NULL, NULL);
	}
}
