/*
 * $Log: LoadDbinfo.c,v $
 * Revision 2.54  2019-04-16 23:03:33+05:30  Cprogrammer
 * return local dbinfo in absence of mcdinfo file
 *
 * Revision 2.53  2019-03-18 23:13:50+05:30  Cprogrammer
 * increment total when no domains found in mcdinfo
 *
 * Revision 2.52  2019-03-18 18:20:58+05:30  Cprogrammer
 * bug in getting total
 *
 * Revision 2.51  2018-09-11 10:36:36+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.50  2018-03-30 09:35:29+05:30  Cprogrammer
 * set socket for local dbinfo structure
 *
 * Revision 2.49  2018-03-27 19:01:13+05:30  Cprogrammer
 * corrected error text
 *
 * Revision 2.48  2018-03-27 17:51:15+05:30  Cprogrammer
 * added documentation
 *
 * Revision 2.47  2018-03-27 12:06:05+05:30  Cprogrammer
 * added use_ssl filed to dbinfo table, mcdinfo control file
 *
 * Revision 2.46  2018-03-27 10:41:07+05:30  Cprogrammer
 * set use_ssl if specified in host.cntrl/host.master
 *
 * Revision 2.45  2018-03-24 22:24:51+05:30  Cprogrammer
 * idented code
 *
 * Revision 2.44  2017-03-13 14:04:14+05:30  Cprogrammer
 * replaced qmaildir with sysconfdir
 *
 * Revision 2.43  2016-05-18 11:48:14+05:30  Cprogrammer
 * use ASSIGNDIR for users/assign
 *
 * Revision 2.42  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.41  2016-01-12 14:21:11+05:30  Cprogrammer
 * set variable _total for dbcount
 *
 * Revision 2.40  2011-07-02 15:13:03+05:30  Cprogrammer
 * fix null dbinfo being returned with no entries in assign file
 *
 * Revision 2.39  2010-05-01 14:11:41+05:30  Cprogrammer
 * initialize fd, last_error, failed_attempts
 *
 * Revision 2.38  2010-02-24 14:54:45+05:30  Cprogrammer
 * allow MYSQL_SOCKET, MYSQL_VPORT variables to override indimail.cnf variables
 *
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
 * writemcdinfo() made visible
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
 * added writemcdinfo()
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
static char     sccsid[] = "$Id: LoadDbinfo.c,v 2.54 2019-04-16 23:03:33+05:30 Cprogrammer Exp mbhangui $";
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
#include <sys/socket.h>
#include <mysqld_error.h>

static DBINFO **loadMCDInfo(int *);
static DBINFO **localDbinfo(int *, DBINFO ***);

static int      _total;

int
loadDbinfoTotal()
{
	return (_total);
}

#ifdef CLUSTERED_SITE
static int      delete_dbinfo_rows(char *);

DBINFO **
LoadDbInfo_TXT(int *total)
{
	char            SqlBuf[SQL_BUF_SIZE + 247], mcdFile[MAX_BUFF], TmpBuf[MAX_BUFF];
	struct stat     statbuf;
	int             num_rows, idx, err = 0, sync_file = 0, sync_mcd = 0, relative;
	DBINFO        **ptr, **relayhosts;
	MYSQL_RES      *res;
	MYSQL_ROW       row;
	time_t          mtime = 0l, mcd_time = 0l, file_time = 0l;
	char           *sysconfdir, *mcdfile, *controldir;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	relative = *controldir == '/' ? 0 : 1;
	if (relative)
		snprintf(TmpBuf, MAX_BUFF, "%s/%s/host.master", sysconfdir, controldir);
	else
		snprintf(TmpBuf, MAX_BUFF, "%s/host.master", controldir);
	if (total)
		_total = *total = 0;
	if (access(TmpBuf, F_OK)) /*- return dbinfo structure loaded from local mcdinfo if host.master is absent */
		return (loadMCDInfo(total));
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		snprintf(mcdFile, MAX_BUFF, "%s", mcdfile);
	else
	if (relative)
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, mcdfile);
	else
		snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
	if (stat(mcdFile, &statbuf)) {
		if (verbose)
			fprintf(stderr, "LoadDbInfo_TXT: stat: %s: %s\n", mcdFile, strerror(errno));
		sync_file = 1;
		file_time = 0l;
	} else {
		file_time = statbuf.st_mtime;
		if (verbose)
			printf("File UNIX  %s Modification Time %s", mcdFile, ctime(&file_time));
	}
	if (open_master()) {
		if (sync_file) { /*- in absense of mcdinfo, we can't proceed further */
			fprintf(stderr, "LoadDbInfo_TXT: Failed to open Master Db\n");
			return ((DBINFO **) 0);
		} else /*- get records from mcdinfo file */
			return (loadMCDInfo(total));
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "select UNIX_TIMESTAMP(timestamp) from dbinfo where filename=\"%s\"", mcdFile);
	if (mysql_query(&mysql[0], SqlBuf) && ((err = mysql_errno(&mysql[0])) != ER_NO_SUCH_TABLE)) {
		fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		if (sync_file) /*- in absense of mcdinfo, we can't proceed further */
			return ((DBINFO **) 0);
		else /*- get records from mcdinfo file */
			return (loadMCDInfo(total));
	}
	if (err == ER_NO_SUCH_TABLE) {
		if (create_table(ON_MASTER, "dbinfo", DBINFO_TABLE_LAYOUT)) {
			if (sync_file) /*- in absense of mcdinfo, we can't proceed further */
				return ((DBINFO **) 0);
			else /*- get records from mcdinfo file */
				return (loadMCDInfo(total));
		}
		sync_mcd = 1;
	} else { /*- figure out if mcdfile or dbinfo needs to be updated */
		if (!(res = mysql_store_result(&mysql[0]))) {
			(void) fprintf(stderr, "LoadDbInfo_TXT: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			if (sync_file)
				return ((DBINFO **) 0);
			else /*- get records from mcdinfo file */
				return (loadMCDInfo(total));
		}
		if (!(num_rows = mysql_num_rows(res))) { /*- dbinfo table is empty */
			if (sync_file) /*- in absense of mcdinfo, we can't proceed further */
				return (loadMCDInfo(total));
			sync_mcd = 1;
			row = mysql_fetch_row(res);
			mysql_free_result(res);
		} else { /*- figure out which is newer - dbinfo or mcdinfo */
			for (mcd_time = 0l;(row = mysql_fetch_row(res));) {
				mtime = atol(row[0]);
				if (mtime > mcd_time)
					mcd_time = mtime; /*- get the time of the newest dbinfo record */
			}
			mysql_free_result(res);
			if (verbose) {
				printf("File MySQL %s Modification Time %s", mcdFile, ctime(&mcd_time));
				if (mcd_time == file_time)
					printf("Nothing to update\n");
			}
			if (mcd_time == file_time) /*- nothing to update */
				return (loadMCDInfo(total));
			else
			if (mcd_time > file_time) {
				sync_file = 1;
				sync_mcd = 0;
			} else
			if (mcd_time < file_time) {
				sync_file = 0;
				sync_mcd = 1;
			}
		}
	}
	if (sync_mcd) {  /*- sync dbinfo table */
		/* 
		 * update dbinfo table with latest modification in mcdfile
		 * and time = file modification time of mcdinfo
		 */
		if (verbose)
			printf("Updating Table dbinfo\n");
		if (!(relayhosts = loadMCDInfo(total))) {
			perror("loadMCDInfo");
			return ((DBINFO **) 0);
		}
		if (delete_dbinfo_rows(mcdFile)) /*- delete from dbinfo records for mcdFile */
			return (relayhosts);
		for (err = 0, ptr = relayhosts;(*ptr);ptr++) {
			if ((*ptr)->isLocal) /*- don't insert dbinfo obtained from localDbinfo() */
				continue;
			snprintf(SqlBuf, sizeof(SqlBuf) - 1, "replace low_priority into dbinfo \
				(filename, domain, distributed, server, mdahost, port, use_ssl, dbname, user, passwd, timestamp) \
				values (\"%s\", \"%s\", %d, \"%s\", \"%s\", %d, %d, \"%s\", \"%s\", \"%s\", FROM_UNIXTIME(%ld) + 0)", 
				mcdFile, (*ptr)->domain, (*ptr)->distributed, (*ptr)->server, (*ptr)->mdahost, (*ptr)->port, 
				(*ptr)->use_ssl, (*ptr)->database, (*ptr)->user, (*ptr)->password, file_time);
			if (mysql_query(&mysql[0], SqlBuf)) {
				fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
				err = 1;
				continue;
			}
		}
		return (relayhosts);
	} else
	if (sync_file) {
		if (verbose)
			printf("Updating File %s\n", mcdFile);
		snprintf(SqlBuf, SQL_BUF_SIZE,
			"select high_priority domain, distributed, server, mdahost, port, use_ssl, dbname, user, passwd, timestamp \
			from dbinfo where filename=\"%s\"", mcdFile);
		if (mysql_query(&mysql[0], SqlBuf)) {
			fprintf(stderr, "LoadDbInfo_TXT: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
			if (access(mcdFile, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (!(res = mysql_store_result(&mysql[0]))) {
			(void) fprintf(stderr, "LoadDbInfo_TXT: mysql_store_result: %s\n", mysql_error(&mysql[0]));
			if (access(mcdFile, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (!(num_rows = mysql_num_rows(res))) {
			mysql_free_result(res);
			fprintf(stderr, "LoadDbInfo_TXT: No rows selected\n");
			if (access(mcdFile, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		if (total)
			_total = (*total += num_rows);
		if (!(relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (num_rows + 1)))) {
			perror("malloc");
			mysql_free_result(res);
			if (access(mcdFile, F_OK)) /*- no records in dbinfo as well as mcdinfo */
				return ((DBINFO **) 0);
			else
				return (loadMCDInfo(total));
		}
		for (ptr = relayhosts, idx = 0;(row = mysql_fetch_row(res));idx++, ptr++) {
			if (!((*ptr) = (DBINFO *) malloc(sizeof(DBINFO)))) {
				perror("malloc");
				free(relayhosts);
				mysql_free_result(res);
				if (access(mcdFile, F_OK)) /*- no records in dbinfo as well as mcdinfo */
					return ((DBINFO **) 0);
				else
					return (loadMCDInfo(total));
			}
			scopy((*ptr)->domain, row[0], DBINFO_BUFF);
			(*ptr)->distributed = atoi(row[1]);
			scopy((*ptr)->server, row[2], DBINFO_BUFF);
			scopy((*ptr)->mdahost, row[3], DBINFO_BUFF);
			(*ptr)->port = atoi(row[4]);
			(*ptr)->socket = (char *) 0;
			(*ptr)->use_ssl = atoi(row[5]);
			scopy((*ptr)->database, row[6], DBINFO_BUFF);
			scopy((*ptr)->user, row[7], DBINFO_BUFF);
			scopy((*ptr)->password, row[8], DBINFO_BUFF);
			(*ptr)->fd = -1;
			(*ptr)->last_error = 0;
			(*ptr)->failed_attempts = 0;
			(*ptr)->isLocal = 0;
		}
 		/* write dbinfo records to mcdinfo and set file modification time to mcd_time */
		if (writemcdinfo(relayhosts, mcd_time))
			fprintf(stderr, "LoadDbInfo_TXT: writemcdinfo failed\n");
		(*ptr) = (DBINFO *) 0;
		mysql_free_result(res);
		return (relayhosts);
	} else
	if (verbose)
		printf("Nothing to update\n");
	return (loadMCDInfo(total));
}

static int
delete_dbinfo_rows(char *filename)
{
	char            SqlBuf[SQL_BUF_SIZE];

	if (open_master()) {
		fprintf(stderr, "delete_dbinfo_rows: Failed to open Master Db\n");
		return (-1);
	}
	snprintf(SqlBuf, SQL_BUF_SIZE, "delete low_priority from dbinfo where filename=\"%s\"", filename);
	if (mysql_query(&mysql[0], SqlBuf)) {
		fprintf(stderr, "delete_dbinfo_rows: mysql_query: %s: %s\n", SqlBuf, mysql_error(&mysql[0]));
		return (-1);
	}
	return (0);
}

/*
 * write dbinfo records to mcdinfo
 */
int
writemcdinfo(DBINFO **rhostsptr, time_t mtime)
{
	char            mcdFile[MAX_BUFF];
	char           *sysconfdir, *mcdfile, *controldir;
	uid_t           uid;
	gid_t           gid;
	FILE           *fp;
	struct utimbuf  ubuf;
	DBINFO        **ptr;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		snprintf(mcdFile, MAX_BUFF, "%s", mcdfile);
	else 
	if (*controldir == '/')
		snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, mcdfile);
	}
	if (!rhostsptr)
		return (1);
	if (!(fp = fopen(mcdFile, "w"))) {
		fprintf(stderr, "writemcdinfo: %s: %s\n", mcdFile, strerror(errno));
		return (1);
	}
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	uid = indimailuid;
	gid = indimailgid;
	if (fchown(fileno(fp), uid, gid))
		fprintf(stderr, "writemcdinfo: fchown: %s: %s\n", mcdFile, strerror(errno));
	else
	if (fchmod(fileno(fp), INDIMAIL_QMAIL_MODE))
		fprintf(stderr, "writemcdinfo: fchmod: %s: %s\n", mcdFile, strerror(errno));
	for (ptr = rhostsptr;(*ptr);ptr++) {
		if ((*ptr)->isLocal)
			continue;
		fprintf(fp, "domain   %-28s %d\n", (*ptr)->domain, (*ptr)->distributed);
		fprintf(fp, "server   %s\n", (*ptr)->server);
		fprintf(fp, "mdahost  %s\n", (*ptr)->mdahost);
		fprintf(fp, "port     %d\n", (*ptr)->port);
		fprintf(fp, "use_ssl  %d\n", (*ptr)->use_ssl);
		fprintf(fp, "database %s\n", (*ptr)->database);
		fprintf(fp, "user     %s\n", (*ptr)->user);
		fprintf(fp, "pass     %s\n\n", (*ptr)->password);
	}
	fclose(fp);
	ubuf.actime = time(0);
	ubuf.modtime = mtime;
	if (utime(mcdFile, &ubuf))
		fprintf(stderr, "writemcdinfo: utime: %s: %s\n", mcdFile, strerror(errno));
	return (0);
}
#else
DBINFO **
LoadDbInfo_TXT(int *total)
{
	return (loadMCDInfo(total));
}
#endif

/*
 * Load records fromm the file mcdinfo
 */
static DBINFO **
loadMCDInfo(int *total)
{
	char            mcdFile[MAX_BUFF], dombuf[DBINFO_BUFF];
	char           *sysconfdir, *mcdfile, *controldir, *ptr; 
	int             ret, count, items, distributed;
	DBINFO        **relayhosts, **rhostsptr;
	char            buffer[MAX_BUFF], dummy1[MAX_BUFF], dummy2[MAX_BUFF];
	FILE           *fp;

	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	getEnvConfigStr(&mcdfile, "MCDFILE", MCDFILE);
	if (*mcdfile == '/' || *mcdfile == '.')
		scopy(mcdFile, mcdfile, MAX_BUFF);
	else 
	if (*controldir == '/')
		snprintf(mcdFile, MAX_BUFF, "%s/%s", controldir, mcdfile);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(mcdFile, MAX_BUFF, "%s/%s/%s", sysconfdir, controldir, mcdfile);
	}
	count = 0;
	relayhosts = (DBINFO **) 0;
	if (!(fp = fopen(mcdFile, "r")))
		return (localDbinfo(total, &relayhosts));
	else {
		/*- 
		 * get count of dbinfo records each
		 * dbinfo record has a 'server line
		 */
		for (;;) {
			if (!fgets(buffer, MAX_BUFF, fp)) {
				if (feof(fp))
					break;
				fprintf(stderr, "loadMCDInfo: fgets: %s\n", strerror(errno));
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
	if (!count) {
		fclose(fp);
		return (localDbinfo(total, &relayhosts));
	} else
	if (total)
		_total = (*total += count);
	if (!(relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (count + 1)))) {
		perror("malloc");
		fclose(fp);
		return ((DBINFO **) 0);
	}
	rewind(fp);
	for (*dombuf = 0, items = 0, count = 1, rhostsptr = relayhosts;; count++) {
		if (!fgets(buffer, MAX_BUFF, fp)) {
			if (feof(fp))
				break;
			fprintf(stderr, "loadMCDInfo: fgets: %s\n", strerror(errno));
		}
		if (buffer[strlen(buffer) - 1] != '\n') {
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
		if ((ret = sscanf(buffer, "%s %s", dummy1, dummy2)) != 2) {
			fprintf(stderr, "Line No %d in %s has no value line is %s", count, mcdFile, buffer);
			fclose(fp);
			free(relayhosts);
			return ((DBINFO **) 0);
		}
		if (!strncmp(dummy1, "domain", 6)) {
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
		if (!strncmp(dummy1, "server", 6)) {
			if (items) {
				fprintf(stderr, "Line Preceding %d in %s is incomplete\n", count, mcdFile);
				fclose(fp);
				free(relayhosts);
				errno = EINVAL;
				return ((DBINFO **) 0);
			}
			if (!((*rhostsptr) = (DBINFO *) malloc(sizeof(DBINFO)))) {
				perror("malloc");
				fclose(fp);
				free(relayhosts);
				return ((DBINFO **) 0);
			}
			items++;
			(*rhostsptr)->isLocal = 0;
			(*rhostsptr)->fd = -1;
			(*rhostsptr)->last_error = 0;
			(*rhostsptr)->failed_attempts = 0;
			scopy((*rhostsptr)->server, dummy2, DBINFO_BUFF);
		} else
		if ((*rhostsptr)) {
			if (!strncmp(dummy1, "mdahost", 7)) {
				items++;
				scopy((*rhostsptr)->mdahost, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "port", 4)) {
				items++;
				(*rhostsptr)->port = atoi(dummy2);
			} else
			if (!strncmp(dummy1, "use_ssl", 7)) {
				items++;
				(*rhostsptr)->use_ssl = (atoi(dummy2) ? 1 : 0);
			} else
			if (!strncmp(dummy1, "database", 8)) {
				items++;
				scopy((*rhostsptr)->database, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "user", 4)) {
				items++;
				scopy((*rhostsptr)->user, dummy2, DBINFO_BUFF);
			} else
			if (!strncmp(dummy1, "pass", 4)) {
				items++;
				scopy((*rhostsptr)->password, dummy2, DBINFO_BUFF);
			} else {
				fprintf(stderr, "Invalid Syntax at line %d  file %s - [%s]", count, mcdFile, buffer);
				fclose(fp);
				free(relayhosts);
				return ((DBINFO **) 0);
			}
			if (items == 7) {
				if (*dombuf) {
					scopy((*rhostsptr)->domain, dombuf, DBINFO_BUFF);
					(*rhostsptr)->distributed = distributed;
				} else {
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
	if (items) {
		fprintf(stderr, "Incomplete Structure in file %s\n", mcdFile);
		free(relayhosts);
		errno = EINVAL;
		return ((DBINFO **) 0);
	}
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
	if (!(rhostsptr = localDbinfo(total, &relayhosts)))
		fprintf(stderr, "localDbinfo: No local Dbinfo\n");
	else
		relayhosts = rhostsptr;
	_total = *total;
	return (relayhosts);
}

static DBINFO **
localDbinfo(int *total, DBINFO ***rhosts)
{
	FILE           *fp, *mfp;
	char           *mysqlhost, *mysql_user = 0, *mysql_passwd = 0; 
	char           *mysql_database = 0, *sysconfdir, *assigndir, *controldir, *ptr, *domain;
	char           *localhost, *mysql_socket = 0, *mysql_port = 0;
	char            host_path[MAX_BUFF], mysqlhost_buf[MAX_BUFF], TmpBuf[MAX_BUFF];
	int             count, field_count, found, use_ssl = 0;
	DBINFO        **relayhosts, **rhostsptr, **tmpPtr;

	relayhosts = *rhosts;
	getEnvConfigStr(&assigndir, "ASSIGNDIR", ASSIGNDIR);
	snprintf(TmpBuf, MAX_BUFF, "%s/assign", assigndir);
	if (!(fp = fopen(TmpBuf, "r"))) {
		fprintf(stderr, "fopen: %s: %s\n", TmpBuf, strerror(errno));
		return ((DBINFO **) 0);
	}
	/*- +indimail.org-:indimail.org:508:508:/var/indimail/domains/indimail.org:-:: -*/
	for (count = 0;;) {
		if (!fgets(TmpBuf, MAX_BUFF, fp)) {
			if (feof(fp))
				break;
			fprintf(stderr, "localDbinfo: fgets: %s\n", strerror(errno));
		}
		if (!(ptr = strchr(TmpBuf, ':')))
			continue;
		if (relayhosts) { /*- check for entries for domain in relayhosts */
			ptr++;
			domain = ptr;
			for (;*ptr && *ptr != ':';ptr++);
			if (*ptr)
				*ptr = 0;
			if (!isvirtualdomain(domain))
				continue;
			if (is_alias_domain(domain))
				continue;
			for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++) {
				if (!strncmp((*tmpPtr)->domain, domain, DBINFO_BUFF)) {
					/*- if relayhosts already has an entry then skip */
					found = 1;
					break;
				}
			}
			if (found)
				continue;
		}
		count++; /*- new domains - domains without entry in relayhosts */
	}
	*mysqlhost_buf = 0;
	if ((mysqlhost = (char *) getenv("MYSQL_HOST")) != (char *) 0)
		scopy(mysqlhost_buf, mysqlhost, MAX_BUFF);
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/') {
		if (snprintf(host_path, MAX_BUFF, "%s/host.mysql", controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	} else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		if (snprintf(host_path, MAX_BUFF, "%s/%s/host.mysql", sysconfdir, controldir) == -1)
			host_path[MAX_BUFF - 1] = 0;
	}
	if (!*mysqlhost_buf && !access(host_path, F_OK)) {
		if (!(mfp = fopen(host_path, "r")))
			scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
		else {
			if (!fgets(mysqlhost_buf, MAX_BUFF - 2, mfp))
				scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
			else
			if ((ptr = strrchr(mysqlhost_buf, '\n')))
				*ptr = 0;
			fclose(mfp);
		}
	} else
	if (!*mysqlhost_buf)
		scopy(mysqlhost_buf, MYSQL_HOST, MAX_BUFF);
	mysqlhost = mysqlhost_buf;
	for (field_count = 0,ptr = mysqlhost_buf;*ptr;ptr++) {
		if (*ptr == ':') {
			*ptr = 0;
			switch (field_count++)
			{
			case 0: /*- mysql user */
				if (*(ptr + 1))
					mysql_user = ptr + 1;
				break;
			case 1: /*- mysql passwd */
				if (*(ptr + 1))
					mysql_passwd = ptr + 1;
				break;
			case 2: /*- mysql socket/port */
				if (*(ptr + 1) == '/' || *(ptr + 1) == '.')
					mysql_socket = ptr + 1;
				else
				if (*(ptr + 1))
					mysql_port = ptr + 1;
				break;
			case 3: /*- ssl/nossl */
				use_ssl = (strncmp(ptr + 1, "ssl", 3) ? 0 : 1);
				break;
			}
		}
	}
	if (!mysql_user)
		getEnvConfigStr(&mysql_user, "MYSQL_USER", MYSQL_USER);
	if (!mysql_passwd)
		getEnvConfigStr(&mysql_passwd, "MYSQL_PASSWD", MYSQL_PASSWD);
	if (!mysql_socket)
		mysql_socket = (char *) getenv("MYSQL_SOCKET");
	if (!mysql_port && !(mysql_port = (char *) getenv("MYSQL_VPORT")))
		mysql_port = "0";
	getEnvConfigStr(&mysql_database, "MYSQL_DATABASE", MYSQL_DATABASE);
	if (!count) { /*- no extra domains found in assign file */
		fclose(fp);
		if (total) {
			/*- 
			 * remember that total is one less than the actual number of records allocated
			 * in loadMCDInfo(). So for one more record we have to allocate total + 1 + 1
			 * The new allocated becomes total + 1 plus 1 for the last NULL dbinfo structure
			 * The new total becoems total + 1
			 */
			relayhosts = (DBINFO **) realloc(relayhosts, sizeof(DBINFO *) * (*total + 2));
			rhostsptr = relayhosts + *total;
			for (tmpPtr = rhostsptr;tmpPtr < relayhosts + *total + 2;tmpPtr++)
				*tmpPtr = (DBINFO *) 0;
			(*total) += 1;
		} else {
			relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * 2);
			rhostsptr = relayhosts;
		}
		if (!((*rhostsptr) = (DBINFO *) malloc(sizeof(DBINFO)))) {
			perror("malloc");
			free(relayhosts);
			return ((DBINFO **) 0);
		}
		/*- Should check virtual domains and smtproutes */
		(*rhostsptr)->isLocal = 1; /*- this indicates that this record was created automatically */
		(*rhostsptr)->fd = -1;
		(*rhostsptr)->last_error = 0;
		(*rhostsptr)->failed_attempts = 0;
		if (!(localhost = get_local_ip(AF_INET))) /*- entry in control/localiphost */
			localhost = "localhost";
		scopy((*rhostsptr)->mdahost, localhost, DBINFO_BUFF);
		scopy((*rhostsptr)->server, mysqlhost, DBINFO_BUFF);
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		scopy((*rhostsptr)->domain, ptr, DBINFO_BUFF);
		(*rhostsptr)->port = atoi(mysql_port);
		(*rhostsptr)->socket = mysql_socket;
		(*rhostsptr)->use_ssl = use_ssl;
		scopy((*rhostsptr)->database, mysql_database, DBINFO_BUFF);
		scopy((*rhostsptr)->user, mysql_user, DBINFO_BUFF);
		scopy((*rhostsptr)->password, mysql_passwd, DBINFO_BUFF);
		(*rhostsptr)->distributed = 0;
		rhostsptr++;
		(*rhostsptr) = (DBINFO *) 0;
		return (relayhosts);
	}
	if (*total) { /*- we found domains in the mcdinfo file */
		relayhosts = (DBINFO **) realloc(relayhosts, sizeof(DBINFO *) * (*total + count + 1));
		rhostsptr = relayhosts + *total;
		for (tmpPtr = rhostsptr;tmpPtr < relayhosts + *total + count + 1;tmpPtr++)
			*tmpPtr = (DBINFO *) 0;
		_total = (*total) += count;
	} else {
		relayhosts = (DBINFO **) calloc(1, sizeof(DBINFO *) * (count + 1));
		rhostsptr = relayhosts;
	}
	if (!relayhosts) {
		perror("malloc");
		fclose(fp);
		return ((DBINFO **) 0);
	}
	rewind(fp);
	for (;;) {
		if (!fgets(TmpBuf, MAX_BUFF, fp)) {
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
		for (found = 0,tmpPtr = relayhosts;*tmpPtr;tmpPtr++) {
			if (!strncmp((*tmpPtr)->domain, domain, DBINFO_BUFF)) {
				/*- if relayhosts already has an entry then skip */
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		if (!((*rhostsptr) = (DBINFO *) malloc(sizeof(DBINFO)))) {
			perror("malloc");
			free(relayhosts);
			fclose(fp);
			return ((DBINFO **) 0);
		}
		/*- Should check virtual domains and smtproutes */
		if (total)
			(*total)++;
		(*rhostsptr)->isLocal = 1; /*- indicate that we were created automatically */
		(*rhostsptr)->fd = -1;
		(*rhostsptr)->last_error = 0;
		(*rhostsptr)->failed_attempts = 0;
		if (!(localhost = get_local_ip(AF_INET))) /*- entry in control/localiphost */
			localhost = "localhost";
		scopy((*rhostsptr)->mdahost, localhost, DBINFO_BUFF);
		scopy((*rhostsptr)->server, mysqlhost, DBINFO_BUFF);
		scopy((*rhostsptr)->domain, domain, DBINFO_BUFF);
		(*rhostsptr)->port = atoi(mysql_port);
		(*rhostsptr)->socket = mysql_socket;
		(*rhostsptr)->use_ssl = use_ssl;
		scopy((*rhostsptr)->database, mysql_database, DBINFO_BUFF);
		scopy((*rhostsptr)->user, mysql_user, DBINFO_BUFF);
		scopy((*rhostsptr)->password, mysql_passwd, DBINFO_BUFF);
		(*rhostsptr)->distributed = 0;
		rhostsptr++;
		(*rhostsptr) = (DBINFO *) 0;
	}
	fclose(fp);
	(*rhostsptr) = (DBINFO *) 0; /*- Null structure to end relayhosts */
	return (relayhosts);
}

void
getversion_LoadDbinfo_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
