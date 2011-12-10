#include <stdio.h>
#include "eps.h"

int
main(int argc, char *argv[])
{
	int             fd = 0, ret = 0;
	unsigned char  *l = NULL;
	struct eps_t   *eps = NULL;
	struct header_t *h = NULL;

	if(!(eps = eps_begin(INTERFACE_STREAM, (int *) &fd)))
		return 1;
	for (h = eps_next_header(eps); h; h = eps_next_header(eps))
	{
		if ((h->name) && (h->data))
			printf("[%s] = [%s]\n", h->name, h->data);
		eps_header_free(eps);
	}
	printf("\n");
	for (l = eps_next_line(eps); l; l = eps_next_line(eps))
		printf("%s\n", l);
	printf("\n");
	while ((!(eps->u->b->eof)) && (eps->content_type & CON_MULTI))
	{
		ret = mime_init_stream(eps);
		if (!ret)
			break;
		for (h = mime_next_header(eps); h; h = mime_next_header(eps))
		{
			if ((h->name) && (h->data))
				printf("[%s]=[%s]\n", h->name, h->data);
			header_kill(h);
		}
		printf("\n");
		for (l = mime_next_line(eps); l; l = mime_next_line(eps))
			printf("%s\n", l);
	}
	eps_end(eps);
	return 0;
}
