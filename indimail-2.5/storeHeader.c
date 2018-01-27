/*-
 * $Log: storeHeader.c,v $
 * Revision 2.4  2005-12-29 10:07:45+05:30  Cprogrammer
 * Prevent segmentation fault
 *
 * Revision 2.3  2005-12-21 09:49:33+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.2  2002-11-22 01:16:05+05:30  Cprogrammer
 * added missing #ifdef VFILTER
 *
 * Revision 2.1  2002-10-10 23:44:29+05:30  Cprogrammer
 * function to store header items
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: storeHeader.c,v 2.4 2005-12-29 10:07:45+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <stdlib.h>
#include <string.h>

int
storeHeader(struct header ***Hptr, struct header_t *h)
{
	struct header **ptr, **hptr;
	static int      count;
	int             i, found, idx, len;

	if (!*h->data)
		return 0;
	if(!Hptr)
		hptr = (struct header **) 0;
	else
		hptr = *Hptr;
	if(!hptr)
	{
		if(!((hptr = (struct header **) malloc(sizeof(struct header *) * 2))))
		{
			perror("malloc");
			return(1);
		} else
			*Hptr = hptr;
		if(!(hptr[0] = (struct header *) malloc(sizeof(struct header))))
		{
			perror("malloc");
			return(1);
		} else
		if(!(hptr[0]->name = (char *) malloc((len = slen((const char *) h->name)) + 1)))
		{
			perror("malloc");
			return(1);
		} else
			scopy(hptr[0]->name, (const char *) h->name, len + 1);
		if(!(hptr[0]->data = (char **) malloc(sizeof(char *) * 2)))
		{
			perror("malloc");
			return(1);
		} else
		if(!(hptr[0]->data[0] = (char *) malloc((len = slen((const char *) h->data)) + 1)))
		{
			perror("malloc");
			return(1);
		} else
		{
			for (i = 0; i_headers[i]; i++)
			{
				if (!(strcasecmp(i_headers[i], (const char *) h->name)))
				{
					parseAddress(h, hptr[0]->data[0]);
					break;
				}
			}
			if(!i_headers[i])
				scopy(hptr[0]->data[0], (const char *) h->data, len + 1);
		}
		hptr[0]->data[1] = (char *) 0;
		hptr[0]->data_items = 1;
		hptr[1] = (struct header *) 0;
		count = 1;
		return(0);
	} 
	for(ptr = hptr,found = idx = 0;idx < count;idx++)
	{
		if(!strcasecmp(ptr[idx]->name, (const char *) h->name))
		{
			found = 1;
			break;
		} 
	}
	if(found)
	{
		if(!(ptr[idx]->data = (char **) realloc(ptr[idx]->data, sizeof(char *) * (ptr[idx]->data_items + 2))))
		{
			perror("malloc");
			return(1);
		} else
		if(!(ptr[idx]->data[ptr[idx]->data_items] = (char *) malloc((len = slen((const char *) h->data)) + 1)))
		{
			perror("malloc");
			return(1);
		} else
		{
			for (i = 0; i_headers[i]; i++)
			{
				if (!(strcasecmp(i_headers[i], (const char *) h->name)))
				{
					parseAddress(h, ptr[idx]->data[ptr[idx]->data_items++]);
					break;
				}
			}
			if(!i_headers[i])
				scopy(ptr[idx]->data[ptr[idx]->data_items++], (const char *) h->data, len + 1);
		}
		ptr[idx]->data[ptr[idx]->data_items] = (char *) 0;
	} else
	{
		if(!((hptr = (struct header **) realloc(hptr, sizeof(struct header *) * (count + 2)))))
		{
			perror("malloc");
			return(1);
		} else
			*Hptr = hptr;
		if(!(hptr[count] = (struct header *) malloc(sizeof(struct header))))
		{
			perror("malloc");
			return(1);
		} else
		if(!(hptr[count]->name = (char *) malloc((len = slen((const char *) h->name)) + 1)))
		{
			perror("malloc");
			return(1);
		} else
			scopy(hptr[count]->name, (const char *) h->name, len + 1);
		if(!(hptr[count]->data = (char **) malloc(sizeof(char *) * 2)))
		{
			perror("malloc");
			return(1);
		} else
		if(!(hptr[count]->data[0] = (char *) malloc((len = slen((const char *) h->data)) + 1)))
		{
			perror("malloc");
			return(1);
		} else
		{
			for (i = 0; i_headers[i]; i++)
			{
				if (!(strcasecmp(i_headers[i], (const char *) h->name)))
				{
					parseAddress(h, hptr[count]->data[0]);
					break;
				}
			}
			if(!i_headers[i])
				scopy(hptr[count]->data[0], (const char *) h->data, len + 1);
		}
		hptr[count]->data[1] = (char *) 0;
		hptr[count++]->data_items = 1;
		hptr[count] = (struct header *) 0;
	}
	return(0);
}
#endif

void
getversion_storeHeader_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
