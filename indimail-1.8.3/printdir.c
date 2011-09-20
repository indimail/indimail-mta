/*
 * $Log: printdir.c,v $
 * Revision 2.4  2010-08-15 15:54:12+05:30  Cprogrammer
 * display max users per level
 *
 * Revision 2.3  2009-12-30 13:09:12+05:30  Cprogrammer
 * run with uid of domain
 *
 * Revision 2.2  2008-09-17 19:29:02+05:30  Cprogrammer
 * setuid to indimail or domain uid to read .filesystems
 *
 * Revision 2.1  2003-01-14 12:48:16+05:30  Cprogrammer
 * changes for silent option in print_control()
 *
 * Revision 1.3  2001-12-08 17:44:14+05:30  Cprogrammer
 * usage message changed
 *
 * Revision 1.2  2001-12-02 18:47:24+05:30  Cprogrammer
 * change because of print_control() function change
 *
 * Revision 1.1  2001-12-02 11:52:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: printdir.c,v 2.4 2010-08-15 15:54:12+05:30 Cprogrammer Stab mbhangui $";
#endif

int
main(int argc, char **argv)
{
	char           *ptr;
	char            tmpbuf[MAX_BUFF];
	int             users_per_level = 0;
	FILE           *fp;
	uid_t           uid, myuid;
	gid_t           gid;

	if ((ptr = strrchr(argv[0], '/')) != NULL)
		ptr++;
	else
		ptr = argv[0];
	if (argc != 2)
	{
		error_stack(stderr, "usage: %s domain_name\n", ptr);
		return (1);
	}
	if(indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	if(!(ptr = vget_assign(argv[1], NULL, 0, &uid, &gid)))
	{
		error_stack(stderr, "%s: No such domain\n", argv[1]);
		return(-1);
	}
	if ((myuid = getuid()) != uid)
	{
		if (setgid(gid) || setuid(uid))
		{
			error_stack(stderr, "setuid/setgid (%d/%d): %s", uid, gid, strerror(errno));
			return (1);
		}
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.users_per_level", ptr);
	if ((fp = fopen(tmpbuf, "r")))
	{
		if (fscanf(fp, "%d", &users_per_level) != 1)
		{
			error_stack(stderr, "invalid domain users per level\n");
			return (1);
		}
		fclose(fp);
	} else
	if (errno != ENOENT)
	{
		error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
		return (1);
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.filesystems", ptr);
	print_control(tmpbuf, argv[1], users_per_level, 0);
	return(0);
}

void
getversion_printdir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
