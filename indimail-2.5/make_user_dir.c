/*
 * $Log: make_user_dir.c,v $
 * Revision 2.10  2016-05-18 11:46:15+05:30  Cprogrammer
 * use DOMAINDIR for users directories
 *
 * Revision 2.9  2010-08-08 13:02:21+05:30  Cprogrammer
 * made users_per_level configurable
 *
 * Revision 2.8  2009-01-13 14:40:50+05:30  Cprogrammer
 * added add operation to backfill
 *
 * Revision 2.7  2009-01-12 10:38:21+05:30  Cprogrammer
 * use function backfill() to backfill empty slots in dir_control
 *
 * Revision 2.6  2008-09-14 21:02:35+05:30  Cprogrammer
 * return error if open_big_dir fails
 *
 * Revision 2.5  2008-09-14 19:48:34+05:30  Cprogrammer
 * return "" to denote success when hash is 0
 *
 * Revision 2.4  2004-06-24 20:01:09+05:30  Cprogrammer
 * BUG - incorrect .qmail file
 * change ownership of .qmail to indimail
 *
 * Revision 2.3  2003-03-27 20:37:54+05:30  Cprogrammer
 * added argument subject to SendWelcomeMail
 *
 * Revision 2.2  2002-08-01 16:15:48+05:30  Cprogrammer
 * change for addtional argument to SendWelcomeMail
 *
 * Revision 2.1  2002-05-16 01:08:50+05:30  Cprogrammer
 * added welcome mail delivery code
 *
 * Revision 1.4  2001-12-02 18:45:42+05:30  Cprogrammer
 * changes for directory name to be passed to open_big_dir() and dec_dir_control()
 *
 * Revision 1.3  2001-11-24 12:19:28+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:23+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.1  2001-10-24 18:15:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: make_user_dir.c,v 2.10 2016-05-18 11:46:15+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * figure out where to put the user and
 * make the directories if needed
 */
char           *
make_user_dir(char *username, char *domain, uid_t uid, gid_t gid, int users_per_level)
{
	char           *tmpstr, *tmpdir, *fname;
	char            tmpbuf1[MAX_BUFF], tmpbuf2[MAX_BUFF];
	FILE           *fp;

	if (!*username)
		return ((char *) 0);
	tmpdir = (char *) NULL;
	if (!domain || !*domain)
	{
		tmpdir = malloc(slen(DOMAINDIR) + 7);
		snprintf(tmpdir, slen(DOMAINDIR) + 7, "%s/users", DOMAINDIR);
	} else
		tmpdir = (char *) get_Mplexdir(username, domain, create_flag, uid, gid);
	if (!(tmpstr = backfill(username, domain, tmpdir, 1)))
	{
		if (!(fname = open_big_dir(username, domain, tmpdir)))
		{
			fprintf(stderr, "open_big_dir: %s\n", strerror(errno));
			return ((char *) 0);
		}
		tmpstr = next_big_dir(uid, gid, users_per_level);
		close_big_dir(fname, domain, uid, gid);
	}
	if(tmpstr && *tmpstr)
		snprintf(tmpbuf1, MAX_BUFF, "%s/%s/%s", tmpdir, tmpstr, username);
	else
		snprintf(tmpbuf1, MAX_BUFF, "%s/%s", tmpdir, username);
	if(tmpdir)
		free(tmpdir);
	if(!domain || !*domain || create_flag)
	{
		if(vmake_maildir(tmpbuf1, uid, gid, domain) && errno != EEXIST)
		{
			fprintf(stderr, "mkdir-%s: %s\n", tmpbuf1, strerror(errno));
			dec_dir_control(tmpbuf1, username, domain, uid, gid);
			return (NULL);
		}
		if(!domain || !*domain)
		{
			snprintf(tmpbuf2, MAX_BUFF, "%s/.qmail", tmpbuf1);
			if ((fp = fopen(tmpbuf2, "w")) != (FILE *) NULL)
			{
				fprintf(fp, "%s/Maildir/\n", tmpbuf1);
				fclose(fp);
				if (chown(tmpbuf2, uid, gid))
				{
					fprintf(stderr, "chown-%s: %s\n", tmpbuf2, strerror(errno));
					dec_dir_control(tmpbuf1, username, domain, uid, gid);
					return(NULL);
				}
			} else
			{
				fprintf(stderr, "fopen-%s: %s\n", tmpbuf2, strerror(errno));
				dec_dir_control(tmpbuf1, username, domain, uid, gid);
				return (NULL);
			}
		} else
		if(create_flag)
			SendWelcomeMail(tmpbuf1, username, domain, 0, 0);
	}
	if (*tmpstr)
		return (tmpstr);
	else
		return ("");
}

void
getversion_make_user_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
