/*
 * $Log: vget_real_domain.c,v $
 * Revision 2.10  2008-11-06 15:38:53+05:30  Cprogrammer
 * added cache reset option
 *
 * Revision 2.9  2008-06-13 10:45:57+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 2.8  2005-12-29 22:53:24+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.7  2005-02-13 21:49:48+05:30  Cprogrammer
 * added environment variales REAL_DOMAINS and ALIAS_DOMAINS for fast domain lookups
 *
 * Revision 2.6  2004-09-20 19:55:50+05:30  Cprogrammer
 * skip comments and blank lines
 *
 * Revision 2.5  2003-02-01 14:12:58+05:30  Cprogrammer
 * use rcpthosts if assign file and host.cntrl are not present
 *
 * Revision 2.4  2002-11-30 09:38:47+05:30  Cprogrammer
 * set permanent error if errno is not ENOENT
 *
 * Revision 2.3  2002-09-11 20:40:13+05:30  Cprogrammer
 * print error for error other than ENOENT
 *
 * Revision 2.2  2002-08-25 22:36:07+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.1  2002-07-05 00:32:00+05:30  Cprogrammer
 * code to implement caching incorporated
 *
 * Revision 1.7  2002-04-03 01:45:05+05:30  Cprogrammer
 * return null if both assign file and host.cntrl are absent
 *
 * Revision 1.6  2002-03-18 19:09:10+05:30  Cprogrammer
 * return null if we can't figure out domain is distributed
 *
 * Revision 1.5  2002-02-24 22:08:52+05:30  Cprogrammer
 * return error when vauth_get_real_domain fails
 *
 * Revision 1.4  2001-12-21 01:12:57+05:30  Cprogrammer
 * check table aliasdomain on central db in case domain is distributed
 *
 * Revision 1.3  2001-11-24 12:22:00+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 11:00:49+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:40+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef	lint
static char     sccsid[] = "$Id: vget_real_domain.c,v 2.10 2008-11-06 15:38:53+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef QUERY_CACHE
static char     _cacheSwitch = 1;
#endif

char *
vget_real_domain(char *domain)
{
	static char     Dir[MAX_BUFF], prevDomainVal[MAX_BUFF], domval[MAX_BUFF];
	char           *ptr, *cptr;
	char            tmpbuf[MAX_BUFF];
	struct stat     statbuf;
	int             len;
	uid_t           uid;
	gid_t           gid;
#ifdef CLUSTERED_SITE
	FILE           *fp;
	char           *qmaildir, *controldir;
	int             ret;
	char            TmpBuf[MAX_BUFF];
#endif

	if (!domain || !*domain)
		return ((char *) 0);
#ifdef QUERY_CACHE
	if (_cacheSwitch && getenv("QUERY_CACHE"))
	{
		if (*prevDomainVal && *domval && !strncmp(domain, prevDomainVal, MAX_BUFF))
			return(domval);
	}
	if (!_cacheSwitch)
		_cacheSwitch = 1;
#endif
	/*
	 * e.g. indimail.org:yahoo.com:hotmail.com
	 */
	if ((ptr = getenv("REAL_DOMAINS")))
	{
		len = slen(domain);
		for (cptr = ptr;*cptr;cptr++)
		{
			if (*cptr == ':')
			{
				if (!memcmp(domain, ptr, len))
				{
					scopy(prevDomainVal, domain, MAX_BUFF);
					scopy(domval, domain, MAX_BUFF);
					return(domval);
				}
				ptr = cptr + 1;
			}
			if (*ptr && !memcmp(domain, ptr, len))
			{
				scopy(prevDomainVal, domain, MAX_BUFF);
				scopy(domval, domain, MAX_BUFF);
				return(domval);
			}
		}
	}
	/*
	 * e.g. satyam.net.in,indimail.org:yahoo.co.in,yahoo.com:msn.com,hotmail.com
	 */
	if ((ptr = getenv("ALIAS_DOMAINS")))
	{
		len = slen(domain);
		for (cptr = ptr;*cptr;cptr++)
		{
			if (*cptr == ':')
			{
				if (!memcmp(domain, ptr, len) && *(ptr + len) == ',')
				{
					scopy(prevDomainVal, domain, MAX_BUFF);
					for (cptr = domval,ptr += len + 1;*ptr && *ptr != ':';*cptr++ = *ptr++);
					*cptr = 0;
					return(domval);
				}
				ptr = cptr + 1;
			}
			if (*ptr && !memcmp(domain, ptr, len) && *(ptr + len) == ',')
			{
				scopy(prevDomainVal, domain, MAX_BUFF);
				for (cptr = domval,ptr += len + 1;*ptr && *ptr != ':';*cptr++ = *ptr++);
				*cptr = 0;
				return(domval);
			}
		}
	}
	if (!vget_assign(domain, tmpbuf, MAX_BUFF, &uid, &gid))
