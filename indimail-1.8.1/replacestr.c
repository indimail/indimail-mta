/*
 * $Log: replacestr.c,v $
 * Revision 1.2  2001-12-02 20:22:15+05:30  Cprogrammer
 * added function getversion_replacestr_c()
 *
 * Revision 1.1  2001-12-01 23:09:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: replacestr.c,v 1.2 2001-12-02 20:22:15+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * function to replace all occurence of string ch with rch in str. NOTE:
 * pointer returned by replace should be freed when not needed
 */
char           *
replacestr(str, ch, rch)
	char           *str, *ch, *rch;
{
	int             len1, len2, diff, slen, resize;
	char           *ptr1, *ptr2, *tmptr, *svptr;

	if(!strstr(str, ch))
		return(str);
	/* copy all necessary strings */
	for (ptr1 = str, slen = 0; *ptr1; ptr1++)
		slen++;
	for (ptr1 = ch, len1 = 0; *ptr1; ptr1++)
		len1++;
	for (ptr1 = rch, len2 = 0; *ptr1; ptr1++)
		len2++;
	/* allocate memory to hold the replaced string */
	if (!(tmptr = (char *) malloc(sizeof(char) * (strlen(str) + 1))))
		return ((char *) 0);
	diff = len2 - len1;
	len2 = 0;
	resize = 1;
	for (ptr1 = str, ptr2 = tmptr; *ptr1; ptr1++)
	{
		/* realloc as no space left in tmptr */
		if (len2 > slen - diff)
		{
			resize++;
			if (!(svptr = (char *) realloc((void *) tmptr, sizeof(char) * resize * (strlen(str) + 1))))
			{
				(void) free((void *) tmptr);
				return ((char *) 0);
			}
			tmptr = svptr;
			ptr2 = tmptr + len2;
		}
		if (*ptr1 == *ch)
		{
			/*
			 * we found the string to be replaced. This can be made
			 * faster if replacement is done along with comparision
			 */
			if (!memcmp(ptr1, ch, len1))
			{
				if(*rch) /* if replacement char is not NULL */
				{
					svptr = ptr1;
					for (ptr1 = rch; *ptr1; ptr1++)
						*ptr2++ = *ptr1;
					len2 += len1 + diff;
					ptr1 = svptr;
				} 
				/* go past the replaced string */
				ptr1 += len1 - 1;
				continue;
			} else
				*ptr2++ = *ptr1;
		} else
			*ptr2++ = *ptr1;
		len2++;
	}
	*ptr2 = 0;
	return (tmptr);
}

void
getversion_replacestr_c()
{
	printf("%s\n", sccsid);
	return;
}
