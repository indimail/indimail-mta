/*-
 * $Log: no_of_days.c,v $
 * Revision 1.1  2001-11-24 20:36:17+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <time.h>

#ifndef	lint
static char     sccsid[] = "$Id: no_of_days.c,v 1.1 2001-11-24 20:36:17+05:30 Cprogrammer Stab mbhangui $";
#endif

char *
no_of_days(time_t seconds)
{
	int days, hours, mins, secs;
	static char     tmpbuf[MAX_BUFF];

	days = seconds/86400;
	hours = (seconds % 86400)/3600;
	mins = ((seconds % 86400) % 3600)/60;
	secs = ((seconds % 86400) % 3600) % 60;
	snprintf(tmpbuf, MAX_BUFF, "%d days %02d Hrs %02d Mins %02d Secs", days, hours, mins, secs);
	return(tmpbuf);
}


void
getversion_no_of_days_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
