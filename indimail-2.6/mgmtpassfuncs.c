/*
 * $Log: mgmtpassfuncs.c,v $
 * Revision 2.25  2019-04-22 07:40:05+05:30  Cprogrammer
 * fixed comparision typo
 *
 * Revision 2.24  2018-09-11 10:41:13+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.23  2017-03-09 23:34:15+05:30  Cprogrammer
 * changed location of pass.dict to /etc/indimail
 *
 * Revision 2.22  2011-10-28 14:16:04+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.21  2011-10-25 20:48:21+05:30  Cprogrammer
 * added status argument to mgmtgetpass() function
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.20  2011-07-29 09:26:13+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.19  2009-09-17 10:33:54+05:30  Cprogrammer
 * fixed problem with adding new user
 *
 * Revision 2.18  2009-02-24 14:06:04+05:30  Cprogrammer
 * Corrected display of disabled users
 *
 * Revision 2.17  2008-09-12 09:57:36+05:30  Cprogrammer
 * use in_crypt() replacement
 *
 * Revision 2.16  2008-09-11 22:46:39+05:30  Cprogrammer
 * use pw_comp for password comparision
 *
 * Revision 2.15  2008-09-08 09:49:41+05:30  Cprogrammer
 * use integer instead of char in sql query
 *
 * Revision 2.14  2008-08-29 16:53:31+05:30  Cprogrammer
 * added error check for mysql_query()
 *
 * Revision 2.13  2008-08-29 14:02:23+05:30  Cprogrammer
 * added md5, sha256, sha516 hash methods
 *
 * Revision 2.12  2008-07-13 19:45:11+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.11  2008-05-28 16:37:05+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.10  2003-09-16 12:31:31+05:30  Cprogrammer
 * added mgmtlist() to list admin users
 *
 * Revision 2.9  2003-07-02 18:26:40+05:30  Cprogrammer
 * return disabled status if row not found
 *
 * Revision 2.8  2002-12-04 02:06:34+05:30  Cprogrammer
 * added option in mgmtpassinfo() to suppress printf
 *
 * Revision 2.7  2002-10-27 21:26:18+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.6  2002-08-03 04:32:22+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.5  2002-08-02 18:18:41+05:30  Cprogrammer
 * added function mgmtadduser()
 *
 * Revision 2.4  2002-07-23 00:06:42+05:30  Cprogrammer
 * added function mgmtpassinfo()
 *
 * Revision 2.3  2002-07-15 19:02:54+05:30  Cprogrammer
 * cached username to prevent mysql connection
 *
 * Revision 2.2  2002-07-15 18:41:09+05:30  Cprogrammer
 * added error messages
 *
 * Revision 2.1  2002-07-15 02:25:13+05:30  Cprogrammer
 * mgmtaccess passwd functions
 *
 */
#include "indimail.h"

#ifndef lint
static char     sccsid[] = "$Id: mgmtpassfuncs.c,v 2.25 2019-04-22 07:40:05+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <mysqld_error.h>

#define SETPAS_MAX_ATTEMPTS 6
#define LOGIN_MAX_ATTEMPTS 3
#define DAILY_MAX_ATTEMPTS 6
#define PASSDICT SYSCONFDIR"/pass.dict"
#include <unistd.h>

int
getpassword(user)
	char           *user;
{
	char           *pwdptr, *passwd;
	char            temp[MAX_BUFF];
	int             count;

	if (isDisabled(user))
		pwdptr = (char *) 0;
	else
		pwdptr = mgmtgetpass(user, 0);
	for (count = 0; count < LOGIN_MAX_ATTEMPTS; count++)
	{
		snprintf(temp, sizeof(temp), "(current %s) Password: ", user);
		passwd = (char *) getpass(temp);
		if (!pwdptr)
		{
			(void) fprintf(stderr, "Login incorrect or you are disabled\n");
			continue;
		}
		if (passwd && *passwd && *pwdptr)
		{
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0))
				return (0);
		}
		updateLoginFailed(user);
		if (isDisabled(user))
			pwdptr = (char *) 0;
		(void) fprintf(stderr, "Login incorrect or you are disabled\n");
	}
	if (pwdptr && *pwdptr)
		(void) ChangeLoginStatus(user, 1);
	fprintf(stderr, "%s you are disabled\n", user);
	return (1);
}

