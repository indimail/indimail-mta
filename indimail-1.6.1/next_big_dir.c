/*
 * $Log: next_big_dir.c,v $
 * Revision 1.3  2001-11-24 12:19:43+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:39+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: next_big_dir.c,v 1.3 2001-11-24 12:19:43+05:30 Cprogrammer Stab mbhangui $";
#endif

char           *
next_big_dir(uid_t uid, gid_t gid)
{
	inc_dir_control(&vdir);
	return (vdir.the_dir);
}

void
getversion_next_big_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
