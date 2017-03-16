/*
 * $Log: LoadBMF.c,v $
 * Revision 2.17  2017-03-13 14:03:55+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.16  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.15  2010-03-30 12:55:40+05:30  Cprogrammer
 * fixed Invalid TIMESTAMP: Internal Bug problem
 *
 * Revision 2.14  2008-11-06 15:37:08+05:30  Cprogrammer
 * initialized es_opt
 *
 * Revision 2.13  2008-10-29 11:17:44+05:30  Cprogrammer
 * added disable_mysql_escape
 *
 * Revision 2.12  2008-09-08 09:46:54+05:30  Cprogrammer
 * undef mysql_query definition
 *
 * Revision 2.11  2008-05-28 16:36:43+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.10  2005-12-29 22:45:37+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.9  2003-12-23 20:04:06+05:30  Cprogrammer
 * spamdb functionality
 *
 * Revision 2.8  2003-06-22 10:52:09+05:30  Cprogrammer
 * removed unused variables
 *
 * Revision 2.7  2003-02-01 22:51:29+05:30  Cprogrammer
 * added option to update badrcptto
 *
 * Revision 2.6  2002-12-29 02:22:28+05:30  Cprogrammer
 * extensive changes for synchronization
 *
 * Revision 2.5  2002-11-04 12:31:34+05:30  Cprogrammer
 * removed duplicate printing of 'Updating Table'
 *
 * Revision 2.4  2002-10-27 21:21:57+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.3  2002-10-24 01:58:36+05:30  Cprogrammer
 * bug fix - mysql_perror used for mysql[0]
 *
 * Revision 2.2  2002-10-23 15:31:46+05:30  Cprogrammer
 * file was not getting closed
 *
 * Revision 2.1  2002-10-21 02:06:04+05:30  Cprogrammer
 * function to load entries into badmailfrom, spam table
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: LoadBMF.c,v 2.17 2017-03-13 14:03:55+05:30 Cprogrammer Exp mbhangui $";
#endif

#ifdef CLUSTERED_SITE
static char   **LoadBMF_internal(int *, char *);
static time_t   BMFTimestamp(int, char *);

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mysqld_error.h>

/*
 * synchronizes table badmailfrom/badrcptto/spamdb with control file
 * "badmailfrom", "badrcptto", "spamdb" or any other spam format file
 */