int
updateLoginFailed(user)
	char           *user;
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;
	time_t          tmval;
	struct tm      *tmptr;

	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return (1);
	}
	tmval = time(0);
	tmptr = localtime(&tmval);
	snprintf(SqlBuf, SQL_BUF_SIZE, \
			"update low_priority mgmtaccess set day=%d, attempts=attempts + 1 where user=\"%s\"",
			 tmptr->tm_mday, user);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "updateLoginFailed: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	}
	if (!(err = mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
ChangeLoginStatus(user, status)
	char           *user;
	int             status;
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             err;

	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority mgmtaccess set status=%d where user=\"%s\"", status, user);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "ChangeLoginStatus: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	}
	if (!(err = mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
mgmtlist()
{
	char            SqlBuf[SQL_BUF_SIZE];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return (1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority user from mgmtaccess");
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
			return (0);
		}
		else
			fprintf(stderr, "mgmtlist: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	} 
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "mgmtlist: MySql Store Result: %s\n", mysql_error(&mysql[0]));
		return (1);
	} else
	if (!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return (0);
	} 
	for(;;)
	{
		if (!(row = mysql_fetch_row(res)))
			break;
		printf("%s\n", row[0]);
	}
	mysql_free_result(res);
	return (0);
}

int
isDisabled(user)
	char           *user;
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             status;
	time_t          tmval;
	struct tm      *tmptr;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return (1);
	}
	tmval = time(0);
	tmptr = localtime(&tmval);
	snprintf(SqlBuf, SQL_BUF_SIZE, "select high_priority day,attempts,status from mgmtaccess where user=\"%s\"", user);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
		else
			fprintf(stderr, "isDisabled: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (1);
	} else
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "isDisabled: MySql Store Result: %s\n", mysql_error(&mysql[0]));
		return (1);
	} else
	if (!mysql_num_rows(res))
	{
		mysql_free_result(res);
		return (1);
	} else
	if ((row = mysql_fetch_row(res)))
	{
		if ((tmptr->tm_mday == atoi(row[0])) && (atoi(row[1]) > DAILY_MAX_ATTEMPTS))
			status = 1;
		else
			status = atoi(row[2]);
	} else
		status = 1;
	mysql_free_result(res);
	return (status);
}

int
mgmtpassinfo(char *username, int print_flag)
{
	char            SqlBuf[SQL_BUF_SIZE];
	time_t          tmval;
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!username || !*username)
		return (1);
	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"select high_priority pass, pw_uid, pw_gid, lastaccess, lastupdate, attempts, status from mgmtaccess \
		where user=\"%s\"", username);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT);
			return (1);
		}
		else
			fprintf(stderr, "mgmtgetpass: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	} else
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "MySql Store Result: %s\n", mysql_error(&mysql[0]));
		return (-1);
	} else
	if (!(mysql_num_rows(res)))
	{
		if (print_flag)
			fprintf(stderr, "%s: No such user\n", username);
		userNotFound = 1;
		mysql_free_result(res);
		return (1);
	}
	if (!print_flag)
	{
		mysql_free_result(res);
		return (0);
	}
	if ((row = mysql_fetch_row(res)))
	{
		printf("User        : %s\n", username);
		printf("Pass        : %s\n", row[0]);
		printf("Uid         : %s\n", row[1]);
		printf("Gid         : %s\n", row[2]);
		tmval = atol(row[3]);
		printf("Last Access : %s", ctime(&tmval));
		tmval = atol(row[4]);
		printf("Last Update : %s", ctime(&tmval));
		printf("Attempts    : %d\n", atoi(row[5]));
		printf("Status      : %d (%s)\n", atoi(row[6]), isDisabled(username) ? "Disabled" : "Enabled");
		mysql_free_result(res);
		return (0);
	}
	mysql_free_result(res);
	return (1);
}

int
setpassword(user)
	char           *user;
{
	char            salt[SALTSIZE + 1];
	char           *newpass1, *newpass2, *pwdptr, *passwd, *crypt_pass;
	int             i1, i2, plen;
	time_t          lastupdate;

	pwdptr = mgmtgetpass(user, 0);
	for (i1 = 0; i1 < LOGIN_MAX_ATTEMPTS; i1++)
	{
		passwd = (char *) getpass("Old password: ");
		if (!pwdptr)
		{
			(void) fprintf(stderr, "Login incorrect or you are disabled\n");
			continue;
		}
		if (passwd && *passwd && *pwdptr)
		{
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0))
				break;
		}
		(void) updateLoginFailed(user);
		if (isDisabled(user))
			pwdptr = (char *) 0;
		(void) fprintf(stderr, "Login incorrect or you are disabled\n");
	}
	if (!pwdptr) /* user does not exist */
		return (1);
	if (i1 == LOGIN_MAX_ATTEMPTS)
	{
		(void) ChangeLoginStatus(user, 1);
		return (1);
	}
	newpass1 = newpass2 = (char *) 0;
	for (i1 = 0; i1 < SETPAS_MAX_ATTEMPTS; i1++)
	{
		makesalt(salt, SALTSIZE);
		for (i2 = 1;;i2++)
		{
			if (i2 > SETPAS_MAX_ATTEMPTS)
			{
				(void) fprintf(stderr, "passwd: Too many tries; try again later.\n");
				if (newpass1)
					free(newpass1);
				if (newpass2)
					free(newpass2);
				return (1);
			}
			passwd = (char *) getpass("New password: ");
			if (passwd_policy(passwd))
				continue;
			if (!pw_comp(0, (unsigned char *) pwdptr, 0, (unsigned char *) passwd, 0))
			{
				fprintf(stderr, "Your passwd cannot be same as the previous one\n");
				continue;
			}
			break;
		}
		if (!(crypt_pass = (char *) in_crypt(passwd, salt)))
		{
			fprintf(stderr, "Error with in_crypt() module: %s\n", strerror(errno));
			continue;
		}
		plen = strlen(crypt_pass);
		if (!newpass1 && !(newpass1 = (char *) malloc(sizeof(char) * (plen + 1))))
		{
			perror("malloc");
			continue;
		}
		(void) strcpy(newpass1, crypt_pass);

		passwd = (char *) getpass("Re-enter new password: ");
		if (!(crypt_pass = (char *) in_crypt(passwd, salt)))
		{
			fprintf(stderr, "Error with in_crypt() module: %s\n", strerror(errno));
			continue;
		}
		plen = strlen(crypt_pass);
		if (!newpass2 && !(newpass2 = (char *) malloc(sizeof(char) * (plen + 1))))
		{
			perror("malloc");
			continue;
		}
		(void) strcpy(newpass2, crypt_pass);
		if (!strncmp(newpass1, newpass2, plen + 1))
		{
			if (newpass2)
				free(newpass2);
			break;
		}
		if (i1 < (SETPAS_MAX_ATTEMPTS - 1))
			(void) fprintf(stderr, "They don't match; try again.\n");
		else
		{
			(void) fprintf(stderr, "passwd: Too many tries; try again later.\n");
			if (newpass1)
				free(newpass1);
			if (newpass2)
				free(newpass2);
			return (1);
		}
	}
	lastupdate = (time_t) time(0);
	encrypt_flag = 1;
	mgmtsetpass(user, newpass1, getuid(), getgid(), lastupdate, lastupdate);
	if (newpass1)
		free(newpass1);
	return (0);
}

