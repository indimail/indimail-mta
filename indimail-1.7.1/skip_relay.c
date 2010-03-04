/*
 * $Log: skip_relay.c,v $
 * Revision 2.4  2009-02-18 21:33:42+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.3  2009-02-06 11:40:15+05:30  Cprogrammer
 * ignore return value of fscanf
 *
 * Revision 2.2  2008-06-13 10:15:29+05:30  Cprogrammer
 * compile skip_relay() if POP_AUTH_OPEN_RELAY defined
 *
 * Revision 2.1  2005-12-29 22:50:17+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 1.4  2002-08-01 17:09:51+05:30  Cprogrammer
 * check only lines with allow,RELAYCLIENT
 * skip comments
 *
 * Revision 1.3  2001-11-24 12:20:08+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:03+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: skip_relay.c,v 2.4 2009-02-18 21:33:42+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int
skip_relay(ipaddr)
	char           *ipaddr;
{
	FILE           *fp;
	char            buffer[MAX_BUFF];
	char           *ptr, *tcp_file;

	getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
	if (!(fp = fopen(tcp_file, "r")))
		return(0);
	for (;;)
	{
		if (fscanf(fp, "%s", buffer) != 1)
		{
			if (ferror(fp))
				break;
		}
		if (feof(fp))
			break;
		if ((ptr = strchr(buffer, '#')))
			*ptr = 0;
		for (ptr = buffer;*ptr && isspace((int) *ptr);ptr++);
		if (!*ptr)
			continue;
		if (!strstr(buffer, "allow") || !strstr(buffer, "RELAYCLIENT"))
			continue;
		if ((ptr = strchr(buffer, ':')) != NULL)
		{
			*ptr = 0;
			if (!strncmp(buffer, ipaddr, slen(buffer)))
			{
				fclose(fp);
				return(1);
			}
		}
	}
	fclose(fp);
	return(0);
}
#endif

void
getversion_skip_relay_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
