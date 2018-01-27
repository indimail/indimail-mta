/*
 * $Log: copyPwdStruct.c,v $
 * Revision 2.1  2002-07-03 01:17:31+05:30  Cprogrammer
 * function to return a new passwd struct after copy
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: copyPwdStruct.c,v 2.1 2002-07-03 01:17:31+05:30 Cprogrammer Stab mbhangui $";
#endif

struct passwd  *
copyPwdStruct(struct passwd *pw)
{
	static char     IUser[MAX_BUFF], IPass[MAX_BUFF], IGecos[MAX_BUFF];
	static char     IDir[MAX_BUFF], IShell[MAX_BUFF];
	static struct passwd pwent;

	pwent.pw_name = IUser;
	pwent.pw_passwd = IPass;
	pwent.pw_gecos = IGecos;
	pwent.pw_dir = IDir;
	pwent.pw_shell = IShell;
	scopy(pwent.pw_name, pw->pw_name, MAX_BUFF);
	scopy(pwent.pw_passwd, pw->pw_passwd, MAX_BUFF);
	pwent.pw_uid = pw->pw_uid;
	pwent.pw_gid = pw->pw_gid;
	scopy(pwent.pw_gecos, pw->pw_gecos, MAX_BUFF);
	scopy(pwent.pw_dir, pw->pw_dir, MAX_BUFF);
	scopy(pwent.pw_shell, pw->pw_shell, MAX_BUFF);
	return (&pwent);
}
void
getversion_copyPwdStruct_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
