/*
 * $Log: config_settings.c,v $
 * Revision 2.7  2016-05-25 08:59:16+05:30  Cprogrammer
 * use SYSCONFDIR for indimail.settings
 *
 * Revision 2.6  2003-07-25 09:22:55+05:30  Cprogrammer
 * correction on multi-line strings
 *
 * Revision 2.5  2003-01-05 16:05:15+05:30  Cprogrammer
 * use ~indimail/etc/indimail.settings if file not found in CWD
 *
 * Revision 2.4  2002-10-10 01:39:10+05:30  Cprogrammer
 * added code to put warning statement
 *
 * Revision 2.3  2002-10-01 13:40:05+05:30  Cprogrammer
 * use indimail/etc/indimail.settings instead of looking in current working dir
 *
 * Revision 3.2  2002-07-31 19:31:18+05:30  Cprogrammer
 * program to generate indimail_settings.c
 *
 */
#include "indimail.h"
#include <string.h>
#include <unistd.h>

#ifndef	lint
static char     sccsid[] = "$Id: config_settings.c,v 2.7 2016-05-25 08:59:16+05:30 Cprogrammer Exp mbhangui $";
#endif

int
main(int argc, char **argv)
{
	FILE           *fp;
	char            buffer[2048];
	char           *ptr;

	snprintf(buffer, sizeof(buffer), "./indimail.settings");
	if(access(buffer, F_OK))
		snprintf(buffer, sizeof(buffer), "%s/indimail.settings", SYSCONFDIR);
	if ((fp = fopen(buffer, "r")))
	{
		printf("/*- WARNING: Do not edit. This file has been automatically generated */\n");
		printf("#include <stdio.h>\n\nvoid indimail_settings()\n{\n");
		for (;;)
		{
			if (!fgets(buffer, sizeof(buffer) - 2, fp))
				break;
			if((ptr = strrchr(buffer, '\n')))
				*ptr = 0;
			printf("printf(\"%s\\n\");\n", buffer);
		}
		printf("return;\n");
		printf("}\n");
		fclose(fp);
		return (0);
	} else
		return (1);
}

void
getversion_config_settings_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
