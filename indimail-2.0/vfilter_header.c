/*
 * $Log: vfilter_header.c,v $
 * Revision 2.5  2016-05-25 09:11:06+05:30  Cprogrammer
 * use SYSCONFDIR for headerlist
 *
 * Revision 2.4  2008-07-13 19:49:53+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.3  2004-09-20 19:55:47+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.2  2004-03-19 14:53:57+05:30  Cprogrammer
 * Bugfix - was copying extra null byte
 *
 * Revision 2.1  2003-12-27 01:15:35+05:30  Cprogrammer
 * function to get header_list
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: vfilter_header.c,v 2.5 2016-05-25 09:11:06+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef VFILTER
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

char          **
headerList()
{
	char            buffer[MAX_BUFF];
	int             count, len;
	char           *ptr = SYSCONFDIR"/headerlist";
	char          **hptr;
	FILE           *fp;

	if (!(fp = fopen(ptr, "r")))
	{
		fprintf(stderr, "headerList: %s: %s\n", ptr, strerror(errno));
		return ((char **) 0);
	}
	for (count = 0;;)
	{
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if ((ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		count++;
	}
	if (!(hptr = (char **) malloc(sizeof(char *) * (count + 1))))
	{
		fprintf(stderr, "headerList: malloc: %s\n", strerror(errno));
		fclose(fp);
		return ((char **) 0);
	}
	rewind(fp);
	for (count = 0;;) 
	{
		if (!fgets(buffer, sizeof(buffer) - 2, fp))
			break;
		if ((ptr = strrchr(buffer, '\n')) || (ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		len = strlen(ptr) + 1;
		if (!(hptr[count] = (char *) malloc(sizeof(char) * len)))
		{
			fprintf(stderr, "headerList: malloc: %s\n", strerror(errno));
			fclose(fp);
		}
		strncpy(hptr[count++], ptr, len);
	}
	fclose(fp);
	hptr[count++] = 0;
	return (hptr);
}
#endif

void
getversion_vfilter_header_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
