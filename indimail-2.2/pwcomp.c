/*
 * $Log: pwcomp.c,v $
 * Revision 1.1  2001-12-19 20:55:27+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#ifndef	lint
static char     sccsid[] = "$Id: pwcomp.c,v 1.1 2001-12-19 20:55:27+05:30 Cprogrammer Stab mbhangui $";
#endif

int
pwcomp(pw1, pw2)
	struct passwd  *pw1, *pw2;
{
	if (!pw1 || !pw2)
		return (1);
	else
	if (strcmp(pw1->pw_name, pw2->pw_name))
		return (1);
	else
	if (strcmp(pw1->pw_passwd, pw2->pw_passwd))
		return (1);
	else
	if (pw1->pw_uid != pw2->pw_uid)
		return (1);
	else
	if (pw1->pw_gid != pw2->pw_gid)
		return (1);
	else
	if (strcmp(pw1->pw_gecos, pw2->pw_gecos))
		return (1);
	else
	if (strcmp(pw1->pw_dir, pw2->pw_dir))
		return (1);
	else
	if (strcmp(pw1->pw_shell, pw2->pw_shell))
		return (1);
	return (0);
}

void
getversion_pwcomp_c()
{
	printf("%s\n", sccsid);
	return;
}
