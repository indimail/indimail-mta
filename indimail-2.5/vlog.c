/*
 * $Log: vlog.c,v $
 * Revision 2.2  2009-09-19 19:04:30+05:30  Cprogrammer
 * fix for mandriva 2009.1
 *
 * Revision 2.1  2008-05-28 17:41:30+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 1.1  2001-12-03 02:29:07+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <syslog.h>

static char    *sysc(char *);

#ifndef	lint
static char     sccsid[] = "$Id: vlog.c,v 2.2 2009-09-19 19:04:30+05:30 Cprogrammer Stab mbhangui $";
#endif
/*
 * log messages and figure out what type they are and where they should go depending on configure options 
 * any one of the pointers can be null, i.e. the information is not available 
 * messages are automatically cleaned for syslog if it is necessary
 */
void
vlog(int verror, char *TheUser, char *TheDomain, char *ThePass, char *TheName, char *IpAddr, char *LogLine)
{
	/*
	 * always log to syslog if enabled 
	 */
	if ((verror == VLOG_ERROR_PASSWD) && (ENABLE_LOGGING == 1 || ENABLE_LOGGING == 2 || ENABLE_LOGGING == 3 || ENABLE_LOGGING == 4))
		syslog(LOG_NOTICE, "%s", sysc(LogLine));
	else
	if (verror == VLOG_ERROR_INTERNAL)
		syslog(LOG_NOTICE, "%s", sysc(LogLine));
	else
	if (verror == VLOG_ERROR_LOGON)
		syslog(LOG_NOTICE, "%s", sysc(LogLine));
	else
	if (verror == VLOG_ERROR_ACCESS)
		syslog(LOG_NOTICE, "%s", sysc(LogLine));
	else
	if (verror == VLOG_AUTH && (ENABLE_LOGGING == 1 || ENABLE_LOGGING == 4))
		syslog(LOG_NOTICE, "%s", sysc(LogLine));
#ifdef ENABLE_MYSQL_LOGGING
	/*
	 * always log to mysql if mysql logging is enabled and it is not internal error 
	 */
	if ((ENABLE_MYSQL_LOGGING > 0) && (verror != VLOG_ERROR_INTERNAL))
	{
		if ((logmysql(verror, TheUser, TheDomain, ThePass, TheName, IpAddr, LogLine)) != 0)
			syslog(LOG_NOTICE, "vlog: can't write MySQL logs");
	}
#endif
}

/* clean a buffer for syslog */
static char    *
sysc(char *mess)
{
	char           *ripper;

	for (ripper = mess; *ripper; ++ripper)
	{
		if (*ripper == '%')
			*ripper = '#';
	}
	return (mess);
}

void
getversion_vlog_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