#ifdef CLUSTERED_SITE
	{
		if ((ret = is_distributed_domain(domain)) == -1)
			return((char *) 0);
		else
		if (ret == 1)
		{
			scopy(prevDomainVal, domain, MAX_BUFF);
			scopy(domval, domain, MAX_BUFF);
			return(domval);
		}
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		getEnvConfigStr(&controldir, "CONTROLDIR", "control");
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/host.cntrl", qmaildir, controldir);
		if (access(TmpBuf, F_OK))
		{
			snprintf(tmpbuf, MAX_BUFF, "%s/%s/rcpthosts", qmaildir, controldir);
			if (!(fp = fopen(tmpbuf, "r")))
			{
				fprintf(stderr, "vget_real_domain: fopen: %s: %s\n", tmpbuf, strerror(errno));
				return((char *) 0);
			}
			for (;;)
			{
				if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
					break;
				if ((ptr = strrchr(tmpbuf, '\n')) || (ptr = strchr(tmpbuf, '#')))
					*ptr = 0;
				for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
				if (!*ptr)
					continue;
				if (!strncmp(domain, tmpbuf, MAX_BUFF))
				{
					scopy(prevDomainVal, domain, MAX_BUFF);
					scopy(domval, domain, MAX_BUFF);
					fclose(fp);
					return(domval);
				}
			}
			fclose(fp);
			snprintf(tmpbuf, MAX_BUFF, "%s/%s/morercpthosts", qmaildir, controldir);
			if (!(fp = fopen(tmpbuf, "r")))
			{
				if (errno != ENOENT)
					fprintf(stderr, "vget_real_domain: fopen: %s: %s\n", tmpbuf, strerror(errno));
				return((char *) 0);
			}
			for (;;)
			{
				if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
					break;
				if ((ptr = strrchr(tmpbuf, '\n')))
					*ptr = 0;
				if (!strncmp(domain, tmpbuf, MAX_BUFF))
				{
					scopy(prevDomainVal, domain, MAX_BUFF);
					scopy(domval, domain, MAX_BUFF);
					fclose(fp);
					return(domval);
				}
			}
			fclose(fp);
			return((char *) 0);
		} else
		if (!(ptr = vauth_get_realdomain(domain)))
			return((char *) 0);
		else
		{
			scopy(prevDomainVal, domain, MAX_BUFF);
			scopy(domval, ptr, MAX_BUFF);
			return(ptr);
		}
	}
#else
		return((char *) 0);
#endif
	if (lstat(tmpbuf, &statbuf))
	{
		if (errno != ENOENT)
		{
			fprintf(stderr, "vget_real_domain: %s: %s\n", tmpbuf, strerror(errno));
			userNotFound = 0;
		} else
			userNotFound = 1;
		return((char *) 0);
	}
	if (S_ISLNK(statbuf.st_mode))
	{
		if ((len = readlink(tmpbuf, Dir, MAX_BUFF)) == -1)
			return((char *) 0);
		if (len < MAX_BUFF)
			Dir[len] = 0;
		else
		{
			errno = ENAMETOOLONG;
			return ((char *) 0);
		}
		if ((ptr = strrchr(Dir, '/')) != (char *) 0)
			ptr++;
		else
			ptr = Dir;
		scopy(prevDomainVal, domain, MAX_BUFF);
		scopy(domval, ptr, MAX_BUFF);
		return(ptr);
	}
	scopy(prevDomainVal, domain, MAX_BUFF);
	scopy(domval, domain, MAX_BUFF);
	return(domain);
}

#ifdef QUERY_CACHE
void
vget_real_domain_cache(char cache_switch)
{
	_cacheSwitch = cache_switch;
	return;
}
#endif

void
getversion_vget_real_domain_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
