/*
 * $Log: is_alias_domain.c,v $
 * Revision 2.3  2009-02-18 09:07:24+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.2  2009-02-06 11:38:10+05:30  Cprogrammer
 * ignore return value of fgets
 *
 * Revision 2.1  2002-05-05 21:08:20+05:30  Cprogrammer
 * alias domains are symbolic links
 *
 * Revision 1.3  2001-11-24 12:19:14+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:15+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:01+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: is_alias_domain.c,v 2.3 2009-02-18 09:07:24+05:30 Cprogrammer Stab mbhangui $";
#endif

int
is_alias_domain(char *domain)
{
	FILE           *fs;
	char            dir[MAX_BUFF], tmpbuf1[MAX_BUFF], tmpbuf2[MAX_BUFF], buffer[MAX_BUFF];
	struct stat     statbuf;

	if (vget_assign(domain, dir, MAX_BUFF, NULL, NULL) == 0)
		return (0);
	if (lstat(dir, &statbuf))
	{
		fprintf(stderr, "is_alias_domain: lstat: %s: %s\n", dir, strerror(errno));
		return(-1);
	}
	if (S_ISLNK(statbuf.st_mode))
		return(1);
	snprintf(tmpbuf1, MAX_BUFF, "%s/.aliasdomains", dir);
	snprintf(tmpbuf2, MAX_BUFF, "%s\n", domain);
	if ((fs = fopen(tmpbuf1, "r")) != (FILE *)NULL)
	{
		for (;;)
		{
			if (!fgets(buffer, MAX_BUFF, fs))
			{
				if(feof(fs))
					break;
				fprintf(stderr, "is_alias_domain: fgets: %s\n", strerror(errno));
				return (-1);
			}
			if (!strcmp(tmpbuf2, buffer))
			{
				fclose(fs);
				return(1);
			}
		}
		fclose(fs);
	}
	return(0);
}

void
getversion_is_alias_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