char           *
mgmtgetpass(char *username, int *status)
{
	char            SqlBuf[SQL_BUF_SIZE];
	static char     _user[MAX_BUFF], mysql_pass[MAX_BUFF];
	MYSQL_RES      *res;
	MYSQL_ROW       row;

	if (!username || !*username)
	{
		fprintf(stderr, "Password incorrect\n");
		return ((char *) 0);
	}
	if (*_user && username && *username && !strncmp(username, _user, MAX_BUFF))
		return (mysql_pass);
	if (open_central_db(0))
	{
		fprintf(stderr, "Unable to open central db\n");
		return ((char *) 0);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, 
		"select high_priority pass,status from mgmtaccess where user=\"%s\"", username);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (!create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT))
				fprintf(stderr, "Password incorrect\n");
			return ((char *) 0);
		} else
			fprintf(stderr, "mgmtgetpass: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return ((char *) 0);
	} else
	if (!(res = mysql_store_result(&mysql[0])))
	{
		fprintf(stderr, "MySql Store Result: %s\n", mysql_error(&mysql[0]));
		return ((char *) 0);
	} else
	if (!(mysql_num_rows(res)))
	{
		mysql_free_result(res);
		fprintf(stderr, "Password incorrect\n");
		return ((char *) 0);
	}
	if ((row = mysql_fetch_row(res)))
	{
		snprintf(mysql_pass, MAX_BUFF, "%s", row[0]);
		if (status)
			*status = atoi(row[1]);
	}
	mysql_free_result(res);
	snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority mgmtaccess set lastaccess=%ld where user=\"%s\"", 
		time(0), username);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "mgmtgetpass: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return ((char *) 0);
	}
	scopy(_user, username, MAX_BUFF);
	return (mysql_pass);
}

int
mgmtsetpass(char *username, char *pass, uid_t uid, gid_t gid, time_t lastaccess, time_t lastupdate)
{
	char            SqlBuf[SQL_BUF_SIZE], crypted[MAX_BUFF];
	time_t          cur_time;
	struct tm      *tmptr;
	int             err;

	if (open_master())
	{
		fprintf(stderr, "mgmtsetpass: Failed to open Master Db\n");
		return (-1);
	}
	if (encrypt_flag)
	{
		scopy(crypted, pass, MAX_BUFF);
	} else
		mkpasswd3(pass, crypted, MAX_BUFF);
	cur_time = time(0);
	tmptr = localtime(&cur_time);
	snprintf(SqlBuf, SQL_BUF_SIZE, "update low_priority mgmtaccess set pass=\"%s\", pw_uid=%d, pw_gid=%d, \
		lastaccess=%ld, lastupdate=%ld, day=%d, attempts=0, status=0 where user=\"%s\"", \
		crypted, uid, gid, cur_time, cur_time, tmptr->tm_mday, username);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return (-1);
			}
		}  else
		{
			fprintf(stderr, "mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	if (!(err = mysql_affected_rows(&mysql[0])) || err == -1)
		return (1);
	return (0);
}

int
mgmtadduser(char *username, char *pass, uid_t uid, gid_t gid, time_t lastaccess, time_t lastupdate)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "mgmtadduser: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE,
		"insert low_priority into mgmtaccess (user, pass) values (\"%s\", \"%s\")",
		username, pass);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, "mgmtaccess", MGMT_TABLE_LAYOUT))
				return (-1);
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				return (-1);
			}
		}  else
		{
			fprintf(stderr, "mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	return (mgmtsetpass(username, pass, uid, gid, lastaccess, lastupdate));
}
#endif

void
getversion_mgmtpassfuncs_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
