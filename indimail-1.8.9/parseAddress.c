/*-
 * $Log: parseAddress.c,v $
 * Revision 2.5  2005-12-21 09:47:43+05:30  Cprogrammer
 * make gcc 4 happy
 *
 * Revision 2.4  2005-12-14 16:20:54+05:30  Cprogrammer
 * initialize addr_buf first to prevent segmentation fault
 *
 * Revision 2.3  2004-04-07 00:08:03+05:30  Cprogrammer
 * printing defect corrected
 *
 * Revision 2.2  2002-11-22 01:15:26+05:30  Cprogrammer
 * added missing #ifdef VFILTER
 *
 * Revision 2.1  2002-10-11 01:06:42+05:30  Cprogrammer
 * function to parse email addresses
 *
 */

#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: parseAddress.c,v 2.5 2005-12-21 09:47:43+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef VFILTER
#include <stdlib.h>
#include <string.h>

void
parseAddress(struct header_t *h, char *addr_buf)
{
	struct group_t *g = NULL;
	struct address_t *a = NULL;

	*addr_buf = 0;
	if (!(h->atoms))
	{
		fprintf(stderr, "no atoms\n");
		return;
	}
	if (!(h->atoms->next))
	{
		fprintf(stderr, "no atoms\n");
		return;
	}
	if(!(g = (struct group_t *) address_evaluate((char *) h->data)))
	{
		fprintf(stderr, "%s: no valid addresses\n", h->name);
		return;
	}
	if(verbose)
		printf("%s: %s%s%s\n", h->name, g->group ? "(of group " : "", g->group ? g->group : "", g->group ? ")" : "\0");
	for (a = g->members; a->next; a = a->next)
	{
		if ((a->next->user) && (a->next->domain))
		{
			if(verbose)
			{
				if (a->next->name)
					printf("  (%s) { [%s] @ [%s] }\n", a->next->name ? a->next->name : "", 
						a->next->user ? a->next->user : "N/A", a->next->domain ? a->next->domain : "N/A");
				else
					printf("  { [%s] @ [%s] }\n", a->next->user ? a->next->user : "N/A", 
						a->next->domain ? a->next->domain : "N/A");
			}
			if(a->next->user)
				strncat(addr_buf, a->next->user, slen(a->next->user) + 1);
			if(a->next->domain)
			{
				strncat(addr_buf, "@", 1);
				strncat(addr_buf, a->next->domain, slen(a->next->domain) + 1);
			}
			if(a->next->next)
				strncat(addr_buf, ",", 1);
		}
	}
	address_kill(g);
}
#endif

void
getversion_parseAddress_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