char **
LoadBMF(int *total, char *bmf)
{
	char            SqlBuf[SQL_BUF_SIZE], badmailfrom[MAX_BUFF];
	struct stat     statbuf;
	int             num_rows, err = 0, sync_file = 0, sync_mcd = 0, badmail_flag;
	MYSQL_RES      *res = 0;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l, file_time = 0l;
	char           *sysconfdir, *controldir;
	struct utimbuf  ubuf;
	FILE           *fp;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (total)
		*total = 0;
	if (*controldir == '/')
		snprintf(badmailfrom, MAX_BUFF, "%s/%s", controldir, bmf);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(badmailfrom, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, bmf);
	}
	if (stat(badmailfrom, &statbuf))
	{
		sync_file = 1;
		file_time = 0l;
	} else
	{
		file_time = statbuf.st_mtime;
		if (verbose)
			printf("File  UNIX  %40s Modification Time %s", badmailfrom, ctime(&file_time));
	}
	if (open_master())
	{
		if (sync_file)
		{
			fprintf(stderr, "LoadBMF: Failed to open Master Db\n");
			return ((char **) 0);
		} else
			return (LoadBMF_internal(total, bmf));
	}
	if (!strncmp(bmf, "badmailfrom", 12) || !strncmp(bmf, "badrcptto", 10) || !strncmp(bmf, "spamdb", 7))
		badmail_flag = 1;
	else
		badmail_flag = 0;
	if (badmail_flag)
		snprintf(SqlBuf, SQL_BUF_SIZE, "select email, UNIX_TIMESTAMP(timestamp) from %s", bmf);
	else
		snprintf(SqlBuf, SQL_BUF_SIZE, "select email, spam_count, UNIX_TIMESTAMP(timestamp) from spam");
	if (mysql_query(&mysql[0], SqlBuf) && ((err = mysql_errno(&mysql[0])) != ER_NO_SUCH_TABLE))
	{
		fprintf(stderr, "LoadBMF: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		if (sync_file)
			return ((char **) 0);
		else
			return (LoadBMF_internal(total, bmf));
	}
	if (err == ER_NO_SUCH_TABLE)
	{
		if (create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
			badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT))
		{
			if (sync_file)
				return ((char **) 0);
			else
				return (LoadBMF_internal(total, bmf));
		}
		sync_mcd = 1;
	} else
	{
		if (!(res = mysql_store_result(&mysql[0])))
		{
			(void) fprintf(stderr, "LoadBMF: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			if (sync_file)
				return ((char **) 0);
			else
				return (LoadBMF_internal(total, bmf));
		}
		if (!(num_rows = mysql_num_rows(res)))
		{
			if (sync_file)
			{
				mysql_free_result(res);
				return ((char **) 0);
			}
			sync_mcd = 1;
			mysql_free_result(res);
			res = 0;
		} else
		{
			for (mcd_time = 0l;(row = mysql_fetch_row(res));)
			{
				if (badmail_flag)
					mtime = atol(row[1]);
				else
					mtime = atol(row[2]);
				if (mtime > mcd_time)
					mcd_time = mtime;
			}
			if (verbose)
			{
				printf("Table MySQL %40s Modification Time %s", badmail_flag ? bmf : "spam", ctime(&mcd_time));
				if (mcd_time == file_time)
					printf("Nothing to update\n");
			}
			if (mcd_time == file_time)
			{
				mysql_free_result(res);
				return (LoadBMF_internal(total, bmf));
			} else
			if (mcd_time > file_time)
			{
				sync_file = 1;
				sync_mcd = 0;
				if ((err = UpdateSpamTable(bmf)) > 0) /*- Reload entries from mysql if this happens */
				{
					if (verbose)
						printf("Reloading Table %s\n", badmail_flag ? bmf : "spam");
					mysql_free_result(res);
					if (mysql_query(&mysql[0], SqlBuf))
					{
						fprintf(stderr, "LoadBMF: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
						return ((char **) 0);
					}
					if (!(res = mysql_store_result(&mysql[0])))
					{
						(void) fprintf(stderr, "LoadBMF: mysql_store_result: %s\n", mysql_error(&mysql[0]));
						return ((char **) 0);
					}
					if (!(num_rows = mysql_num_rows(res))) /*- should never happen */
					{
						mysql_free_result(res);
						return ((char **) 0);
					}
				}
			} else
			if (mcd_time < file_time)
			{
				sync_file = 0;
				sync_mcd = 1;
			}
		}
	}
	if (sync_mcd)
	{
		if (res)
			mysql_free_result(res);
		if ((err = UpdateSpamTable(bmf)) == -1)
			return ((char **) 0);
		else
		{
			if (verbose)
				printf("Syncing time of %s with Table %s\n", badmailfrom, bmf);
			if (!err && (file_time = BMFTimestamp(badmail_flag, bmf)) == -1)
			{
				fprintf(stderr, "LoadBMF: Invalid TIMESTAMP: Internal BUG\n");
				return ((char **) 0);
			}
			ubuf.actime = ubuf.modtime = (err ? time(0) : file_time);
			if (ubuf.actime && utime(badmailfrom, &ubuf))
				fprintf(stderr, "LoadBMF: utime: %s: %s\n", badmailfrom, strerror(errno));
			return (LoadBMF_internal(total, bmf));
		}
	} else
	if (sync_file && res)
	{
		if (verbose)
			printf("Updating File %s\n", badmailfrom);
		if (!(fp = fopen(badmailfrom, "w")))
		{
			fprintf(stderr, "LoadBMF: %s: %s\n", badmailfrom, strerror(errno));
			mysql_free_result(res);
			return ((char **) 0);
		}
		mysql_data_seek(res, 0);
		for (;(row = mysql_fetch_row(res));)
		{
			if (badmail_flag)
				fprintf(fp, "%s\n", row[0]);
			else
				fprintf(fp, "%s %s\n", row[0], row[1]);
		}
		mysql_free_result(res);
		fclose(fp);
		ubuf.actime = time(0);
		ubuf.modtime = mcd_time;
		if (utime(badmailfrom, &ubuf))
			fprintf(stderr, "LoadBMF: utime: %s: %s\n", badmailfrom, strerror(errno));
	} else
	if (verbose)
		printf("Nothing to update\n");
	return (LoadBMF_internal(total, bmf));
}

/*
 * loads entry from file defined by environment variable BADMAILFROM. If not defined, the qmail
 * control file badmailfrom is used.
 * After successful updation, utime of the file is updated to the current time
 */
int
UpdateSpamTable(char *bmf)
{
	char            SqlBuf[SQL_BUF_SIZE], badmailfrom[MAX_BUFF];
	char           *sysconfdir, *controldir;
	int             badmail_flag, err, es_opt = 0;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(badmailfrom, MAX_BUFF, "%s/%s", controldir, bmf);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(badmailfrom, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, bmf);
	}
	if (access(badmailfrom, F_OK))
	{
		fprintf(stderr, "UpdateSpamTable: %s: %s\n", badmailfrom, strerror(errno));
		return (-1);
	}
	if (verbose)
		printf("Updating Table %s\n", bmf);
	if (!strncmp(bmf, "badmailfrom", 12) || !strncmp(bmf, "badrcptto", 10) || !strncmp(bmf, "spamdb", 7))
		badmail_flag = 1;
	else
		badmail_flag = 0;
	if (open_master())
	{
		fprintf(stderr, "UpdateSpamTable: Failed to open Master Db\n");
		return (-1);
	}
	if (badmail_flag)
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"LOAD DATA LOW_PRIORITY LOCAL INFILE \"%s\" IGNORE INTO TABLE \
			%s (email)", badmailfrom, bmf);
	else
	{
		es_opt = disable_mysql_escape(1);
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"LOAD DATA LOW_PRIORITY LOCAL INFILE \"%s\" REPLACE INTO TABLE \
			spam fields terminated by ' ' (email, spam_count)", badmailfrom);
	}
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if (mysql_errno(&mysql[0]) == ER_NO_SUCH_TABLE)
		{
			if (create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
				badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT))
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "UpdateSpamTable: mysql_query: %s: %s", SqlBuf, mysql_error(&mysql[0]));
				if (!badmail_flag)
					disable_mysql_escape(es_opt);
				return (-1);
			}
		} else
		{
			if (!badmail_flag)
				disable_mysql_escape(es_opt);
			fprintf(stderr, "UpdateSpamTable: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			return (-1);
		}
	}
	if (!badmail_flag)
		disable_mysql_escape(es_opt);
	if ((err = mysql_affected_rows(&mysql[0])) == -1)
	{
		fprintf(stderr, "UpdateSpamTable: mysql_affected_rows: %s", mysql_error(&mysql[0]));
		return (-1);
	}
	/*
	 * If err is 0, possibility is file has been  updated without any content
	 * being changed.
	 */
	if (err && utime(badmailfrom, 0))
		fprintf(stderr, "UpdateSpamTable: utime: %s: %s\n", badmailfrom, strerror(errno));
	if (verbose)
		printf("%d rows affected\n", err);
	return (err);
}

