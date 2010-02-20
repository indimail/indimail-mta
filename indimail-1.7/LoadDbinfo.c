/*
 * $Log: LoadDbinfo.c,v $
 * Revision 2.37  2010-02-19 16:16:44+05:30  Cprogrammer
 * mysql_port was wrongly getting initialized to zero
 *
 * Revision 2.36  2010-02-19 13:13:35+05:30  Cprogrammer
 * use defaults for mysql_port and mysql_socket
 *
 * Revision 2.35  2010-02-18 22:43:45+05:30  Cprogrammer
 * accept mysql host in host:user:password:socket/port parameter
 *
 * Revision 2.34  2009-09-27 12:10:19+05:30  Cprogrammer
 * set 644 perm for mcdfile
 *
 * Revision 2.33  2009-07-23 13:36:44+05:30  Cprogrammer
 * BUG - double fclose
 *
 * Revision 2.32  2009-02-18 21:28:22+05:30  Cprogrammer
 * check return value of fscanf
 *
 * Revision 2.31  2009-02-18 09:07:35+05:30  Cprogrammer
 * fixed fgets warning
 *
 * Revision 2.30  2009-02-09 12:24:56+05:30  Cprogrammer
 * added prototype for localDbinfo()
 *
 * Revision 2.29  2009-02-06 11:38:16+05:30  Cprogrammer
 * ignore return value of fgets/fscanf
 *
 * Revision 2.28  2008-09-08 09:48:04+05:30  Cprogrammer
 * formatted long line
 *
 * Revision 2.27  2008-06-13 09:36:25+05:30  Cprogrammer
 * corrected compilation warning if CLUSTERED_SITE was not defined
 *
 * Revision 2.26  2008-05-28 16:36:46+05:30  Cprogrammer
 * removed USE_MYSQL
 *
 * Revision 2.25  2007-12-22 00:15:52+05:30  Cprogrammer
 * loadDbinfoTotal() scope changed to global
 *
 * Revision 2.24  2006-03-29 11:18:17+05:30  Cprogrammer
 * initialize variable containing the total no of MySQL servers
 *
 * Revision 2.23  2005-12-29 22:45:45+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.22  2003-10-29 23:22:04+05:30  Cprogrammer
 * skip local dbinfo entries when updating dbinfo table
 * initialize dbinfo structure as non-clustered when using local entries
 *
 * Revision 2.21  2003-10-29 13:19:26+05:30  Cprogrammer
 * added missing fclose()
 *
 * Revision 2.20  2003-10-28 00:23:35+05:30  Cprogrammer
 * mark dbinfo entries allocated by localDbinfo() as local
 *
 * Revision 2.19  2003-10-27 22:37:37+05:30  Cprogrammer
 * initialize unassigned DBINFO structures in localDbinfo()
 *
 * Revision 2.18  2003-10-26 18:42:06+05:30  Cprogrammer
 * skip alias domains
 *
 * Revision 2.17  2003-10-24 22:41:09+05:30  Cprogrammer
 * overhauled code for getting local domain entries
 * writedbinfo() made visible
 *
 * Revision 2.16  2003-10-18 00:16:14+05:30  Cprogrammer
 * added code for future loading of non dbinfo domains
 *
 * Revision 2.15  2003-10-16 00:01:39+05:30  Cprogrammer
 * BUG allocation for relayhost was not being done (wrong else condition)
 *
 * Revision 2.14  2003-01-16 17:25:50+05:30  Cprogrammer
 * set errno for correct display of error by LoadDbInfo_TXT
 *
 * Revision 2.13  2002-12-29 18:58:58+05:30  Cprogrammer
 * use local definitions or environment variables to load relayhosts variable if hostcntrl is absent
 * This is to allow the distributed code to work without the MCD control file
 *
 * Revision 2.12  2002-12-16 20:24:19+05:30  Cprogrammer
 * changed owner of mcd file to indimail
 *
 * Revision 2.11  2002-12-08 19:04:13+05:30  Cprogrammer
 * added more information in error messages
 *
 * Revision 2.10  2002-10-27 21:22:13+05:30  Cprogrammer
 * used generic function create_table()
 *
 * Revision 2.9  2002-10-23 15:32:21+05:30  Cprogrammer
 * do not check table dbinfo if host.master is missing
 *
 * Revision 2.8  2002-08-25 22:48:44+05:30  Cprogrammer
 * made control dir configurable
 *
 * Revision 2.7  2002-08-03 04:24:01+05:30  Cprogrammer
 * mysql string fix
 *
 * Revision 2.6  2002-06-26 03:25:16+05:30  Cprogrammer
 * changes for non-distributed code
 *
 * Revision 2.5  2002-05-15 01:35:19+05:30  Cprogrammer
 * added verbose messages
 *
 * Revision 2.4  2002-05-14 23:50:22+05:30  Cprogrammer
 * set total only if not null
 *
 * Revision 2.3  2002-05-13 03:12:40+05:30  Cprogrammer
 * added writedbinfo()
 * correction made for correct updation of timestamps
 *
 * Revision 2.2  2002-05-13 02:26:01+05:30  Cprogrammer
 * added code to sync dbinfo between mysql and filesystem
 *
 * Revision 2.1  2002-05-11 15:23:05+05:30  Cprogrammer
 * added perror to display error opening mcdFile
 *
 * Revision 1.2  2002-04-10 15:19:48+05:30  Cprogrammer
 * removed redundant variables
 *
 * Revision 1.1  2002-04-07 13:41:50+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: LoadDbinfo.c,v 2.37 2010-02-19 16:16:44+05:30 Cprogrammer Exp mbhangui $";
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <utime.h>
#include <mysqld_error.h>

static DBINFO **LoadDbInfo_TXT_internal(int *);
static DBINFO **localDbinfo(int *, DBINFO ***);

static int      _total;

int
loadDbinfoTotal()
{
	return(_total);
}

#ifdef CLUSTERED_SITE
static int      delete_dbinfo_rows(char *);

DBINFO **
LoadDbInfo_TXT(int *total)
{
	char            SqlBuf[SQL_BUF_SIZE], mcdFile[MAX_BUFF], TmpBuf[MAX_BUFF];
	struct stat     statbuf;
	int             num_rows, idx, err = 0, sync_file = 0, sync_mcd = 0;
	DBINFO        **ptr, **relayhosts;
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l, file_time = 0l;
	char           *qmaildir, *mcdfile, *controldir;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	snprintf(TmpBuf, MAX_BUFF, "%s/%s/host.master", qmaildir, controldir);
	if (total)
		_total = *total = 0;
	if (access(TmpBuf, F_OK))
		return(LoadDbInfo_TXT_internal(total));
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		snprintf(mcdFile, MAX_BUFF, "%s", mcdfile);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	if (stat(mcdFile, &statbuf))
	{
		if (verbose)
			fprintf(stderr, "LoadDbInfo_TXT: stat: %s: %s\n", mcdFile, strerror(errno));
		sync_file = 1;
		file_time = 0l;
	} else
	{
		file_time = statbuf.st_mtime;
		if (verbose)
			printf("File UNIX  %s Modification Time %s", mcdFile, ctime(&file_time));
	}
	if (open_master())
	{
		if (sync_file)
		{
			fprintf(stderr, "Failed to open Master Db\n");
			return((DBINFO **) 0);
		} else
			return(LoadDbInfo_TXT_internal(total));
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "select UNIX_TIMESTAMP(timestamp) from dbinfo where filename=\"%s\"", mcdFile);
	if (mysql_query(&mysql[0], SqlBuf) && ((err = mysql_errno(&mysql[0])) != ER_NO_SUCH_TABLE))
	{
		fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		if (sync_file)
			return((DBINFO **) 0);
		else
			return(LoadDbInfo_TXT_internal(total));
	}
	if (err == ER_NO_SUCH_TABLE)
	{
		if (create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT))
		{
			if (sync_file)
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		sync_mcd = 1;
	} else 
	{
		if (!(res = mysql_store_result(&mysql[0])))
		{
			(void) fprintf(stderr, "LoadDbInfo_TXT: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			if (sync_file)
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		if (!(num_rows = mysql_num_rows(res)))
		{
			if (sync_file)
			{
				mysql_free_result(res);
				errno = ENOENT;
				return((DBINFO **) 0);
			}
			sync_mcd = 1;
			row = mysql_fetch_row(res);
			mysql_free_result(res);
		} else
		{
			for (mcd_time = 0l;(row = mysql_fetch_row(res));)
			{
				mtime = atol(row[0]);
				if (mtime > mcd_time)
					mcd_time = mtime;
			}
			mysql_free_result(res);
			if (verbose)
			{
				printf("File MySQL %s Modification Time %s", mcdFile, ctime(&mcd_time));
				if (mcd_time == file_time)
					printf("Nothing to update\n");
			}
			if (mcd_time == file_time)
				return(LoadDbInfo_TXT_internal(total));
			else
			if (mcd_time > file_time)
			{
				sync_file = 1;
				sync_mcd = 0;
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
		if (verbose)
			printf("Updating Table dbinfo\n");
		if (!(relayhosts = LoadDbInfo_TXT_internal(total)))
		{
			perror("LoadDbInfo_TXT_internal");
			return((DBINFO **) 0);
		}
		if (delete_dbinfo_rows(mcdFile))
			return(relayhosts);
		for (err = 0, ptr = relayhosts;(*ptr);ptr++)
		{
			if ((*ptr)->isLocal)
				continue;
			snprintf(SqlBuf, SQL_BUF_SIZE, "replace low_priority into dbinfo \
				(filename, domain, distributed, server, mdahost, port, dbname, user, passwd, timestamp) \
				values (\"%s\", \"%s\", %d, \"%s\", \"%s\", %d, \"%s\", \"%s\", \"%s\", FROM_UNIXTIME(%ld) + 0)", 
				mcdFile, (*ptr)->domain, (*ptr)->distributed, (*ptr)->server, (*ptr)->mdahost, (*ptr)->port, 
				(*ptr)->database, (*ptr)->user, (*ptr)->password, file_time);
			if (mysql_query(&mysql[0], SqlBuf))
			{
				fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				err = 1;
				continue;
			}
		}
		return(relayhosts);
	} else
	if (sync_file)
	{
		if (verbose)
			printf("Updating File %s\n", mcdFile);
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority domain, distributed, server, mdahost, port, dbname, user, passwd, timestamp \
			from dbinfo where filename=\"%s\"", mcdFile);
		if (mysql_query(&mysql[0], SqlBuf))
		{
			fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			if (access(mcdFile, F_OK))
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		if (!(res = mysql_store_result(&mysql[0])))
		{
			(void) fprintf(stderr, "LoadDbInfo_TXT: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			if (access(mcdFile, F_OK))
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		if (!(num_rows = mysql_num_rows(res)))
		{
			mysql_free_result(res);
			fprintf(stderr, "LoadDbInfo_TXT: No rows selected\n");
			if (access(mcdFile, F_OK))
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		if(total)
			_total = (*total += num_rows);
		if (!(relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (num_rows + 1))))
		{
			perror("malloc");
			mysql_free_result(res);
			if (access(mcdFile, F_OK))
				return((DBINFO **) 0);
			else
				return(LoadDbInfo_TXT_internal(total));
		}
		for (ptr = relayhosts, idx = 0;(row = mysql_fetch_row(res));idx++, ptr++)
		{
			if (!((*ptr) = (DBINFO *) malloc(sizeof(DBINFO))))
			{
				perror("malloc");
				free(relayhosts);
				mysql_free_result(res);
				if (access(mcdFile, F_OK))
					return((DBINFO **) 0);
				else
					return(LoadDbInfo_TXT_internal(total));
			}
			scopy((*ptr)->domain, row[0], DBINFO_BUFF);
			(*ptr)->distributed = atoi(row[1]);
			scopy((*ptr)->server, row[2], DBINFO_BUFF);
			scopy((*ptr)->mdahost, row[3], DBINFO_BUFF);
			(*ptr)->port = atoi(row[4]);
			scopy((*ptr)->database, row[5], DBINFO_BUFF);
			scopy((*ptr)->user, row[6], DBINFO_BUFF);
			scopy((*ptr)->password, row[7], DBINFO_BUFF);
			(*ptr)->isLocal = 0;
		}
		if (writedbinfo(relayhosts, mcd_time))
			fprintf(stderr, "LoadDbInfo_TXT: writedbinfo failed\n");
		(*ptr) = (DBINFO *) 0;
		mysql_free_result(res);
		return(relayhosts);
	} else
	if (verbose)
		printf("Nothing to update\n");
	return(LoadDbInfo_TXT_internal(total));
}

static int
delete_dbinfo_rows(char *filename)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master())
	{
		fprintf(stderr, "Failed to open Master Db\n");
		return(-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from dbinfo where filename=\"%s\"", filename);
	if (mysql_query(&mysql[0], SqlBuf))
	{
		fprintf(stderr, "delete_dbinfo: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return(-1);
	}
	return(0);
}

int
writedbinfo(DBINFO **rhostsptr, time_t mtime)
{
	char            mcdFile[MAX_BUFF];
	char           *qmaildir, *mcdfile, *controldir;
	uid_t           uid;
	gid_t           gid;
	FILE           *fp;
	struct utimbuf  ubuf;
	DBINFO        **ptr;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		snprintf(mcdFile, MAX_BUFF, "%s", mcdfile);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	if (!rhostsptr)
		return(1);
	if (!(fp = fopen(mcdFile, "w")))
	{
		fprintf(stderr, "writedbinfo: %s: %s\n", mcdFile, strerror(errno));
		return(1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = indimailuid;
	gid = indimailgid;
	if (fchown(fileno(fp), uid, gid))
		fprintf(stderr, "fchown: %s: %s\n", mcdFile, strerror(errno));
	else
	if (fchmod(fileno(fp), INDIMAIL_QMAIL_MODE))
		fprintf(stderr, "fchmod: %s: %s\n", mcdFile, strerror(errno));
	for (ptr = rhostsptr;(*ptr);ptr++)
	{
		if ((*ptr)->isLocal)
			continue;
		fprintf(fp, "domain   %-28s %d\n", (*ptr)->domain, (*ptr)->distributed);
		fprintf(fp, "server   %s\n", (*ptr)->server);
		fprintf(fp, "mdahost  %s\n", (*ptr)->mdahost);
		fprintf(fp, "port     %d\n", (*ptr)->port);
		fprintf(fp, "database %s\n", (*ptr)->database);
		fprintf(fp, "user     %s\n", (*ptr)->user);
		fprintf(fp, "pass     %s\n\n", (*ptr)->password);
	}
	fclose(fp);
	ubuf.actime = time(0);
	ubuf.modtime = mtime;
	if (utime(mcdFile, &ubuf))
		fprintf(stderr, "utime: %s: %s\n", mcdFile, strerror(errno));
	return(0);
}
#else
DBINFO **
LoadDbInfo_TXT(int *total)
{
	return(LoadDbInfo_TXT_internal(total));
}
#endif

static DBINFO **
LoadDbInfo_TXT_internal(int *total)
{
	char            mcdFile[MAX_BUFF], dombuf[DBINFO_BUFF];
	char           *qmaildir, *mcdfile, *controldir, *ptr; 
	int             ret, count, items, distributed;
	DBINFO        **relayhosts, **rhostsptr;
	char            buffer[MAX_BUFF], dummy1[MAX_BUFF], dummy2[MAX_BUFF];
	FILE           *fp;

	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		scopy(mcdFile, mcdfile, MAX_BUFF);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", qmaildir, controldir, mcdfile);
	count = 0;
	relayhosts = (DBINFO **) 0;
	if (!(fp = fopen(mcdFile, "r")))
		return(localDbinfo(total, &relayhosts));
	else
	{
		for (;;)
		{
			if (!fgets(buffer, MAX_BUFF, fp))
			{
				if (feof(fp))
					break;
				fprintf(stderr, "loadDbInfo_TXT_internal: fgets: %s\n", strerror(errno));
				return ((DBINFO **) 0);
			}
			if ((ptr = strchr(buffer, '#')))
				*ptr = '\0';
			for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
			if (!*ptr)
				continue;
			if (!(strncmp(ptr, "server", 6)))
				count++;
		}
	}
	if (!count)
	{
		fclose(fp);
		return(localDbinfo(total, &relayhosts));
	} else
	if (total)
		_total = (*total += count);
	if (!(relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (count + 1))))
	{
		perror("malloc");
		fclose(fp);
		return ((DBINFO **) 0);
	}
	rewind(fp);
	for (*dombuf = 0, items = 0, count = 1, rhostsptr = relayhosts;; count++)
	{
		if (!fgets(buffer, MAX_BUFF, fp))
		{
			if (feof(fp))
				break;
			fprintf(stderr, "loadDbInfo_TXT_internal: fgets: %s\n", strerror(errno));
		}
		if (buffer[strlen(buffer) - 1] != '\n')
		{
			fprintf(stderr, "Line No %d in %s Exceeds %d chars\n", count, mcdFile, MAX_BUFF);
			fclose(fp);
			free(relayhosts);
			return ((DBINFO **) 0);
		}
		if ((ptr = strchr(buffer, '#')))
			*ptr = '\0';
		for (ptr = buffer; *ptr && isspace((int) *ptr); ptr++);
		if (!*ptr)
			continue;
		if ((ret = sscanf(buffer, "%s %s", dummy1, dummy2)) != 2)
		{
			fprintf(stderr, "Line No %d in %s has no value line is %s", count, mcdFile, buffer);
			fclose(fp);
			free(relayhosts);
			return ((DBINFO **) 0);
		}
		if (!strncmp(dummy1, "domain", 6))
		{
			scopy(dombuf, dummy2, DBINFO_BUFF);
			if ((ret = sscanf(buffer, "%s %s %d", dummy1, dummy2, &distributed)) != 3)
				distributed = 0;
			continue;
		} else
		if (!strncmp(dummy1, "count", 5))
			continue;
		else
		if (!strncmp(dummy1, "table", 5))
			continue;
		else
		if (!strncmp(dummy1, "time", 4))
			continue;
		if (!strncmp(dummy1, "server", 6))
		{
			if (items)
			{
				fprintf(stderr, "Line Preceding %d in %s is incomplete\n", count, mcdFile);
				fclose(fp);
				free(relayhosts);
				errno = EINVAL;
				return ((DBINFO **) 0);
			}
			if (!((*rhostsptr) = (DBINFO *) malloc(sizeof(DBINFO))))
			{
				perror("malloc");
				fclose(fp);
				free(relayhosts);
				return ((DBINFO **) 0);
			}
			items++;
			(*rhostsptr)->isLocal = 0;
			scopy((*rhostsptr)->server, dummy2, DBINFO_BUFF);
		} else
		if ((*rhostsptr))
		{
			if (!strncmp(dummy1, "mdahost", 7))
			{
				items++;
				scopy((*rhostsptr)->mdahost, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "port", 4))
			{
				items++;
				(*rhostsptr)->port = atoi(dummy2);
			} else
			if (!strncmp(dummy1, "database", 8))
			{
				items++;
				scopy((*rhostsptr)->database, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "user", 4))
			{
				items++;
				scopy((*rhostsptr)->user, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "pass", 4))
			{
				items++;
				scopy((*rhostsptr)->password, dummy2, DBINFO_BUFF);
			} else
			{
				fprintf(stderr, "Invalid Syntax at line %d  file %s - [%s]", count, mcdFile, buffer);
				fclose(fp);
				free(relayhosts);
				return ((DBINFO **) 0);
			}
			if (items == 6)
			{
				if (*dombuf)
				{
					scopy((*rhostsptr)->domain, dombuf, DBINFO_BUFF);
					(*rhostsptr)->distributed = distributed;
				}
				else
				{
					scopy((*rhostsptr)->domain, "unknown domain", DBINFO_BUFF);
					(*rhostsptr)->distributed = -1;
				}
				rhostsptr++;
				items = 0;
				*dombuf = 0;
			}
		}
	}
	fclose(fp);
	if (items)
	{
		fprintf(stderr, "Incomplete Structure in file %s\n", mcdFile);
		free(relayhosts);
		errno = EINVAL;
		return ((DBINFO **) 0);
	}
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
#if 1
	if (!(rhostsptr = localDbinfo(total, &relayhosts)))
		fprintf(stderr, "LoadDbInfo_TXT_internal: No local Dbinfo\n");
	else
		relayhosts = rhostsptr;
#endif
	_total = *total;
	return (relayhosts);
}

static DBINFO **
localDbinfo(int *total, DBINFO ***rhosts)
{
	FILE           *fp, *mfp;
	char           *mysqlhost, *mysql_user = 0, *mysql_passwd = 0; 
	char           *mysql_database = 0, *qmaildir, *controldir, *ptr, *domain;
	char           *localhost, *mysql_socket = 0, *mysql_port = 0;
	char            host_path[MAX_BUFF], mysqlhost_buf[MAX_BUFF], TmpBuf[MAX_BUFF];
	int             count, field_count, found;
	DBINFO        **relayhosts, **rhostsptr, **tmpPtr;

	relayhosts = *rhosts;
	getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
	snprintf(TmpBuf, MAX_BUFF, "%s/users/assign", qmaildir);
	if (!(fp = fopen(TmpBuf, "r")))
	{
		fprintf(stderr, "fopen: %s: %s\n", TmpBuf, strerror(errno));
		return ((DBINFO **) 0);
	}
	/*- +testindi.com-:testindi.com:508:508:/var/indimail/domains/testindi.com:-:: -*/
	for (count = 0;;)
	{
		if (!fgets(TmpBuf, MAX_BUFF, fp))
		{
			if (feof(fp))
				break;
			fprintf(stderr, "localDbinfo: fgets: %s\n", strerror(errno));
		}
		if (!(ptr = strchr(TmpBuf, ':')))
			continue;
		if (relayhosts)
		{
			ptr++;
			domain = ptr;
			for (;*ptr && *ptr != ':';ptr++);
			if (*ptr)
				*ptr = 0;
			if (!isvirtualdomain(domain))
				continue;
			if (is_alias_domain(domain))
				continue;
			for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++)
			{
				if (!strncmp((*tmpPtr)->domain, domain, DBINFO_BUFF))
				{
					found = 1;
					break;
				}
			}
			if (found)
				continue;
		}
		count++;
	}
	if (!count)
	{
		fclose(fp);
		return(*rhosts);
	}
	*mysqlhost_buf = 0;
	if ((mysqlhost = (char *) getenv("MYSQL_HOST")) != (char *) 0)
		scopy(mysqlhost_buf, mysqlhost, MAX_BUFF);
	getEnvConfigStr(&controldir, "CONTROLDIR", "control");
	if (snprintf(host_path, MAX_BUFF, "%s/%s/host.mysql", qmaildir, controldir) == -1)
		host_path[MAX_BUFF - 1] = 0;
	if (!*mysqlhost_buf && !access(host_path, F_OK))
	{
		if (!(mfp = fopen(host_path, "r")))
			scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
		else
		{
			if (!fgets(mysqlhost_buf, MAX_BUFF - 2, mfp))
				scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
			else
			{
				if ((ptr = strrchr(mysqlhost_buf, '\n')))
					*ptr = 0;
			}
			fclose(mfp);
		}
	} else
	if (!*mysqlhost_buf)
		scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
	mysqlhost = mysqlhost_buf;
	for (field_count = 0,ptr = mysqlhost_buf;*ptr;ptr++)
	{
		if (*ptr == ':')
		{
			*ptr = 0;
			switch (field_count++)
			{
			case 0: /*- mysql user */
				if (*(ptr + 1))
					mysql_user = ptr + 1;
			case 1: /*- mysql passwd */
				if (*(ptr + 1))
					mysql_passwd = ptr + 1;
			case 2: /*- mysql socket/port */
				if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
					mysql_socket = ptr + 1;
				else
				if (*(ptr + 1))
					mysql_port = ptr + 1;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "MYSQL_USER", MYSQL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "MYSQL_PASSWD", MYSQL_PASSWD);
#if 0
	if (!mysql_socket)
		getEnvConfigStr(&mysql_socket, "MYSQL_SOCKET", MYSQL_SOCKET);
	if (!mysql_port)
		getEnvConfigStr(&mysql_port, "MYSQL_VPORT", MYSQL_VPORT);
#else
	if (!mysql_port)
		mysql_port = "0";
#endif
	getEnvConfigStr(&mysql_database, "MYSQL_DATABASE", MYSQL_DATABASE);
	if (total)
	{
		relayhosts = (DBINFO **) realloc(relayhosts, sizeof(DBINFO *) * (*total + count + 1));
		rhostsptr = relayhosts + *total;
		for (tmpPtr = rhostsptr;tmpPtr < relayhosts + *total + count + 1;tmpPtr++)
			*tmpPtr = (DBINFO *) 0;
		(*total) += count;
	} else
	{
		relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (count + 1));
		rhostsptr = relayhosts;
	} 
	if (!relayhosts)
	{
		perror("malloc");
		fclose(fp);
		return((DBINFO **) 0);
	}
	rewind(fp);
	for (;;)
	{
		if (!fgets(TmpBuf, MAX_BUFF, fp))
		{
			if (feof(fp))
				break;
			fprintf(stderr, "localDbinfo: fgets: %s\n", strerror(errno));
		}
		if (!(ptr = strchr(TmpBuf, ':')))
			continue;
		ptr++;
		domain = ptr;
		for (;*ptr && *ptr != ':';ptr++);
		if (*ptr)
			*ptr = 0;
		if (!isvirtualdomain(domain))
			continue;
		if (is_alias_domain(domain))
			continue;
		for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++)
		{
			if (!strncmp((*tmpPtr)->domain, domain, DBINFO_BUFF))
			{
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		if (!((*rhostsptr) = (DBINFO *) malloc(sizeof(DBINFO))))
		{
			perror("malloc");
			free(relayhosts);
			fclose(fp);
			return ((DBINFO **) 0);
		}
		/*- Should check virtual domains and smtproutes */
		(*rhostsptr)->isLocal = 1;
		if (!(localhost = get_local_ip()))
			localhost = "localhost";
		scopy((*rhostsptr)->mdahost, localhost, DBINFO_BUFF);
		scopy((*rhostsptr)->server, mysqlhost, DBINFO_BUFF);
		scopy((*rhostsptr)->domain, domain, DBINFO_BUFF);
		(*rhostsptr)->port = atoi(mysql_port);
		scopy((*rhostsptr)->database, mysql_database, DBINFO_BUFF);
		scopy((*rhostsptr)->user, mysql_user, DBINFO_BUFF);
		scopy((*rhostsptr)->password, mysql_passwd, DBINFO_BUFF);
		(*rhostsptr)->distributed = 0;
		rhostsptr++;
		(*rhostsptr) = (DBINFO *) 0;
	}
	fclose(fp);
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
	return(relayhosts);
}

void
getversion_LoadDbinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
