/*
 * $Log: count_rcpthosts.c,v $
 * Revision 2.4  2016-05-17 15:39:36+05:30  Cprogrammer
 * fixed comment
 *
 * Revision 2.3  2016-05-17 14:42:34+05:30  Cprogrammer
 * replace control directory with CONTROLDIR
 *
 * Revision 2.2  2005-12-29 22:40:39+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2002-08-25 22:30:22+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 1.3  2001-11-24 12:18:41+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:06+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>

#ifndef	lint
static char     sccsid[] = "$Id: count_rcpthosts.c,v 2.4 2016-05-17 15:39:36+05:30 Cprogrammer Exp mbhangui $";
#endif

/*
 * count the lines in rcpthosts
 */
int
count_rcpthosts()
{
	static char     tmpstr[MAX_BUFF];
	char           *qmaildir, *controldir;
	FILE           *fs;
	register int    count;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(tmpstr, MAX_BUFF, "%s/rcpthosts", controldir);
	else
		snprintf(tmpstr, MAX_BUFF, "%s/%s/rcpthosts", qmaildir, controldir);
	if(!(fs = fopen(tmpstr, "r")))
		return (0);
	for(count = 0; fgets(tmpstr, MAX_BUFF, fs);count++);
	fclose(fs);
	return (count);
}

void
getversion_count_rcpthosts_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
