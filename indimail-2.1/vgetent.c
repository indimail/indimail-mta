/*
 * $Log: vgetent.c,v $
 * Revision 1.3  2001-11-24 12:22:01+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:51+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <string.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: vgetent.c,v 1.3 2001-11-24 12:22:01+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * fill out a passwd structure from then next
 * line in a file 
 */
struct passwd  *
vgetent(FILE * pw)
{
	static struct passwd pwent;
	static char     line[MAX_BUFF];
	int             i = 0, j = 0;

	if (!fgets(line, sizeof(line), pw))
		return NULL;
	for (i = 0; line[i] != 0; i++)
		if (line[i] == ':')
	if (j != 6)
		return NULL;
	pwent.pw_name = strtok(line, ":");
	pwent.pw_passwd = strtok(NULL, ":");
	pwent.pw_uid = atoi(strtok(NULL, ":"));
	pwent.pw_gid = atoi(strtok(NULL, ":"));
	pwent.pw_gecos = strtok(NULL, ":");
	pwent.pw_dir = strtok(NULL, ":");
	pwent.pw_shell = strtok(NULL, ":");
	return &pwent;
}

void
getversion_vgetent_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
