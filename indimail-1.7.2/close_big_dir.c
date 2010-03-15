/*
 * $Log: close_big_dir.c,v $
 * Revision 1.4  2001-12-02 18:44:05+05:30  Cprogrammer
 * changed variable name filename to table_name
 *
 * Revision 1.3  2001-11-24 12:17:54+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:41+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:53+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: close_big_dir.c,v 1.4 2001-12-02 18:44:05+05:30 Cprogrammer Stab mbhangui $";
#endif

int
close_big_dir(char *table_name, char *domain, uid_t uid, gid_t gid)
{
	return(vwrite_dir_control(table_name, &vdir, domain, uid, gid));
}

void
getversion_close_big_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
