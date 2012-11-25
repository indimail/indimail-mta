/*
 * $Log: fixwvdialconf.c,v $
 * Revision 1.3  2008-06-20 22:21:58+05:30  Cprogrammer
 * made return type of main as int
 *
 * Revision 1.2  2008-06-20 20:50:43+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2003-12-13 17:46:42+05:30  Cprogrammer
 * Initial revision
 *
 */

#include <stdio.h>
#include <string.h>
int
main()
{
	FILE           *fp;
	char            buffer[1024];
	int             found;

	if (!(fp = fopen("/etc/wvdial.conf", "r")))
	{
		perror("/etc/wvdial.conf");
		return (1);
	}
	for (found = 0;;)
	{
		if (!fgets(buffer, sizeof(buffer), fp))
			break;
		if (strstr(buffer, "Auto Reconnect"))
		{
			found = 1;
			break;
		}
	}
	if (found)
	{
		fclose(fp);
		return (0);
	}
	if (!(fp = freopen("/etc/wvdial.conf", "a", fp)))
	{
		perror("freopen: /etc/wvdial.conf");
		return (1);
	}
	fprintf(fp, "Auto Reconnect = 0\n");
	fclose(fp);
	return (0);
}
