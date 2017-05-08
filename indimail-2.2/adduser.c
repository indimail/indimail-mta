/*
 * $Log: adduser.c,v $
 * Revision 2.24  2016-01-28 00:04:45+05:30  Cprogrammer
 * maildirquota specification for -q option to vadduser
 *
 * Revision 2.23  2012-04-22 13:56:44+05:30  Cprogrammer
 * use 64bit integer for quota
 *
 * Revision 2.22  2010-08-08 20:39:41+05:30  Cprogrammer
 * take users_per_level from domain directory
 *
 * Revision 2.21  2010-08-08 13:00:06+05:30  Cprogrammer
 * made users_per_level configurable
 *
 * Revision 2.20  2009-05-27 16:02:57+05:30  Cprogrammer
 * allow char in username as per RFC-5321
 *
 * Revision 2.19  2009-03-06 19:57:05+05:30  Cprogrammer
 * set errno = EEXIST if user exists
 *
 * Revision 2.18  2009-02-06 11:35:46+05:30  Cprogrammer
 * fixed potential buffer overflow
 *
 * Revision 2.17  2008-09-14 19:39:53+05:30  Cprogrammer
 * removed setgid, setuid calls
 * added error_checks
 *
 * Revision 2.16  2008-09-12 22:39:29+05:30  Cprogrammer
 * exit if setting of uid to indimail's uid fails
 *
 * Revision 2.15  2008-09-08 09:20:48+05:30  Cprogrammer
 * removed mysql_escape
 *
 * Revision 2.14  2008-08-02 09:11:54+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.13  2008-06-03 19:41:41+05:30  Cprogrammer
 * added mdahost argument to vadduser
 *
 * Revision 2.12  2008-05-28 14:51:28+05:30  Cprogrammer
 * removed sqwebmail
 *
 * Revision 2.11  2005-12-29 22:38:55+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.10  2004-09-21 23:34:30+05:30  Cprogrammer
 * added actFlag to vauth_adduser() to create user as active or inactive
 * depending on the value of actFlag
 *
 * Revision 2.9  2004-07-03 23:01:54+05:30  Cprogrammer
 * display the offending non-alphanumeric character
 *
 * Revision 2.8  2004-07-02 09:48:37+05:30  Cprogrammer
 * Use ALLOWCHARS for a list of valid chars for a username
 *
 * Revision 2.7  2004-05-17 14:00:28+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.6  2004-05-17 01:08:00+05:30  Cprogrammer
 * force flag argument added to addusercntrl()
 *
 * Revision 2.5  2004-05-17 00:47:07+05:30  Cprogrammer
 * added hostid argument to addusercntrl()
 *
 * Revision 2.4  2003-03-30 23:34:38+05:30  Cprogrammer
 * allow user starting with numerals
 *
 * Revision 2.3  2002-08-04 23:59:03+05:30  Cprogrammer
 * added mysql_escape() for username
 *
 * Revision 2.2  2002-08-03 04:25:24+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.1  2002-04-23 12:09:54+05:30  Cprogrammer
 * invalid return code if user existed
 *
 * Revision 1.13  2002-02-25 13:53:45+05:30  Cprogrammer
 * defined SqlBuf only when CLUSTERED_SITE is defined
 *
 * Revision 1.12  2002-02-23 20:19:30+05:30  Cprogrammer
 * added code to remove from local indimail if user is found to be in hostcntrl (to take care of RACE condition)
 *
 * Revision 1.11  2001-12-22 18:05:41+05:30  Cprogrammer
 * changed error string for open_master failure
 *
 * Revision 1.10  2001-12-14 13:49:29+05:30  Cprogrammer
 * prevent race condition where entry gets added in indimail and
 * another process adds entry for the same user in hostcntrl
 *
 * Revision 1.9  2001-12-11 11:30:16+05:30  Cprogrammer
 * open master for updates
 *
 * Revision 1.8  2001-12-03 01:01:37+05:30  Cprogrammer
 * dynamic assignment of uid and gid for user indimail
 *
 * Revision 1.7  2001-11-29 20:48:16+05:30  Cprogrammer
 * added conditional compilation for Distributed architecture
 *
 * Revision 1.6  2001-11-29 13:17:05+05:30  Cprogrammer
 * removed a bug where vget_assign was not getting called if domain was not distributed
 *
 * Revision 1.5  2001-11-29 00:30:50+05:30  Cprogrammer
 * added distributed code
 *
 * Revision 1.3  2001-11-24 12:17:47+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:36+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:52+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#define ALLOWCHARS              " .!#$%&'*+-/=?^_`{|}~\""

#ifndef	lint
static char     sccsid[] = "$Id: adduser.c,v 2.24 2016-01-28 00:04:45+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * E-mail addresses are formally defined in RFC 5322 (mostly section 3.4.1) and to a lesser degree RFC 5321. An e-mail address
 * is a string of a subset of ASCII characters separated into 2 parts by an "@" (at sign), a "local-part" and a domain, that is,
 * local-part@domain.
 *
 * The "local-part" of an e-mail address can be up to 64 characters and the domain name a maximum of 255 characters. Clients may
 * attempt to use larger objects, but they must be prepared for the server to reject them if they cannot be handled by it.
 *
 * The local-part of the e-mail address may use any of these ASCII characters:
 *
 *   * Uppercase and lowercase English letters (a-z, A-Z)
 *   * Digits 0 through 9
 *   * Characters ! # $ % & ' * + - / = ? ^ _ ` { | } ~
 *   * Character . (dot, period, full stop) provided that it is not the first or last character, and provided also that it does
 *     not appear two or more times consecutively.
 *
 * Additionally, quoted-strings (ie: "John Doe"@example.com) are permitted, thus allowing characters that would otherwise be
 * prohibited, however they do not appear in common practice. RFC 5321 also warns that "a host that expects to receive mail SHOULD
 * avoid defining mailboxes where the Local-part requires (or uses) the Quoted-string form".
 *
 * The local-part is case sensitive, so "jsmith@example.com" and "JSmith@example.com" may be delivered to different people. This
 * practice is discouraged by RFC 5321. However, only the authoritative mail servers for a domain may make that decision. The only
 * exception is for a local-part value of "postmaster" which is case insensitive, and should be forwarded to the server's administrator.
 *
 * Notwithstanding the addresses permitted by these standards, some systems impose more restrictions on e-mail addresses, both in
 * e-mail addresses created on the system and in e-mail addresses to which messages can be sent. Hotmail, for example, only allows
 * creation of e-mail addresses using alphanumerics, dot (.), underscore (_) and hyphen (-), and will not allow sending mail to any
 * e-mail address containing ! # $ % * / ? | ^ { } ` ~[2]. The domain name is much more restricted: it must match the requirements
 * for a hostname, consisting of letters, digits, hyphens and dots. In addition, the domain may be an IP address literal, surrounded
 * by square braces, such as jsmith@[192.168.2.1] (this is rarely seen, except in spam).
 *
 * The informational RFC 3696 written by the author of RFC 5321 explains the details in a readable way, with a few minor errors noted
 * in the 3696 errata.
 */
