/*
 * $Log: getEnvConfig.c,v $
 * Revision 2.2  2015-08-21 10:46:32+05:30  Cprogrammer
 * added getEnvConfigLong
 *
 * Revision 2.1  2006-03-17 14:43:38+05:30  Cprogrammer
 * Initial Version
 *
 */
#include "indimail.h"
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: getEnvConfig.c,v 2.2 2015-08-21 10:46:32+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * getEnvConfigStr
 */
void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = getenv(envname)))
		*source = defaultValue;
	return;
}

void
getEnvConfigInt(int *source, char *envname, long defaultValue)
{
	char           *value;

	if (!(value = getenv(envname)))
		*source = defaultValue;
	else
		*source = atoi(value);
	return;
}

void
getEnvConfigLong(long *source, char *envname, long defaultValue)
{
	char           *value;

	if (!(value = getenv(envname)))
		*source = defaultValue;
	else
		*source = atol(value);
	return;
}

void
getversion_getenvConfig_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
