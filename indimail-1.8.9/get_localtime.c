/*
 * $Log: get_localtime.c,v $
 * Revision 2.1  2011-07-29 09:25:42+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.2  2002-07-29 16:15:57+05:30  Cprogrammer
 * conditional compilation of timezone info for OE bug
 *
 * Revision 1.1  2002-02-25 13:57:12+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: get_localtime.c,v 2.1 2011-07-29 09:25:42+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>

char           *
get_localtime()
{
	time_t          tmval;
	struct tm      *tmptr;
	char           *ptr;
	static char     tmpbuf[36];
	int             mins;
#ifdef USE_TIMEZONE
	int             hours, len;
#endif

	tmval = time(0);
	tmptr = localtime(&tmval);
	mins = (timezone % 3600) / 60;
	if (mins < 0)
		mins = -mins;
	snprintf(tmpbuf, sizeof(tmpbuf), "%s", asctime(tmptr));
	if ((ptr = strchr(tmpbuf, '\n')))
		*ptr = 0;
#ifdef USE_TIMEZONE
	hours = timezone / 3600;
	len = strlen(tmpbuf);
	snprintf(tmpbuf + len, sizeof(tmpbuf), " %s %d:%02d", tzname[0], hours, mins);
#endif
	return (tmpbuf);
}

void
getversion_get_localtime_c()
{
	printf("%s\n", sccsid);
}
