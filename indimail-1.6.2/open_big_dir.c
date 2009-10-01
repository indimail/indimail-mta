/*
 * $Log: open_big_dir.c,v $
 * Revision 2.1  2008-09-14 19:21:38+05:30  Cprogrammer
 * removed function dc_filename()
 *
 * Revision 1.4  2001-12-02 18:46:26+05:30  Cprogrammer
 * additional path argument to be passed to dc_filename()
 *
 * Revision 1.3  2001-11-24 12:19:44+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:40+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: open_big_dir.c,v 2.1 2008-09-14 19:21:38+05:30 Cprogrammer Stab mbhangui $";
#endif

char *
open_big_dir(char *username, char *domain, char *path)
{
	char           *filesys_prefix;

	if(!(filesys_prefix = GetPrefix(username, path)))
		return((char *) 0);
	if(vread_dir_control(filesys_prefix, &vdir, domain))
		return((char *) 0);
	return(filesys_prefix);
}

void
getversion_open_big_dir_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