static char **
LoadBMF_internal(int *total, char *bmf)
{
	FILE           *fp;
	char           *ptr, *sysconfdir, *controldir;
	char            tmpbuf[MAX_BUFF], badmailfrom[MAX_BUFF];
	int             count, len;
	static int      _count;
	static char   **bmfptr;
	static time_t   file_time;
	struct stat     statbuf;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(badmailfrom, MAX_BUFF, "%s/%s", controldir, bmf);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(badmailfrom, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, bmf);
	}
	if (total)
		*total = 0;
	if (stat(badmailfrom, &statbuf))
		return ((char **) 0);
	if (bmfptr && (statbuf.st_mtime == file_time))
	{
		if (total)
			*total = _count;
		return (bmfptr);
	}
	file_time = statbuf.st_mtime;
	if (!(fp = fopen(badmailfrom, "r")))
		return ((char **) 0);
	for (count = 0;;count++)
	{
		if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
	}
	if (total)
		*total = -1;
	_count = -1;
	if (!count)
	{
		fclose(fp);
		return ((char **) 0);
	}
	if (!(bmfptr = (char **) malloc(sizeof(char *) * (count + 1))))
	{
		fprintf(stderr, "LoadBMF_internal: malloc: %s\n", strerror(errno));
		fclose(fp);
		return ((char **) 0);
	}
	rewind(fp);
	for (count = 1;;count++)
	{
		if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
		if (tmpbuf[(len = strlen(tmpbuf)) - 1] != '\n')
		{
			fprintf(stderr, "Line No %d in %s Exceeds %d chars\n", count, badmailfrom, MAX_BUFF);
			fclose(fp);
			free(bmfptr);
			return ((char **) 0);
		}
		if ((ptr = strchr(tmpbuf, '#')))
			*ptr = '\0';
		for (ptr = tmpbuf; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		tmpbuf[len - 1] = 0;
		len = strlen(ptr);
		if (!(bmfptr[count - 1] = (char *) malloc(sizeof(char) * (len + 1))))
		{
			fprintf(stderr, "LoadBMF_internal: malloc: %s\n", strerror(errno));
			fclose(fp);
			free(bmfptr);
			return ((char **) 0);
		}
		scopy(bmfptr[count - 1], ptr, len + 1);
	}
	fclose(fp);
	if (total)
		*total = (count - 1);
	_count = count - 1;
	bmfptr[count - 1] = 0;
	return (bmfptr);
}

