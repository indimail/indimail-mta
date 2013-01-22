/*
 * $Log: strToPw.c,v $
 * Revision 2.1  2008-07-13 19:47:45+05:30  Cprogrammer
 * removed compilation warning on FC 9
 *
 * Revision 1.2  2002-04-08 14:59:28+05:30  Cprogrammer
 * set is_overquota
 * will work for strings which do not contain 'PWSTRUCT='
 *
 * Revision 1.1  2002-04-07 13:42:06+05:30  Cprogrammer
 * Initial revision
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: strToPw.c,v 2.1 2008-07-13 19:47:45+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include "indimail.h"

struct passwd  *
strToPw(char *pwbuf, int len)
{
	char           *ptr;
	int             row_count, pwstruct_len;
	static struct passwd pwent;
	static char     __PWstruct[MAX_BUFF], _pwstruct[MAX_BUFF];
	static char     IUser[MAX_BUFF], IPass[MAX_BUFF], IGecos[MAX_BUFF];
	static char     IDir[MAX_BUFF], IShell[MAX_BUFF];

	if(!pwbuf || !*pwbuf)
		return((struct passwd *) 0);
	if(!strncmp(pwbuf, "PWSTRUCT=", 9))
	{
		pwstruct_len = len - 9;
		scopy(_pwstruct, pwbuf + 9, len - 9);
	} else
	{
		pwstruct_len = len;
		scopy(_pwstruct, pwbuf, len);
	}
	if(*__PWstruct && !strncmp(__PWstruct, _pwstruct, pwstruct_len))
		return((struct passwd *) &pwent);
	if(!pwent.pw_name) /*- first time */
	{
		pwent.pw_name = IUser;
		pwent.pw_passwd = IPass;
		pwent.pw_gecos = IGecos;
		pwent.pw_dir = IDir;
		pwent.pw_shell = IShell;
	}
	is_overquota = is_inactive = userNotFound = 0;
	scopy(__PWstruct, _pwstruct, pwstruct_len);
	if(!strncmp(_pwstruct, "No such user", 12))
	{
		userNotFound = 1;
		return ((struct passwd *) 0);
	} else
	if((ptr = strtok(_pwstruct, ":")))
	{
		scopy(pwent.pw_name, ptr, MAX_BUFF);
		if((ptr = strchr(pwent.pw_name, '@')))
			*ptr = 0;
		for(row_count = 1;(ptr = strtok(0, ":"));row_count++)
		{
			switch(row_count)
			{
				case 1:
					scopy(pwent.pw_passwd, ptr, MAX_BUFF);
					break;
				case 2:
					pwent.pw_uid = atoi(ptr);
					break;
				case 3:
					pwent.pw_gid = atoi(ptr);
					if (pwent.pw_gid & BOUNCE_MAIL)
						is_overquota = 1;
					break;
				case 4:
					scopy(pwent.pw_gecos, ptr, MAX_BUFF);
					break;
				case 5:
					scopy(pwent.pw_dir, ptr, MAX_BUFF);
					break;
				case 6:
					scopy(pwent.pw_shell, ptr, MAX_BUFF);
					break;
				case 7:
					if(*ptr)
						is_inactive = atoi(ptr);
					break;
			}
		} /*- for(row_count = 1;ptr = strtok(0, ":");row_count++) */
		if(row_count == 8)
			return(&pwent);
	} 
	return ((struct passwd *) 0);
}

void
getversion_strToPw_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
