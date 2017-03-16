/*
 * $Log: vget_assign.c,v $
 * Revision 2.5  2016-05-18 11:47:38+05:30  Cprogrammer
 * use ASSIGNDIR for users/cdb
 *
 * Revision 2.4  2009-02-09 12:25:26+05:30  Cprogrammer
 * added prototype for cdb_seek()
 *
 * Revision 2.3  2008-11-06 15:38:43+05:30  Cprogrammer
 * added cache reset option
 *
 * Revision 2.2  2005-12-29 22:53:20+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.1  2003-10-23 13:23:07+05:30  Cprogrammer
 * changed fread() to read()
 *
 * Revision 1.3  2001-11-24 12:21:56+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:45+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:38+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: vget_assign.c,v 2.5 2016-05-18 11:47:38+05:30 Cprogrammer Exp mbhangui $";
#endif

extern int      cdb_seek(int, unsigned char *, unsigned int, int *);
/*
 * get uid, gid, dir from users/assign with caching
 */
#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

char *
vget_assign(char *domain, char *dir, int dir_len, uid_t *uid, gid_t *gid)
{
	int             dlen, i, fs;
	char           *ptr, *assigndir, *tmpstr, *tmpbuf1;
	char            tmpbuf2[MAX_BUFF];
	static char    *in_domain = NULL;
	static char    *in_dir = NULL;
	static int      in_domain_size = 0;
	static int      in_dir_size = 0;
	static int      in_uid = -1;
	static int      in_gid = -1;

	if (!domain || !*domain)
		return (NULL);
	lowerit(domain);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE"))
	{
		if (in_domain_size && in_domain  && in_dir && 
			!strncmp(in_domain, domain, in_domain_size + 1))
		{
			if (uid)
				*uid = in_uid;
			if (gid)
				*gid = in_gid;
			if (dir)
				scopy(dir, in_dir, dir_len);
			return (in_dir);
		}
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	snprintf(tmpbuf2, MAX_BUFF, "%s/cdb", assigndir);
	if ((fs = open(tmpbuf2, O_RDONLY)) == -1)
	{
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			*dir = 0;
		return (NULL);
	}
	in_domain_size = slen(domain);
	if (!(in_domain = realloc(in_domain, in_domain_size + 1)))
	{
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			*dir = 0;
		return (NULL);
	}
	scopy(in_domain, domain, in_domain_size + 1);
	if (!(tmpstr = malloc(in_domain_size + 3)))
	{
		if (uid)
			*uid = -1;
		if (gid)
			*gid = -1;
		if (dir)
			*dir = 0;
		return (NULL);
	}
	strcpy(tmpstr, "!");
	strncat(tmpstr, domain, in_domain_size + 1);
	strncat(tmpstr, "-", 1);
	if ((i = cdb_seek(fs, (unsigned char *) tmpstr, in_domain_size + 2, &dlen)) == 1)
	{
		if (!(tmpbuf1 = (char *) malloc(dlen + 1)))
		{
			close(fs);
			free(tmpstr);
			if (uid)
				*uid = -1;
			if (gid)
				*gid = -1;
			if (dir)
				*dir = 0;
			return (NULL);
		}
		i = read(fs, tmpbuf1, dlen);
		tmpbuf1[dlen] = 0;
		for(ptr = tmpbuf1; *ptr; ptr++);
		ptr++;
		in_uid = atoi(ptr);
		if (uid)
			*uid = in_uid;
		for(; *ptr; ptr++);
		ptr++;
		in_gid = atoi(ptr);
		if (gid)
			*gid = in_gid;
		for(; *ptr; ptr++);
		ptr++;
		in_dir_size = slen(ptr) + 1;
		if (!(in_dir = realloc(in_dir, in_dir_size)))
		{
			close(fs);
			free(tmpstr);
			if (uid)
				*uid = -1;
			if (gid)
				*gid = -1;
			if (dir)
				*dir = 0;
			return (NULL);
		}
		if (dir)
			scopy(dir, ptr, dir_len);
		scopy(in_dir, ptr, in_dir_size);
		free(tmpstr);
		free(tmpbuf1);
		close(fs);
		return (in_dir);
	} 
	close(fs);
	free(tmpstr);
	if (uid)
		*uid = -1;
	if (gid)
		*gid = -1;
	if (dir)
		*dir = 0;
	return (NULL);
}

#ifdef QUERY_CACHE
void
vget_assign_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif

void
getversion_vget_assign_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
