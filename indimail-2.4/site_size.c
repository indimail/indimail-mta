/*
 * $Log: site_size.c,v $
 * Revision 1.3  2001-11-24 12:20:06+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:02+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: site_size.c,v 1.3 2001-11-24 12:20:06+05:30 Cprogrammer Stab mbhangui $";
#endif
/*-
 * Description: Set the format of the table indimail depending on
 * wether large site or small site was taken during installation
 * of indimail
 * 1. Small Site : indimail table contains the following fields
 *                 pw_name
 *                 pw_domain,
 *                 pw_passwd,
 *                 pw_uid,
 *                 pw_gid,
 *                 pw_gecos,
 *                 pw_dir,
 *                 pw_shell
 *    Selecting Small Site gives a single indimail table containing all
 *    records
 * 2. Large Site : Each table contains the following fields
 *                 pw_name
 *                 pw_domain,
 *                 pw_passwd,
 *                 pw_uid,
 *                 pw_gid,
 *                 pw_gecos,
 *                 pw_dir,
 *                 pw_shell
 *     Selecting Large Site gives multiple tables named as the domain name
 */
void
SetSiteSize(int size)
{
	site_size = size;
	return;
}


void
getversion_site_size_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
