/*
 * $Log: GetPrefix.c,v $
 * Revision 2.5  2008-05-21 15:50:26+05:30  Cprogrammer
 * added string.h
 *
 * Revision 2.4  2008-01-11 20:40:40+05:30  Cprogrammer
 * use /root filesystem when pathToFilesystem() returns '/'
 *
 * Revision 2.3  2005-12-29 22:44:52+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2004-06-20 16:35:27+05:30  Cprogrammer
 * rename ISOCOR_BASE_PATH to BASE_PATH
 *
 * Revision 2.1  2002-08-11 00:23:54+05:30  Cprogrammer
 * getactualpath() and code to parse mtab to get the filsystem separated into separate functions
 *
 * Revision 1.8  2001-12-19 16:26:25+05:30  Cprogrammer
 * Removed Dirname() as a static function
 *
 * Revision 1.7  2001-12-03 09:39:28+05:30  Cprogrammer
 * getactualpath was not returning to the original directory on success
 *
 * Revision 1.6  2001-12-03 00:11:23+05:30  Cprogrammer
 * incorporated code for reading /etc/mnttab on sun
 *
 * Revision 1.5  2001-12-02 20:18:40+05:30  Cprogrammer
 * removed unecessary argument domain
 *
 * Revision 1.4  2001-12-02 18:43:34+05:30  Cprogrammer
 * GetPrefix to generate prefix based on filesystem on which the user lies
 *
 * Revision 1.3  2001-11-24 12:16:54+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:14+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <ctype.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: GetPrefix.c,v 2.5 2008-05-21 15:50:26+05:30 Cprogrammer Stab mbhangui $";
#endif
char           *
GetPrefix(char *user, char *path)
{
	char           *ptr, *suffix_ptr, *base_path;
	int             ch;
	static char     PathPrefix[MAX_BUFF];

	if (!user || !*user)
		return (" ");
	if (path && *path)
		base_path = path;
	else
		getEnvConfigStr(&base_path, "BASE_PATH", BASE_PATH);
	ch = tolower(*user);
	if (ch >= 'a' && ch <= 'e')
		suffix_ptr = "A2E";
	else
	if (ch >= 'f' && ch <= 'k')
		suffix_ptr = "F2K";
	else
	if (ch >= 'l' && ch <= 'p')
		suffix_ptr = "L2P";
	else
	if (ch >= 'q' && ch <= 's')
		suffix_ptr = "Q2S";
	else
		suffix_ptr = "T2Z";
	if(!(ptr = pathToFilesystem(base_path)))
		return((char *) 0);
	if (!strncmp(ptr, "/", 2))
		ptr = "root";
	snprintf(PathPrefix, MAX_BUFF, "%s_%s", ptr, suffix_ptr);
	for (ptr = PathPrefix; *ptr; ptr++)
		if (*ptr == '/')
			*ptr = '_';
	return (PathPrefix);
}

void
getversion_GetPrefix_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