static time_t
BMFTimestamp(int badmail_flag, char *bmf)
{
	char            SqlBuf[SQL_BUF_SIZE];
	int             num_rows, err;
	MYSQL_RES      *res = 0;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l;

	snprintf(SqlBuf, SQL_BUF_SIZE, "select UNIX_TIMESTAMP(timestamp) from %s", badmail_flag ? bmf : "spam");
	if (mysql_query(&mysql[0], SqlBuf))
	{
		if ((err = mysql_errno(&mysql[0])) == ER_NO_SUCH_TABLE)
			create_table(ON_MASTER, badmail_flag == 1 ? bmf : "spam",
				badmail_flag == 1 ? BADMAILFROM_TABLE_LAYOUT : SPAM_TABLE_LAYOUT);
		else
			fprintf(stderr, "BMFTimestamp: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	if (!(res = mysql_store_result(&mysql[0])))
	{
		(void) fprintf(stderr, "BMFTimestamp: mysql_store_result: %s\n", mysql_error(&mysql[0]));
		return (-1);
	}
	if (!(num_rows = mysql_num_rows(res)))
	{
		mysql_free_result(res);
		return (0);
	}
	for (mcd_time = 0l;(row = mysql_fetch_row(res));)
	{
		mtime = atol(row[0]);
		if (mtime > mcd_time)
			mcd_time = mtime;
	}
	mysql_free_result(res);
	if (!mcd_time)
		return (-1);
	return (mcd_time);
}
#endif /*- CLUSTERED_SITE */

void
getversion_LoadBMF_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
