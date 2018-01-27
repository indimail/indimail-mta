/*
 * $Log: valiasinfo.c,v $
 * Revision 2.4  2005-12-29 22:51:06+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.3  2004-05-19 20:04:16+05:30  Cprogrammer
 * new logic for replacing '.' with ':'
 *
 * Revision 2.2  2004-04-20 10:53:37+05:30  Cprogrammer
 * replace '.' with ':' for .qmail files
 *
 * Revision 2.1  2002-05-12 01:23:08+05:30  Cprogrammer
 * formatting changes of printf statements
 * added code to display .qmail files lying in user's homedir
 *
 * Revision 1.3  2001-11-24 12:20:42+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:34+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <pwd.h>

#ifndef	lint
static char     sccsid[] = "$Id: valiasinfo.c,v 2.4 2005-12-29 22:51:06+05:30 Cprogrammer Stab mbhangui $";
#endif

int
valiasinfo(char *user, char *domain)
{
	int             flag1;
	char            tmpbuf[MAX_BUFF];
	FILE           *fp;
	char            Dir[MAX_BUFF];
	struct passwd  *pw;
	char           *qmaildir, *tmpalias;
#ifdef VALIAS
	int             flag2;
#endif
	uid_t           uid;
	gid_t           gid;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	if (!domain || !*domain)
		snprintf(Dir, MAX_BUFF, "%s/alias", qmaildir);
	else
	if (!vget_assign(domain, Dir, MAX_BUFF, &uid, &gid))
		*Dir = 0;
	if (*Dir)
	{
		snprintf(tmpbuf, MAX_BUFF, "%s/.qmail-%s", Dir, user);
		/* replace all dots with ':' */
		for (tmpalias = tmpbuf + slen(Dir) + 8;*tmpalias;tmpalias++)
		{
			if (*tmpalias == '.')
				*tmpalias = ':';
		}
		if ((fp = fopen(tmpbuf, "r")) != (FILE *) 0)
		{
			for (flag1 = 0;;)
			{
				if (!fgets(tmpbuf, MAX_BUFF, fp))
					break;
				if (!flag1++)
					printf("Forwarding    : ");
				else
					printf("              : ");
				if (*tmpbuf == '&')
					printf("%s", tmpbuf + 1);
				else
					printf("%s", tmpbuf);
			}
			fclose(fp);
			if (!flag1++)
				printf("Forwarding    : %s/Maildir/\n", Dir);
		} else
			flag1 = 0;
		if ((pw = vauth_getpw(user, domain)) != (struct passwd *) 0)
			snprintf(tmpbuf, MAX_BUFF, "%s/.qmail", pw->pw_dir);
		if (pw && ((fp = fopen(tmpbuf, "r")) != (FILE *) 0))
		{
			for (;;)
			{
				if (!fgets(tmpbuf, MAX_BUFF, fp))
					break;
				if (!flag1++)
					printf("Forwarding    : ");
				else
					printf("              : ");
				if (*tmpbuf == '&')
					printf("%s", tmpbuf + 1);
				else
					printf("%s", tmpbuf);
			}
			fclose(fp);
		}
	} else
		flag1 = 0;
#ifdef VALIAS
	for (flag2 = 0;;)
	{
		if (!(tmpalias = valias_select(user, domain)))
			break;
		if (*tmpalias == '&')
			tmpalias++;
		if (!flag2++)
			printf("Forwarding    : %s\n", tmpalias);
		else
			printf("              : %s\n", tmpalias);
	}
	return(flag1 + flag2);
#else
	return(flag1);
#endif
}

void
getversion_valiasinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
