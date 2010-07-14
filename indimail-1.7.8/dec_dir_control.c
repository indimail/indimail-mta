/*
 * $Log: dec_dir_control.c,v $
 * Revision 2.4  2009-01-15 00:12:50+05:30  Cprogrammer
 * BUG vdir.cur_users was not getting decremented
 *
 * Revision 2.3  2009-01-13 14:39:52+05:30  Cprogrammer
 * backfill user removed
 *
 * Revision 2.2  2008-05-28 16:34:57+05:30  Cprogrammer
 * removed USE_MYSQL, removed cdb code
 *
 * Revision 2.1  2003-04-08 11:33:17+05:30  Cprogrammer
 * bug fix. GetPrefix returning null caused core dump
 *
 * Revision 1.5  2001-12-02 20:19:10+05:30  Cprogrammer
 * removed argument domain
 * called dc_filename if mysql is not defined
 *
 * Revision 1.4  2001-12-02 18:40:35+05:30  Cprogrammer
 * modification for GetPrefix() function change
 *
 * Revision 1.3  2001-11-24 12:18:44+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:54:09+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:57+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: dec_dir_control.c,v 2.4 2009-01-15 00:12:50+05:30 Cprogrammer Stab mbhangui $";
#endif

int
dec_dir_control(char *path, char *user, char *domain, uid_t uid, gid_t gid)
{
	char           *ptr;

	if (!(ptr = GetPrefix(user, path)))
		return(-1);
	backfill(user, domain, path, 2);
	vread_dir_control(ptr, &vdir, domain);
	if (vdir.cur_users)
		vdir.cur_users--;
	return(vwrite_dir_control(ptr, &vdir, domain, uid, gid));
}

void
getversion_dec_dir_control_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