int
vadduser(char *username, char *domain, char *mdahost, char *password,
		 char *gecos, char *quota, int max_users_per_level, int apop, int actFlag)
{
	char            Dir[MAX_BUFF], Crypted[MAX_BUFF], tmpbuf[MAX_BUFF];
	char           *tmpstr, *dir, *ptr, *allow_chars;
	uid_t           uid;
	gid_t           gid;
	FILE           *fp;
	int             ulen, u_level = 0;
#ifdef CLUSTERED_SITE
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;
#endif

	if (!username || !*username || !isalnum((int) *username))
	{
		error_stack(stderr, "Illegal Username\n");
		return (-1);
	}
	if ((ulen = slen(username)) > MAX_PW_NAME || slen(domain) > MAX_PW_DOMAIN || 
			slen(gecos) > MAX_PW_GECOS || slen(password) > MAX_PW_PASS)
	{
		error_stack(stderr, "Name too long\n");
		return (-1);
	}
	if (*username == '.' || username[ulen - 1] == '.')
	{
		error_stack(stderr, "Trailing/Leading periods not allowed\n");
		return (-1);
	}
	getEnvConfigStr(&allow_chars, "ALLOWCHARS", ALLOWCHARS);
	for (ptr = username;*ptr;ptr++)
	{
		if (*ptr == ':')
		{
			error_stack(stderr, "':' not allowed in names\n");
			return (-1);
		}
		if (*ptr == '.' && *(ptr + 1) == '.')
		{
			error_stack(stderr, "successive periods not allowed in local-part See RFC-5322\n",  *ptr);
			return (-1);
		}
		if (strchr(allow_chars, *ptr))
			continue;
		if (!isalnum((int) *ptr))
		{
			error_stack(stderr, "[%c] not allowed in local-part See RFC-5322\n",  *ptr);
			return (-1);
		}
		if (isupper((int) *ptr))
			*ptr = tolower((int) *ptr);
	}
	if (domain && *domain)
	{
		for (ptr = domain;*ptr;ptr++)
		{
			if (*ptr == ':')
			{
				error_stack(stderr, "':' not allowed in names\n");
				return (-1);
			} else
			if (isupper((int) *ptr))
				*ptr = tolower(*ptr);
		}
		apop = 1;
#ifdef CLUSTERED_SITE
		if ((err = is_distributed_domain(domain)) == -1)
		{
			error_stack(stderr, "unable to verify %s as a distributed domain\n", domain);
			return(-1);
		}
		if (err == 1)
		{
			if (open_master())
			{
				error_stack(stderr, "vadduser: Failed to open Master Db\n");
				return (-1);
			}
			apop = ADD_FLAG;
			err = is_user_present(username, domain);
			if (err == 1)
			{
				error_stack(stderr, "username %s@%s exists\n", username, domain);
				return (-1);
			} else
			if (err == -1)
			{
				error_stack(stderr, "Auth Db Error\n");
				return (-1);
			}
		} else
		if (vauth_getpw(username, domain))
		{
			error_stack(stderr, "username %s@%s exists\n", username, domain);
			errno = EEXIST;
			return (-1);
		}
#else
		if (vauth_getpw(username, domain))
		{
			error_stack(stderr, "username %s@%s exists\n", username, domain);
			errno = EEXIST;
			return (-1);
		}
#endif
		if (!(tmpstr = vget_assign(domain, Dir, MAX_BUFF, &uid, &gid)))
		{
			error_stack(stderr, "Domain %s does not exist\n", domain);
			return (-1);
		}
	} else /*- if domain is null */
	{
		if (vget_assign(username, Dir, MAX_BUFF, &uid, &gid))
		{
			error_stack(stderr, "username %s exists\n", username);
			return (-1);
		}
		GetIndiId(&uid, &gid);
	}
	snprintf(tmpbuf, MAX_BUFF, "%s/.users_per_level", Dir);
	if (!(fp = fopen(tmpbuf, "r")))
	{
		if (errno != ENOENT)
		{
			error_stack(stderr, "%s: %s\n", tmpbuf, strerror(errno));
			return (-1);
		}
	} else
	{
		if (fscanf(fp, "%d", &u_level) != 1)
		{
			error_stack(stderr, "invalid domain users per level\n");
			return (-1);
		}
		fclose(fp);
	}
	/*
	 * check gecos for : characters - bad 
	 */
	if (gecos && *gecos && strchr(gecos, ':'))
	{
		error_stack(stderr, "':' not allowed in names\n");
		return (-1);
	}
	umask(INDIMAIL_UMASK); /*- This function always succeeds according to man */
	if (!(dir = make_user_dir(username, domain, uid, gid,
		!max_users_per_level ? u_level : max_users_per_level)))
	{
		error_stack(stderr, "make user dir failed\n");
		return (-1);
	}
	if (!*dir)
		dir = (char *) 0;
	if (domain && *domain)
	{
		mkpasswd3(password, Crypted, MAX_BUFF);
		ptr = vauth_adduser(username, domain, Crypted, gecos, dir, quota, apop, actFlag);
		if (!ptr || !*ptr)
			return(-1);
#ifdef CLUSTERED_SITE
		if (apop == ADD_FLAG)
		{
			/*
			 * Get hostid of the Local machine 
			 */
			if (mdahost && *mdahost)
			{
				if (!(ptr = vauth_gethostid(mdahost)))
				{
					error_stack(stderr, "adduser-vauth_gethostid: Unable to get hostid for %s\n", mdahost);
					return (-1);
				}
			} else /* avoid looping of mails */
			if (!(ptr = get_local_hostid()))
			{
				error_stack(stderr, "adduser-gethostid: could not get local hostid: %s\n", strerror(errno));
				return (-1);
			}
			/*
			 * This can happen under a race condition where some other process 
			 * adds the same user to hostcntrl. In this case the entries added
			 * locally should be removed.
			 */
			if ((err = addusercntrl(username, domain, ptr, Crypted, 0)) == 2)
			{
				snprintf(SqlBuf, SQL_BUF_SIZE, 
					"delete low_priority from %s where pw_name = \"%s\" and pw_domain = \"%s\"", 
					default_table, username, domain);
				if (mysql_query(&mysql[1], SqlBuf))
				{
					error_stack(stderr, "adduser-delvpomail: mysql error: %s\n", 
						mysql_error(&mysql[1]));
					return(1);
				}
#ifdef ENABLE_AUTH_LOGGING
				snprintf(SqlBuf, SQL_BUF_SIZE, 
						"delete low_priority from lastauth where user=\"%s\" and domain=\"%s\"", 
						username, domain);
				if (mysql_query(&mysql[1], SqlBuf))
				{
					error_stack(stderr, "adduser-lastauth: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
					return(1);
				}
#endif
				error_stack(stderr, "username %s@%s exists\n", username, domain);
				return (-1);
			} else
			if (!err)
				vauth_updateflag(username, domain, 1); /*- Reset the pw_uid flag to 1 */
			/* - don't bother for if (err) as hostsync should take care */
		}
#endif
	} else
	if (add_user_assign(username, dir))
		return(-1);
	return (0);
}

void
getversion_adduser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
