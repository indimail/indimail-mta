/*
 * $Log: skip_system_files.c,v $
 * Revision 2.5  2014-04-18 17:27:47+05:30  Cprogrammer
 * added folder.dateformat
 *
 * Revision 2.4  2004-07-02 18:16:45+05:30  Cprogrammer
 * removed '.' from control files
 *
 * Revision 2.3  2003-10-01 02:13:14+05:30  Cprogrammer
 * added files for shared maildirs
 *
 * Revision 2.2  2003-06-07 20:21:38+05:30  Cprogrammer
 * added file .deliveryCount
 *
 * Revision 2.1  2002-10-16 23:38:26+05:30  Cprogrammer
 * function to return true for a IndiMail system file/directory
 *
 */
#include <stdio.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: skip_system_files.c,v 2.5 2014-04-18 17:27:47+05:30 Cprogrammer Stab mbhangui $";
#endif

int
skip_system_files(char *filename)
{
	char           *system_files[] = {
		".Trash",
		".current_size",
		"domain",
		"QuotaWarn",
		"vfilter",
		"folder.dateformat",
		"noprefilt",
		"nopostfilt",
		"BulkMail",
		"deliveryCount", 
		"maildirfolder",
		"maildirsize",
		"core",
		"sqwebmail",
		"courier",
		"shared-maildirs",
		"shared-timestamp",
		"shared-folders",
		0,
	};
	char          **ptr;
	int             len;

	for (ptr = system_files; ptr && *ptr; ptr++)
	{
		len = strlen(*ptr);
		if (!memcmp(filename, *ptr, len))
			return (1);
	}
	return (0);
}

void
getversion_skip_system_files_c()
{
	printf("%s\n", sccsid);
}
