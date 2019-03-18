/*
 * $Log: renameuser.c,v $
 * Revision 2.17  2019-03-16 19:27:13+05:30  Cprogrammer
 * removed mailing list code
 *
 * Revision 2.16  2016-01-28 16:11:11+05:30  Cprogrammer
 * use maildirquota for vadduser argument
 *
 * Revision 2.15  2010-08-08 20:17:13+05:30  Cprogrammer
 * use configurable users per level
 *
 * Revision 2.14  2010-03-02 08:18:13+05:30  Cprogrammer
 * changed Username xxx@yyy does not exist to xxx@yyy: No such user
 *
 * Revision 2.13  2009-10-14 20:45:26+05:30  Cprogrammer
 * use strtoll() instead of atol()
 *
 * Revision 2.12  2008-08-02 09:08:36+05:30  Cprogrammer
 * use new function error_stack
 *
 * Revision 2.11  2008-06-13 10:14:13+05:30  Cprogrammer
 * compile code to rename entries in userquota table only if ENABLE_AUTH_LOGGING defined
 *
 * Revision 2.10  2008-06-03 19:46:28+05:30  Cprogrammer
 * new argument mdahost added to vadduser
 *
 * Revision 2.9  2008-05-28 16:37:41+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.8  2004-11-22 09:51:49+05:30  Cprogrammer
 * do not abort if update fails and table does not exist
 *
 * Revision 2.7  2004-11-16 11:11:43+05:30  Cprogrammer
 * typo fix
 *
 * Revision 2.6  2004-09-21 23:38:47+05:30  Cprogrammer
 * change for actFlag to vauth_adduser()
 *
 * Revision 2.5  2003-07-02 18:29:41+05:30  Cprogrammer
 * corrected logic for case where table is not present
 *
 * Revision 2.4  2003-06-18 23:38:47+05:30  Cprogrammer
 * take care of renaming entries in all user tables
 * vfilter, valias, userquota, lastauth, indimail, indibak
 *
 * Revision 2.3  2002-06-26 03:17:47+05:30  Cprogrammer
 * correction for non-distributed code
 *
 * Revision 2.2  2002-05-10 10:09:02+05:30  Cprogrammer
 * return error if new username exists
 *
 * Revision 2.1  2002-05-06 22:15:56+05:30  Cprogrammer
 * function to rename users
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: renameuser.c,v 2.17 2019-03-16 19:27:13+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <ctype.h>
#include <unistd.h>
#define XOPEN_SOURCE = 600
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mysqld_error.h>

int
vrenameuser(char *oldUser, char *oldDomain, char *newUser, char *newDomain)
{
	char            oldDir[MAX_PW_DIR], SqlBuf[SQL_BUF_SIZE];
	char           *real_domain;
#ifdef VALIAS
	char           *ptr1, *ptr2;
	char            User[MAX_BUFF], oldEmail[MAX_BUFF], newEmail[MAX_BUFF], tmp_domain[MAX_BUFF];
#endif
#ifdef CLUSTERED_SITE
	char            TmpBuf[MAX_BUFF];
	char           *ptr, *mailstore;
#endif
	struct passwd  *pw;
	int             err, inactive_flag;

	if (!oldUser || !*oldUser || !newUser || !*newUser || !oldDomain || !*oldDomain 
		|| !newDomain || !*newDomain || !isalpha((int) *newUser))
	{
		fprintf(stderr, "Illegal Username/domain\n");
		return (-1);
	}
	if (slen(newUser) > MAX_PW_NAME || slen(newDomain) > MAX_PW_DOMAIN)
	{
		fprintf(stderr, "Name Too Long (name > %d or domain > %d)\n", MAX_PW_NAME, MAX_PW_DOMAIN);
		return (-1);
	}
	if (!(real_domain = vget_real_domain(oldDomain)))
	{
		fprintf(stderr, "Domain %s does not exist\n", oldDomain);
		return (-1);
	} else
	if (!vget_assign(real_domain, 0, 0, 0, 0))
	{
		fprintf(stderr, "Domain %s does not exist\n", real_domain);
		return (-1);
	}
#ifdef CLUSTERED_SITE
	if ((err = is_distributed_domain(real_domain)) == -1)
	{
		fprintf(stderr, "Unable to verify %s as a distributed domain\n", real_domain);
		return (-1);
	} else
	if (err == 1)
	{
		if (open_master())
		{
			fprintf(stderr, "vrenameuser: Failed to Open Master Db\n");
			return (-1);
		}
		snprintf(TmpBuf, MAX_BUFF, "%s@%s", newUser, real_domain);
		if (is_user_present(newUser, real_domain))
		{
			fprintf(stderr, "New Username %s@%s exists\n", newUser, real_domain);
			return (-1);
		}
		snprintf(TmpBuf, MAX_BUFF, "%s@%s", oldUser, real_domain);
		if ((mailstore = findhost(TmpBuf, 0)) != (char *) 0)
		{
			if ((ptr = strrchr(mailstore, ':')) != (char *) 0)
				*ptr = 0;
			for(;*mailstore && *mailstore != ':';mailstore++);
			mailstore++;
		} else
		{
			if (userNotFound)
				fprintf(stderr, "%s@%s: No such user\n", oldUser, real_domain);
			else
				fprintf(stderr, "Error connecting to db\n");
			return (-1);
		}
		if (!islocalif(mailstore))
		{
			fprintf(stderr, "%s@%s not local (mailstore %s)\n", oldUser, real_domain, mailstore);
			return (-1);
		}
	}
#endif
	if ((pw = vauth_getpw(newUser, real_domain)))
	{
		fprintf(stderr, "New Username %s@%s exists\n", newUser, real_domain);
		return (-1);
	} else
	if (!(pw = vauth_getpw(oldUser, real_domain)))
	{
		if (userNotFound)
			fprintf(stderr, "%s@%s: No such user\n", oldUser, real_domain);
		else
			fprintf(stderr, "Error connecting to db\n");
		return (-1);
	} else
		scopy(oldDir, pw->pw_dir, MAX_PW_DIR);
	inactive_flag = is_inactive;
	if (!(real_domain = vget_real_domain(newDomain)))
	{
		fprintf(stderr, "Domain %s does not exist\n", newDomain);
		return (-1);
	} else
	if (!vget_assign(real_domain, 0, 0, 0, 0))
	{
		fprintf(stderr, "Domain %s does not exist\n", real_domain);
		return (-1);
	}
	encrypt_flag = 1;
	if ((err = vadduser(newUser, newDomain, 0, pw->pw_passwd, pw->pw_gecos,
		pw->pw_shell, 0, 1, !inactive_flag)) == -1)
	{
		error_stack(stderr, 0);
		return (-1);
	} else
	if (!(pw = vauth_getpw(newUser, newDomain)))
	{
		if (userNotFound)
			fprintf(stderr, "%s@%s: No such user\n", newUser, newDomain);
		else
			fprintf(stderr, "Error connecting to db\n");
		return (-1);
	}
	create_flag = !access(oldDir, F_OK);
	if (create_flag && MoveFile(oldDir, pw->pw_dir))
	{
		fprintf(stderr, "renameuser: MoveFile: %s\n", strerror(errno));
		return (-1);
	}
#ifdef ENABLE_AUTH_LOGGING
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority lastauth set user=\"%s\",domain=\"%s\" where user=\"%s\" and domain=\"%s\"",
		newUser, newDomain, oldUser, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "lastauth", LASTAUTH_TABLE_LAYOUT))
				return (-1);
		} else
		{
			fprintf(stderr, "vrenameuser: lastauth update: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
			return (-1);
		}
	}
	if (inactive_flag)
		return (vdeluser(oldUser, real_domain, 1));
#endif
#ifdef VALIAS
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority valias set alias=\"%s\",domain=\"%s\" where alias=\"%s\" and domain=\"%s\"",
		newUser, newDomain, oldUser, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "valias", VALIAS_TABLE_LAYOUT))
				return (-1);
		} else
		{
			fprintf(stderr, "vrenameuser: alias update: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
			return (-1);
		}
	}
	/*
	 * Replace all occurence of OldDir with NewDir
	 */
	snprintf(oldEmail, sizeof(oldEmail), "%s@%s", oldUser, oldDomain);
	snprintf(newEmail, sizeof(newEmail), "%s@%s", newUser, newDomain);
	for(;;)
	{
		*tmp_domain = 0;
		if (!(ptr1 = valias_select_all(User, tmp_domain, MAX_BUFF)))
			break;
		if (!(ptr2 = replacestr(ptr1, oldEmail, newEmail)))
			continue;
		if (ptr1 == ptr2)
			continue;
		valias_update(User, tmp_domain, ptr1, ptr2);
		free(ptr2);
	}
#endif
#ifdef VFILTER
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority vfilter set emailid=\"%s@%s\" where emailid=\"%s@%s\"",
		newUser, newDomain, oldUser, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "vfilter", FILTER_TABLE_LAYOUT))
				return (-1);
		} else
		{
			fprintf(stderr, "vrenameuser: vfilter update: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
			return (-1);
		}
	}
#endif
#ifdef ENABLE_AUTH_LOGGING
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"update low_priority userquota set user=\"%s\",domain=\"%s\" where user=\"%s\" and domain=\"%s\"",
		newUser, newDomain, oldUser, real_domain);
	if (mysql_query(&mysql[1], SqlBuf))
	{
		if (mysql_errno(&mysql[1]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_LOCAL, "userquota", USERQUOTA_TABLE_LAYOUT))
				return (-1);
		} else
		{
			fprintf(stderr, "vrenameuser: lastauth update: %s: %s\n", SqlBuf, mysql_error(&mysql[1]));
			return (-1);
		}
	}
#endif
	if (vdeluser(oldUser, real_domain, 1))
		return (-1);
	return (0);
}

void
getversion_renameuser_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
