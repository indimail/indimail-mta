/*
 * $Log: deliver_mail.c,v $
 * Revision 2.68  2019-04-02 10:59:00+05:30  Cprogrammer
 * removed duplicate statement and fixed makeseekable
 *
 * Revision 2.67  2018-09-11 10:30:34+05:30  Cprogrammer
 * fixed compiler warnings
 *
 * Revision 2.66  2017-03-13 13:43:08+05:30  Cprogrammer
 * use PREFIX for binaries
 *
 * Revision 2.65  2016-05-25 09:00:40+05:30  Cprogrammer
 * use LIBEXECDIR for overquota.sh
 *
 * Revision 2.64  2016-05-17 17:09:39+05:30  Cprogrammer
 * use control directory set by configure
 *
 * Revision 2.63  2015-12-17 17:39:57+05:30  Cprogrammer
 * Fixed X-Forwarded-For, X-Forwarded-To headers for valias delivery
 * use fork() instead of vfork for makeseekable to work
 *
 * Revision 2.62  2015-12-15 16:18:46+05:30  Cprogrammer
 * make pipe seekable
 *
 * Revision 2.61  2014-04-18 17:30:45+05:30  Cprogrammer
 * dateFolder() function to deliver emails to a date formatted folder
 *
 * Revision 2.60  2011-12-01 20:28:59+05:30  Cprogrammer
 * fixed comparision when command executed in .qmail has multiple arguments
 *
 * Revision 2.59  2011-11-26 15:30:48+05:30  Cprogrammer
 * use a variable list for list of programs where DTLINE, RPLINE should not be unset
 *
 * Revision 2.58  2011-11-06 17:48:04+05:30  Cprogrammer
 * added command autoresponder for preserving DTLINE env variable
 *
 * Revision 2.57  2011-06-30 20:38:37+05:30  Cprogrammer
 * moved duplicate eliminator into a separate function in separate file ismaildup.c
 *
 * Revision 2.56  2011-06-24 16:03:00+05:30  Cprogrammer
 * do not unset DTLINE, RPLINE for preline, condtomaildir, qsmhook
 *
 * Revision 2.55  2011-06-22 22:30:52+05:30  Cprogrammer
 * unset RPLINE, DTLINE before calling external program
 *
 * Revision 2.54  2011-06-03 22:10:10+05:30  Cprogrammer
 * added comment for the return status of deliver_mail()
 *
 * Revision 2.53  2010-08-15 09:44:21+05:30  Cprogrammer
 * added X-Forwarded-To, X-Forwarded-For headers
 *
 * Revision 2.52  2010-07-14 22:14:49+05:30  Cprogrammer
 * initialize CurBytes, CurCount if NOQUOTA is set
 *
 * Revision 2.51  2010-04-03 20:32:05+05:30  Cprogrammer
 * fixed calling overquota_command when user is over quota
 * moved overquota.sh to libexec directory
 * QQEH to be unset only if QHPSI is set
 * fixed newline being removed from Delivered-To header
 *
 * Revision 2.50  2009-11-09 10:42:37+05:30  Cprogrammer
 * changed BUFF_SIZE to MAX_BUFF
 *
 * Revision 2.49  2009-10-14 20:42:24+05:30  Cprogrammer
 * check return status of parse_quota()
 *
 * Revision 2.48  2009-09-23 14:59:34+05:30  Cprogrammer
 * change for new runcmmd()
 *
 * Revision 2.47  2009-06-04 16:26:37+05:30  Cprogrammer
 * check return value of recalc_quota
 *
 * Revision 2.46  2009-03-31 10:22:41+05:30  Cprogrammer
 * do lseek only if MAKE_SEEKABLE is defined
 *
 * Revision 2.45  2009-02-18 21:24:21+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 2.44  2009-02-18 09:06:48+05:30  Cprogrammer
 * check chown error
 *
 * Revision 2.43  2008-11-14 08:52:04+05:30  Cprogrammer
 * fix for courier-imap putting blank line in maildirquota
 *
 * Revision 2.42  2008-11-13 19:27:36+05:30  Cprogrammer
 * removed newlines from error messages
 *
 * Revision 2.41  2008-11-11 15:58:08+05:30  Cprogrammer
 * fixes for quota checks
 *
 * Revision 2.40  2008-11-06 15:04:52+05:30  Cprogrammer
 * fix for NOQUOTA not working
 *
 * Revision 2.39  2008-11-01 15:41:11+05:30  Cprogrammer
 * return NOQUOTA if quota is 0
 *
 * Revision 2.38  2008-07-13 19:16:05+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.37  2008-06-25 15:39:32+05:30  Cprogrammer
 * removed sstrcmp
 *
 * Revision 2.36  2008-06-24 22:05:48+05:30  Cprogrammer
 * porting for 64 bit
 *
 * Revision 2.35  2008-06-17 12:00:58+05:30  Cprogrammer
 * use HAVE_OPENSSL_EVP_H to include openssl/evp.h
 *
 * Revision 2.34  2008-05-26 14:33:05+05:30  Cprogrammer
 * fixed compilation error if POSTFIXDIR was not defined
 *
 * Revision 2.33  2007-12-22 00:12:25+05:30  Cprogrammer
 * 1. Prevent race condition when creating temp file in Maildir/tmp
 * 2. BUG - Fixed wrong lenght passed to strncmp
 *
 * Revision 2.32  2005-12-29 22:43:38+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.31  2005-08-23 16:38:40+05:30  Cprogrammer
 * configurable mail duplicate eliminator
 *
 * Revision 2.30  2005-04-02 20:05:59+05:30  Cprogrammer
 * eliminate duplicates just before actual delivery
 *
 * Revision 2.29  2005-02-22 18:32:06+05:30  Cprogrammer
 * added code for qqeh
 *
 * Revision 2.28  2005-02-09 22:40:34+05:30  Cprogrammer
 * set CurBytes, CurCount to -1
 *
 * Revision 2.27  2005-01-22 00:40:42+05:30  Cprogrammer
 * added message duplicate eliminator
 *
 * Revision 2.26  2005-01-05 22:42:44+05:30  Cprogrammer
 * defer mail if holdoverquota is present. This will allow mails to
 * be delivered to a user when user's disk usage is brought back to
 * below the quota allocated
 *
 * Revision 2.25  2004-07-13 15:10:26+05:30  Cprogrammer
 * bug fix for calling overquota.sh
 *
 * Revision 2.24  2004-07-12 22:45:21+05:30  Cprogrammer
 * call a script when user gets overquota
 *
 * Revision 2.23  2004-07-02 18:04:42+05:30  Cprogrammer
 * renamed .deliveryCount to deliveryCount, .QuotaWarn to QuotaWarn
 *
 * Revision 2.22  2004-06-22 22:25:11+05:30  Cprogrammer
 * defer mail if sticky bit is set on Maildir
 * use stderr for all error messages
 *
 * Revision 2.21  2004-06-21 23:38:56+05:30  Cprogrammer
 * added MAILSIZE_LIMIT
 *
 * Revision 2.20  2004-06-20 17:38:36+05:30  Cprogrammer
 * run overquota.sh for MAILCOUNT_LIMIT exceeded
 *
 * Revision 2.19  2004-06-20 16:32:47+05:30  Cprogrammer
 * run overquota.sh when user runs out of quota
 * handle case when email is null
 *
 * Revision 2.18  2004-06-20 01:04:42+05:30  Cprogrammer
 * added recordMailcount for MAILCOUNT_LIMIT
 * renamed QUOTA_MAILSIZE to OVERQUOTA_MAILSIZE
 *
 * Revision 2.17  2004-06-19 23:41:00+05:30  Cprogrammer
 * new quota definition - AUTO for automatically figuring out quota while delivering
 *
 * Revision 2.16  2004-06-19 23:14:30+05:30  Cprogrammer
 * OVERQUOTA_MAILSIZE configurable through environment variable
 * use maildirsize for determining quota
 *
 * Revision 2.15  2004-05-17 14:00:52+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 2.14  2004-02-26 12:05:53+05:30  Cprogrammer
 * removed extra new line while writing Delivered-To header
 *
 * Revision 2.13  2004-01-15 23:21:20+05:30  Cprogrammer
 * added missing new line in Delivered-To header
 *
 * Revision 2.12  2003-10-21 21:36:49+05:30  Cprogrammer
 * delete local file if rename fails
 *
 * Revision 2.11  2003-10-01 03:09:56+05:30  Cprogrammer
 * shortened X-Filter header
 * added XDelivered-To header to keep track of forwardings
 *
 * Revision 2.10  2003-10-01 02:09:25+05:30  Cprogrammer
 * have Return-Path in the header when forwarding to an email id
 *
 * Revision 2.9  2003-08-24 16:29:36+05:30  Cprogrammer
 * changes for DTLINE compatibility with fetchmail
 *
 * Revision 2.8  2003-02-01 15:35:03+05:30  Cprogrammer
 * removed update_flag argument to user_over_quota()
 *
 * Revision 2.7  2002-12-11 10:28:04+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.6  2002-12-05 14:15:06+05:30  Cprogrammer
 * use sendmail if mta is postfix
 * set Return-Path when injecting mail
 *
 * Revision 2.5  2002-11-24 15:50:03+05:30  Cprogrammer
 * logic for using MAILDIRFOLDER moved to vdelivermail
 *
 * Revision 2.4  2002-11-21 00:56:55+05:30  Cprogrammer
 * return correct code in is_looping()
 * added function names in error messages
 *
 * Revision 2.3  2002-10-26 21:10:57+05:30  Cprogrammer
 * added more cases (see man qmail-command
 *
 * Revision 2.2  2002-10-15 11:42:11+05:30  Cprogrammer
 * added X-Filter header for mails forwarded
 *
 * Revision 2.1  2002-10-12 21:14:10+05:30  Cprogrammer
 * function to deliver mails to Maildir
 *
 */
#include "indimail.h"
#include "ismaildup.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifndef	lint
static char     sccsid[] = "$Id: deliver_mail.c,v 2.68 2019-04-02 10:59:00+05:30 Cprogrammer Exp mbhangui $";
#endif

/*- Function Prototypes */
static int      is_looping(char *);
static int      open_command(char *, int *);
static int      qmail_inject_open(char *, int *);
static void     getAlertConfig(char *, char *);

static void
getAlertConfig(char *mailalert_host, char *mailalert_port)
{
	char           *tmpstr, *cptr, *sysconfdir, *controldir;
	char            TmpBuf[MAX_BUFF];
	static char     alert_host[MAX_BUFF], alert_port[MAX_BUFF];
	FILE           *fp;

	if (*alert_host)
		scopy(mailalert_host, alert_host, MAX_BUFF);
	if (*alert_port)
		scopy(mailalert_port, alert_port, MAX_BUFF);
	if (*alert_host || *alert_port)
		return;
	*mailalert_host = *mailalert_port = 0;
	getEnvConfigStr(&controldir, "CONTROLDIR", CONTROLDIR);
	if (*controldir == '/')
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/mailalert.cfg", controldir);
	else {
		getEnvConfigStr(&sysconfdir, "SYSCONFDIR", SYSCONFDIR);
		snprintf(TmpBuf, sizeof(TmpBuf), "%s/%s/mailalert.cfg", sysconfdir, controldir);
	}
	if ((fp = fopen(TmpBuf, "r")))
	{
		for (;;)
		{
			if (!fgets(TmpBuf, sizeof(TmpBuf) - 2, fp))
				break;
			if ((tmpstr = strrchr(TmpBuf, '\n')))
				*tmpstr = 0;
			lowerit(TmpBuf);
			if ((tmpstr = strstr(TmpBuf, "host")))
			{
				tmpstr += 4;
				for (;*tmpstr && isspace((int) *tmpstr);tmpstr++);
				if (*tmpstr)
				{
					for (cptr = mailalert_host;*tmpstr;*cptr++ = *tmpstr++);
					*cptr = 0;
				}
				scopy(alert_host, mailalert_host, MAX_BUFF);
			} else
			if ((tmpstr = strstr(TmpBuf, "port")))
			{
				tmpstr += 4;
				for (;*tmpstr && isspace((int) *tmpstr);tmpstr++);
				if (*tmpstr)
				{
					for (cptr = mailalert_port;*tmpstr;*cptr++ = *tmpstr++);
					*cptr = 0;
				}
				scopy(alert_port, mailalert_port, MAX_BUFF);
			}
		} /*- for */
		fclose(fp);
	}
	return;
}

static int
qmail_inject_open(char *address, int *write_fd)
{
	int             pim[2];
	long unsigned   pid;
	char           *sender;
#ifdef POSTFIXDIR
	char           *mta;
#endif
	char            Address[AUTH_SIZE];
	char           *binqqargs[6], *bin0;

	/*- skip over an & sign if there */
	*write_fd = -1;
	if (*address == '&')
		address++;
	if (pipe(pim) == -1)
		return (-1);
	scopy(Address, address, AUTH_SIZE);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		if (getenv("QHPSI"))
			unsetenv("QQEH");
		close(pim[1]);
		if (dup2(pim[0], 0) == -1)
			_exit(111);
		getEnvConfigStr(&sender, "SENDER", "postmaster");
#ifdef POSTFIXDIR
		if (!(mta = getenv("MTA")))
			bin0 = PREFIX"/bin/qmail-inject";
		else
		{
			if (!strncmp(mta, "Postfix", 8))
				bin0 = PREFIX"/bin/sendmail";
			else
				bin0 = PREFIX"/bin/qmail-inject";
		}
#else
	bin0 = PREFIX"/bin/qmail-inject";
#endif
		binqqargs[0] = bin0;
		binqqargs[1] = "-f";
		binqqargs[2] = sender; 
		binqqargs[3] = Address;
		binqqargs[4] = 0;
		execv(*binqqargs, binqqargs);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	*write_fd = pim[1];
	close(pim[0]);
	return (pid);
}

#ifdef USE_MAILDIRQUOTA
char           *
read_quota(char *Maildir)
{
	char            maildir[MAX_BUFF];
	static char     tmpbuf[MAX_BUFF + 13];
	char           *ptr;
	int             count;
	FILE           *fp;

	scopy(maildir, Maildir, MAX_BUFF);
	if ((ptr = strstr(maildir, "/Maildir/")) && *(ptr + 9))
		*(ptr + 9) = 0;
	snprintf(tmpbuf, sizeof(tmpbuf), "%s/maildirsize", maildir);
	for(count = 0;;count++)
	{
		if (!(fp = fopen(tmpbuf, "r")))
			return ("NOQUOTA");
		if (!fgets(tmpbuf, MAX_BUFF - 2, fp))
		{
			fclose(fp);
#ifdef USE_MAILDIRQUOTA	
			if (recalc_quota(maildir, 0, 0, 0, 2) == -1)
				return ((char *) 0);
#else
			if (recalc_quota(maildir, 2) == -1)
				return ((char *) 0);
#endif
			if (!count)
				continue;
			fprintf(stderr, "invalid maildirquota specification");
			return ((char *) 0);
		}
		fclose(fp);
		break;
	}
	if (*tmpbuf == '\n') /*- fix for courier imap mucking things up */
	{
#ifdef USE_MAILDIRQUOTA	
		(void) recalc_quota(maildir, 0, 0, 0, 2);
#else
		(void) recalc_quota(maildir, 2);
#endif
		return ("NOQUOTA");
	} else
	if ((ptr = strchr(tmpbuf, '\n')))
		*ptr = 0;
	if (!strncmp(tmpbuf, "0S", 2))
		return ("NOQUOTA");
	return (tmpbuf);
}
#endif

int
recordMailcount(char *maildir, mdir_t curmsgsize, mdir_t *dailyMsgSize, mdir_t *dailyMsgCount)
{
	FILE           *fp;
	char            tmpbuf[MAX_BUFF], datebuf[80], tmpdate[MAX_BUFF], fileName[MAX_BUFF];
	char           *ptr;
	long            pos, savepos, count, size, mail_limit, size_limit;
#ifdef FILE_LOCKING
	int             fd;
#endif
	time_t          tmval;
	struct tm      *tmptr;

	if (access(maildir, F_OK))
		return (0);
	tmval = time(0);
	if (!(tmptr = localtime(&tmval)))
	{
		fprintf(stderr, "recordMailcount: localtime: %s\n", strerror(errno));
		return (-2);
	}
	snprintf(datebuf, sizeof(datebuf), "%02d-%02d-%04d:%02d:%02d:%02d", tmptr->tm_mday, tmptr->tm_mon + 1, 
		tmptr->tm_year + 1900, tmptr->tm_hour, tmptr->tm_min, tmptr->tm_sec);
	snprintf(fileName, sizeof(fileName), "%s/deliveryCount", maildir);
#ifdef FILE_LOCKING
	if ((fd = getDbLock(fileName, 1)) == -1)
	{
		fprintf(stderr, "recordMailcount: getDbLock: %s\n", strerror(errno));
		return (-2);
	}
#endif
	if (access(fileName, F_OK))
		fp = fopen(fileName, "w");
	else
		fp = fopen(fileName, "r+");
	if (!fp)
	{
		perror(fileName);
		return (-2);
	}
	*tmpdate = 0;
	for (savepos = 0;;)
	{
		pos = ftell(fp);
		if (!fgets(tmpbuf, sizeof(tmpbuf) - 2, fp))
			break;
		savepos = pos;
		if (sscanf(tmpbuf, "%s %ld %ld", tmpdate, &count, &size) != 3)
			continue;
	}
	/*-
	 * match indicates that mail has already been
	 * delivered on a date. Else this is
	 * the first time mail is being delivered
	 */
	if (!memcmp(tmpdate, datebuf, 10))
	{
		if (fseek(fp, savepos, SEEK_SET) == -1)
		{
			fclose(fp);
#ifdef FILE_LOCKING
			delDbLock(fd, fileName, 1);
#endif
			return (-2);
		}
	} else
		size = count = 0;
	if (dailyMsgCount)
		*dailyMsgCount = count;
	if (dailyMsgSize)
		*dailyMsgSize = size;
	mail_limit = ((ptr = getenv("MAILCOUNT_LIMIT")) && *ptr) ? atoi(ptr) : MAILCOUNT_LIMIT;
	size_limit = ((ptr = getenv("MAILSIZE_LIMIT")) && *ptr) ? atoi(ptr) : MAILSIZE_LIMIT;
	count++;
	size += curmsgsize;
	if ((mail_limit > 0 && count > mail_limit) || (size_limit > 0 && size > size_limit))
	{
		fclose(fp);
#ifdef FILE_LOCKING
		delDbLock(fd, fileName, 1);
#endif
		if (count > mail_limit && size > size_limit)
			fprintf(stderr, "Dir has insufficient quota. Mail count and size exceeded. %ld/%ld %ld/%ld. indimail (#5.1.4)",
				count, mail_limit, size, size_limit);
		else
		{
			if (count > mail_limit)
				fprintf(stderr, "Dir has insufficient quota. Mail count exceeded. %ld/%ld. indimail (#5.1.4)",
					count, mail_limit);
			if (size > size_limit)
				fprintf(stderr, "Dir has insufficient quota. Mail size exceeded. %ld/%ld. indimail (#5.1.4)",
					size, size_limit);
		}
		return (-1);
	}
	fprintf(fp, "%s %ld %ld\n", datebuf, count, size);
	fclose(fp);
#ifdef FILE_LOCKING
	delDbLock(fd, fileName, 1);
#endif
	return (0);
}

int
dateFolder(time_t tmval, char *buffer, size_t bufsize, char *format)
{
	struct tm      *tmptr;

	if (!(tmptr = localtime(&tmval)))
		return (1);
	if (!strftime(buffer, bufsize, format, tmptr))
		return (1);
	return (0);
}

/* 
 * Deliver an email to an address
 * Return 0 on success
 * Return less than zero on failure
 * 
 * -1 user is over quota
 * -2 system errors
 * -3 mail is looping 
 * -4 mail is over quota due to limits (MAILSIZE_LIMIT or MAILCOUNT_LIMIT)
 * -5 defer over quota mail instead of bouncing
 */
int
deliver_mail(char *address, mdir_t MsgSize, char *quota, uid_t uid, gid_t gid, 
	char *Domain, mdir_t *QuotaCount, mdir_t *QuotaSize)
{
	time_t          tm;
	int             wait_status, write_fd, is_injected, is_file, sfd, code, ret;
	off_t           file_count;
	char            local_file[MAX_BUFF + 1], local_file_new[MAX_BUFF], hostname[FILE_SIZE],
	                DeliveredTo[AUTH_SIZE], msgbuf[MSG_BUF_SIZE], user[AUTH_SIZE],
	                homedir[MAX_BUFF], mailalert_host[MAX_BUFF], mailalert_port[MAX_BUFF],
					date_dir[MAX_BUFF], tmpbuf[50];
	char           *maildirquota, *domain, *email, *rpline, *dtline, *xfilter, *ptr, *alert_host,
	               *alert_port, *cmmd, *ptr1, *ptr2, *qqeh, *faddr;
	char           *MailDirNames[] = {
		"cur",
		"new",
		"tmp",
	};
	mdir_t         *MailQuotaCount, *MailQuotaSize;
	mdir_t          _MailQuotaCount, _MailQuotaSize, quota_mailsize, dailyMsgCount, dailyMsgSize;
	FILE           *fp;
	static int      counter;
	struct stat     statbuf;
	struct passwd  *pw;
	long unsigned   pid;

	if (QuotaCount)
		MailQuotaCount = QuotaCount;
	else
		MailQuotaCount = &_MailQuotaCount;
	*MailQuotaCount = 0;
	if (QuotaSize)
		MailQuotaSize = QuotaSize;
	else
		MailQuotaSize = &_MailQuotaSize;
	is_injected = 0;
	/*- check if the email is looping to this user */
	if ((code = is_looping(address)) == 1)
	{
		fprintf(stderr, "Message is looping: [%s] ", address);
		return (-3);
	} else
	if (code == -1)
		return (-2);
	CurBytes = CurCount = 0;
	/*- This is a directory/Maildir location */
	if (*address == '/')
	{
		cmmd = "";
		is_file = 1;
		getEnvConfigStr(&rpline, "RPLINE", "Return-PATH: <>\n");
		dtline = getenv("DTLINE");
		xfilter = getenv("XFILTER");
		qqeh = getenv("QQEH");
		if (stat(address, &statbuf))
		{
			fprintf(stderr, "%s: %s ", address, strerror(errno));
			return (-2);
		}
		if (statbuf.st_mode & S_ISVTX)
		{
			errno = EAGAIN;
			fprintf(stderr, "Dir %s is sticky ", address);
			return (-2);
		}
		email = maildir_to_email(address, Domain);
		if (dtline)
		{
			if (qqeh && *qqeh)
			{
				snprintf(DeliveredTo, AUTH_SIZE, 
					"%s%s%s%s\nReceived: (indimail %d invoked by uid %d); %s\n",
					rpline, dtline, qqeh,
					xfilter ? xfilter : "X-Filter: None",
					getpid(), getuid(), get_localtime());
			} else
				snprintf(DeliveredTo, AUTH_SIZE, 
					"%s%s%s\nReceived: (indimail %d invoked by uid %d); %s\n",
					rpline, dtline,
					xfilter ? xfilter : "X-Filter: None",
					getpid(), getuid(), get_localtime());
		} else
		if (qqeh && *qqeh)
			snprintf(DeliveredTo, AUTH_SIZE,
				"%s%sDelivered-To: %s-%s\n%s\nReceived: (indimail %d invoked by uid %d); %s\n",
				rpline, qqeh, Domain, (email && *email) ? email : address,
				xfilter ? xfilter : "X-Filter: None",
				getpid(), getuid(), get_localtime());
		else
			snprintf(DeliveredTo, AUTH_SIZE,
				"%sDelivered-To: %s-%s\n%s\nReceived: (indimail %d invoked by uid %d); %s\n",
				rpline, Domain, (email && *email) ? email : address,
				xfilter ? xfilter : "X-Filter: None",
				getpid(), getuid(), get_localtime());
		MsgSize += slen(DeliveredTo);
		ptr1 = getenv("MAILSIZE_LIMIT");
		ptr2 = getenv("MAILCOUNT_LIMIT");
		if ((ptr1 && *ptr1) || (ptr2 && *ptr2))
		{
			if ((ret = recordMailcount(address, MsgSize, &dailyMsgSize, &dailyMsgCount)) == -1)
			{
				getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
				if (!access(ptr, X_OK))
				{
					/*
					 * Call overquota command with 5 arguments
					 * address currMessSize dailyMsgSize dailyMsgCount "dailySizeLimit"S,"dailyCountLimit"C
					 */
					snprintf(local_file, sizeof(local_file),
						"%s %s %"PRIu64" %"PRIu64" %"PRIu64" %s,%s",
						ptr, address, MsgSize, dailyMsgSize, dailyMsgCount, ptr1 ? ptr1 : "0S", ptr2 ? ptr2 : "0C");
					runcmmd(local_file, 0);
				}
				return (-4);
			} else
			if (ret) /*- system error */
				return (ret);
		}
		domain = 0;
		if (email && *email)
		{
			scopy(user, email, AUTH_SIZE);
			if ((domain =  strchr(user, '@')) != NULL)
			{
				*domain = 0;
				domain++;
			} else
			getEnvConfigStr(&domain, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
		}
		if (!strncmp(quota, "AUTO", 5))
		{
			maildirquota = (char *) 0;
#ifdef USE_MAILDIRQUOTA
			/*- if the user has a quota set */
			snprintf(homedir, sizeof(homedir), "%s/maildirsize", address);
			if (!access(homedir, F_OK))
			{
				if (!(maildirquota = read_quota(homedir)))
					return (-2);
			}  else
#endif
			/*
			 * Try to figure out the quota from the username
			 */
			if (!email || !*email || !(pw = vauth_getpw(user, domain)))
				maildirquota = "NOQUOTA";
			else
				maildirquota = pw->pw_shell;
			if ((ptr = strstr(homedir, "/maildirsize")))
				*ptr = 0;
		} else
		{
			maildirquota = quota;
			scopy(homedir, address, MAX_BUFF);
		}
		if (strncmp(maildirquota, "NOQUOTA,", 8))
		{
			/*
			 * If the user has insufficient quota to accept
			 * the current message and the msg size < OVERQUOTA_MAILSIZE bytes
			 * accept it. Else bounce it back.
			 * If the user has already exceeded quota, update
			 * userquota table and set the BOUNCE_MAIL flag
			 * if MAILDROP is defined use the format - MAILDIRQUOTA="5000000S,200C"
			 */
			if ((ret = user_over_quota(address, maildirquota, MsgSize)) == -1)
				return (-2);
			if (ret == 1)
			{
				getEnvConfigStr(&ptr, "OVERQUOTA_CMD", LIBEXECDIR"/overquota.sh");
				if (!access(ptr, X_OK))
				{
					/*
					 * Call overquota command with 6 arguments (including last dummy)
					 * email message_size curr_usage curr_count maildirquota "dummy"
					 */
					snprintf(local_file, sizeof(local_file),
						"%s %s %"PRIu64" %"PRIu64" %"PRIu64" %s dummy",
						ptr, address, MsgSize, CurBytes, CurCount, maildirquota);
					runcmmd(local_file, 0);
				}
				/*
				 * Defer Mail if holdoverquota file is present
				 */
				getEnvConfigStr(&ptr, "HOLDOVERQUOTA", "holdoverquota");
				if (ptr && *ptr == '/')
					scopy(local_file, ptr, sizeof(local_file));
				else
					snprintf(local_file, sizeof(local_file), "%s/%s", homedir, ptr);
				if (!access(local_file, F_OK))
					return (-5); /*- defer overquota mail */
				/*- Update quota in userquota and BOUNCE_MAIL in indimail */
				if (email && *email)
				{
#ifdef USE_MAILDIRQUOTA
					if ((*MailQuotaSize = parse_quota(maildirquota, MailQuotaCount)) == -1)
					{
						fprintf(stderr, "parse_quota: %s: %s\n", maildirquota, strerror(errno));
						return (-2);
					} else
					if (*MailQuotaSize)
					{
						if (CurBytes > *MailQuotaSize || (*MailQuotaCount && (CurCount > *MailQuotaCount)))
							vset_lastdeliver(user, domain, CurBytes);
					}
#else
					if ((*MailQuotaSize = atol(maildirquota)))
					{
						if (CurBytes > *MailQuotaSize)
							vset_lastdeliver(user, domain, CurBytes); 
					}
#endif
				}
				/*- 
				 * Bounce if Message size is greater 
				 * than quota_mailsize bytes
				 */
				quota_mailsize = ((ptr = getenv("OVERQUOTA_MAILSIZE")) && *ptr) ? atoi(ptr) : OVERQUOTA_MAILSIZE;
				if (MsgSize > quota_mailsize)
				{
					if (email && *email)
						MailQuotaWarn(user, domain, address, maildirquota);
					return (-1);
				}
			} 
			/*
			 * If we are going to deliver it, then add in the size 
			 */
			update_quota(address, MsgSize);
			if (email && *email)
			{
				MailQuotaWarn(user, domain, address, maildirquota);
				alert_host = alert_port = (char *) 0;
				getAlertConfig(mailalert_host, mailalert_port);
				if (*mailalert_host && *mailalert_port)
				{
					alert_host = mailalert_host;
					alert_port = mailalert_port;
				} else
				{
					alert_host = getenv("MAILALERT_HOST");
					alert_port = getenv("MAILALERT_PORT");
				}
				if (alert_host && alert_port && *alert_host && *alert_port)
				{
					if ((sfd = udpopen(alert_host, alert_port)) != -1)
					{
						snprintf(msgbuf, sizeof(msgbuf), "%s@%s %"PRIu64"\n", user, domain, MsgSize);
						if (write(sfd, msgbuf, slen(msgbuf)) == -1) ; /*- make compiler happy */
						close(sfd);
					}
				}
			}
		} else
			CurBytes = CurCount = 0;
#ifdef HAVE_SSL 
		if (getenv("ELIMINATE_DUPS") && ismaildup(address))
		{
			fprintf(stderr, "discarding duplicate msg ");
			return (0);
		}
#endif
		/*- Format the email file name */
		gethostname(hostname, sizeof(hostname));
		pid = getpid() + counter++;
		umask(INDIMAIL_UMASK);
		time(&tm);
		snprintf(local_file, sizeof(local_file), "%sfolder.dateformat", address);
		if (!access(local_file, R_OK)) {
			if (!(fp = fopen(local_file, "r"))) {
				fprintf(stderr, "open: %s: %s. indimail (#5.1.2)", local_file, strerror(errno));
				return (-2);
			}
			if (!fgets(date_dir, sizeof(date_dir) - 2, fp)) {
				fprintf(stderr, "%s: EOF. indimail (#5.1.2)", local_file);
				fclose(fp);
				return (-2);
			}
			fclose(fp);
			if ((ptr = strrchr(date_dir, '\n')))
				*ptr = 0;
			if (!(ret = dateFolder(tm, tmpbuf, sizeof(tmpbuf) - 1, date_dir))) {
				snprintf(local_file, sizeof(local_file), "%s%s", address, tmpbuf);
				if (access(local_file, F_OK)) {
					if (r_mkdir(local_file, INDIMAIL_DIR_MODE, uid, gid)) {
						fprintf(stderr, "r_mkdir: %s: %s. indimail (#5.1.2)", local_file, strerror(errno));
						return (-2);
					} else {
						for (code = 0;code < 3;code++) {
							snprintf(local_file, sizeof(local_file), "%s%s/%s", address, tmpbuf, MailDirNames[code]);
							if (r_mkdir(local_file, INDIMAIL_DIR_MODE, uid, gid)) {
								fprintf(stderr, "r_mkdir: %s: %s. indimail (#5.1.2)", local_file, strerror(errno));
								return (-2);
							}
						}
					}
				} else {
					for (code = 0;code < 3;code++) {
						snprintf(local_file, sizeof(local_file), "%s%s/%s", address, tmpbuf, MailDirNames[code]);
						if (access(local_file, F_OK)) {
							if (r_mkdir(local_file, INDIMAIL_DIR_MODE, uid, gid)) {
								fprintf(stderr, "r_mkdir: %s: %s. indimail (#5.1.2)", local_file, strerror(errno));
								return (-2);
							}
						}
					}
				}
			} else {
				fprintf(stderr, "dateFolder: failed to get date format: %s. indimail (#5.1.2)", strerror(errno));
				return (-2);
			}
			snprintf(local_file, sizeof(local_file), "%s%s/tmp/%ld.%ld.%s,S=%"PRIu64, address, tmpbuf, tm, pid, hostname,
				MsgSize);
			snprintf(local_file_new, sizeof(local_file_new), "%s%s/new/%ld.%ld.%s,S=%"PRIu64, address, tmpbuf, tm, pid, hostname,
				MsgSize);
		} else {
			snprintf(local_file, sizeof(local_file), "%stmp/%ld.%ld.%s,S=%"PRIu64, address, tm, pid, hostname,
				MsgSize);
			snprintf(local_file_new, sizeof(local_file_new), "%snew/%ld.%ld.%s,S=%"PRIu64, address, tm, pid, hostname,
				MsgSize);
		}
		if ((write_fd = open(local_file, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
		{
			fprintf(stderr, "%s: %s ", local_file, strerror(errno));
			return (-2);
		}
		if ((!getuid() || !geteuid()) && fchown(write_fd, uid, gid))
		{
			fprintf(stderr, "fchown: %s: %s ", local_file, strerror(errno));
			return (-2);
		}
		/*- write the Return-Path: and Delivered-To: headers */
		if (write(write_fd, DeliveredTo, slen(DeliveredTo)) != slen(DeliveredTo))
		{
			/*- Check if the user is over quota */
			if (errno == EDQUOT)
			{
				close(write_fd);
				return (-1);
			} else
			{
				fprintf(stderr, "write: delivered to line: %s ", strerror(errno));
				close(write_fd);
				return (-2);
			}
		}
	} else
	if (*address == '|')
	{
		is_file = 0;
		/*- This is an command */
		is_injected = 1;
		/*- open up a pipe to a command */
		if ((pid = open_command(address, &write_fd)) < 0)
		{
			fprintf(stderr, "open_cmmd: %s: %s ", address, strerror(errno));
			return (-2);
		}
		for (cmmd = address + 1;*cmmd && isspace((int) *cmmd);cmmd++);
	} else /*- must be an email address */
	{
		is_file = 0;
		is_injected = 1;
		cmmd = "qmail-inject";
		/*- exec qmail-inject and connect the file descriptors */
		if ((pid = qmail_inject_open(address, &write_fd)) < 0)
		{
			fprintf(stderr, "qmail-inject: %s: %s ", address, strerror(errno));
			return (-2);
		}
	}
	if (is_injected)
	{
		char           *tstr = 0;
		if ((dtline = getenv("DTLINE"))) /*- original address */
		{
			for (;*dtline && *dtline != ':';dtline++);
			if (*dtline)
				dtline++;
			for (;*dtline && isspace((int) *dtline);dtline++);
			for (tstr = dtline;*tstr && *tstr != '\n';tstr++);
			if (*tstr == '\n')
				*tstr = 0;
			if (!*dtline)
				dtline = (*address == '&') ? address + 1: address;
		} else
		if (!(dtline = getenv("RECIPIENT")))
			dtline = "pipe";
		faddr = *address == '&' ? address + 1 : address;
		xfilter = getenv("XFILTER");
		qqeh = getenv("QQEH");
		if (qqeh && *qqeh)
			snprintf(DeliveredTo, AUTH_SIZE, 
				"X-Forwarded-To: %s\nX-Forwarded-For: %s\n%s%s\n"
				"Received: (indimail %d invoked by uid %d); %s\n",
				faddr, dtline, qqeh, xfilter ? xfilter : "X-Filter: None",
				getpid(), getuid(), get_localtime());
		else
			snprintf(DeliveredTo, AUTH_SIZE, 
				"X-Forwarded-To: %s\nX-Forwarded-For: %s\n%s\n"
				"Received: (indimail %d invoked by uid %d); %s\n",
				faddr, dtline, xfilter ? xfilter : "X-Filter: None",
				getpid(), getuid(), get_localtime());
		if (tstr) /*- replace the newline in DTLINE environment variable */
			*tstr = '\n';
		if (write(write_fd, DeliveredTo, slen(DeliveredTo)) != slen(DeliveredTo))
		{
			/*- Check if the user is over quota */
			if (errno == EDQUOT)
			{
				close(write_fd);
				return (-1);
			} else
			{
				fprintf(stderr, "write: delivered to line: %s ", strerror(errno));
				close(write_fd);
				return (-2);
			}
		}
	}
	/*- start the the begining of the email file */
	if ((ptr = getenv("MAKE_SEEKABLE")) && *ptr != '0' && lseek(0, 0L, SEEK_SET) < 0)
	{
		fprintf(stderr, "deliver_mail: lseek: %s ", strerror(errno));
		close(write_fd);
		return (-2);
	}
	/*- read it in chunks and write it to the new file */
	while ((file_count = read(0, msgbuf, MSG_BUF_SIZE)) > 0)
	{
		if (write(write_fd, msgbuf, file_count) == -1)
		{
			/*- Check if the user is over quota */
			if (errno == EDQUOT)
			{
				close(write_fd);
				return (-1);
			} else
			{
				fprintf(stderr, "write: is_file %d: %s ", is_file, strerror(errno));
				close(write_fd);
				return (-2);
			}
			/*
			 * if the write fails and we are writing to a Maildir
			 * then unlink the file
			 */
			if (is_file == 1 && unlink(local_file))
			{
				fprintf(stderr, "unlink-%s: %s ", local_file, strerror(errno));
				return (-2);
			}
		}
	}
	/*
	 * if we are writing to a Maildir, move it
	 * into the new directory
	 */
	if (is_file == 1)
	{
		/*- sync the data to disk and close the file */
		errno = 0;
		if (
#ifdef FILE_SYNC
#ifdef HAVE_FDATASYNC
			   fdatasync(write_fd) == 0 &&
#else
			   fsync(write_fd) == 0 &&
#endif
#endif
			   close(write_fd) == 0)
		{
#ifdef USE_LINK
			/*- if this succeeds link the file to the new directory */
			if (link(local_file, local_file_new))
			{
				/*- coda fs has problems with link, check for that error */
				if (errno == EXDEV)
				{
					/*- try to rename the file instead */
					if (rename(local_file, local_file_new) != 0)
					{
						/*- even rename failed, time to give up */
						fprintf(stderr, "rename %s %s: %s ", local_file, local_file_new, strerror(errno));
						unlink(local_file);

						return (-2);
						/*- rename worked, so we are okay now */
					} else
						errno = 0;
				} else /*- link failed and we are not on coda */
					fprintf(stderr, "link %s %s: %s ", local_file, local_file_new, strerror(errno));
				if (unlink(local_file))
					fprintf(stderr, "unlink-%s: %s ", local_file, strerror(errno));
			} else
			if (unlink(local_file))
				fprintf(stderr, "unlink-%s: %s\n", local_file, strerror(errno));
#else
			if (rename(local_file, local_file_new) != 0)
			{
				fprintf(stderr, "rename %s %s: %s ", local_file, local_file_new, strerror(errno));
				return (-2);
			} else
				errno = 0;
#endif /*- #ifdef USE_LINK */
		} /* Data Sync */
		/*- if any errors occured then return the error */
		if (errno != 0)
			return (-2);
	} else
	if (is_injected == 1)
	{
		/*
		 * If we were writing it to qmail-inject
		 * then wait for qmail-inject to finish 
		 */
		close(write_fd);
		for (;;)
		{
			pid = wait(&wait_status);
#ifdef ERESTART
			if (pid == -1 && (errno == EINTR || errno == ERESTART))
#else
			if (pid == -1 && errno == EINTR)
#endif
				continue;
			else
			if (pid == -1)
			{
				fprintf(stderr, "qmail-inject crashed. indimail bug ");
				return (111);
			}
			break;
		}
		if (WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status))
		{
			fprintf(stderr, "qmail-inject crashed. ");
			return (111);
		} else
		if (WIFEXITED(wait_status))
		{
			switch (code = WEXITSTATUS(wait_status))
			{
			case 0:
				return (0);
			case 99:
				return (99);
			case 64:
			case 65:
			case 70:
			case 76: 
			case 77: 
			case 78: 
			case 112:
			case 100:
				fprintf(stderr, "%s failed code(%d). Permanent error ", *address == '|' ? cmmd : address, code);
				_exit(100);
			default:
				fprintf(stderr, "%s failed code(%d). Temporary error ", *address == '|' ? cmmd : address, code);
				_exit(111);
			}
		}
	}
	return (0);
}

/* 
 * Get the size of the email message 
 * return the size 
 */
mdir_t
get_message_size()
{
	struct stat     statbuf;

	if (fstat(0, &statbuf))
	{
		fprintf(stderr, "fstat: %s\n", strerror(errno));
		return (-1);
	}
	return (statbuf.st_size);
}

/* 
 * open a pipe to a command 
 * return the pid or -1 if error
 */

static int
open_command(char *command, int *write_fd)
{
	int             pim[2];
	long unsigned   pid;
	char            cmmd[MAX_BUFF], ncmmd[MAX_BUFF];
	char           *p, *r, *binqqargs[4];
	char          **q;
	char *allowed_programs[] = { 
		"autoresponder",
		"condredirect",
		"condtomaildir",
		"dot-forward",
		"fastforward",
		"filterto",
		"forward",
		"maildirdeliver",
		"preline",
		"qnotify",
		"qsmhook",
		"replier",
		"rrforward",
		"serialcmd",
		0
	};

	/*- skip over an | sign if there */
	*write_fd = -1;
	if (*command == '|')
		++command;
	if (pipe(pim) == -1)
		return (-1);
	scopy(cmmd, command, AUTH_SIZE);
	cmmd[AUTH_SIZE - 1] = 0;
	switch (pid = fork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		if (getenv("QHPSI"))
			unsetenv("QQEH");
		for (r = cmmd;*r && isspace(*r);r++);
		/*- copy only the first command argument (i.e. argv[0]) */
		for (p = ncmmd;*r && !isspace(*r);*p++ = *r++);
		*p = 0;
		if ((p = strrchr(ncmmd, '/')))
			p++;
		else
			p = ncmmd;
		for (q = allowed_programs;*q;q++) {
			if (!strcmp(p, *q))
				break;
		}
		if (!*q)
		{
			if (getenv("RPLINE"))
				unsetenv("RPLINE");
			if (getenv("DTLINE"))
				unsetenv("DTLINE");
		}
		close(pim[1]);
		if (dup2(pim[0], 0) == -1)
			_exit(-1);
#ifdef MAKE_SEEKABLE
		if ((p = getenv("MAKE_SEEKABLE")) && *p != '0' && makeseekable(stdin))
		{
			fprintf(stderr, "makeseekable: system error: %s\n", strerror(errno));
			_exit(111);
		}
#endif
		binqqargs[0] = "/bin/sh";
		binqqargs[1] = "-c";
		binqqargs[2] = cmmd;
		binqqargs[3] = 0;
		execv(*binqqargs, binqqargs);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	*write_fd = pim[1];
	close(pim[0]);
	return (pid);
}

/*
 * Check for a looping message
 * This is done by checking for a matching line
 * in the email headers for Delivered-To: which
 * we put in each email
 * 
 * Return  1 if looping
 * Return  0 if not looping
 */
int
is_looping(char *address)
{
	int             i;
	int             found;
	char            loop_buf[FILE_SIZE];
	char           *dtline, *ptr;

	if (*address == '&')
		address++;
	/*- check the DTLINE */
	if ((dtline = (char *) getenv("DTLINE")) != (char *) 0)
		ptr = strstr(dtline, address);
	else
		ptr = 0;
	if (dtline && ptr && !strcmp(ptr, address))
		return (1);
	if (!(ptr = getenv("MAKE_SEEKABLE")) || *ptr == '0')
		return (0);
	if (lseek(0, 0L, SEEK_SET) < 0)
	{
		fprintf(stderr, "is_looping: lseek: %s\n", strerror(errno));
		return (-1);
	}
	while (fgets(loop_buf, sizeof(loop_buf), stdin) != NULL)
	{
		/*
		 * if we find the line, return error (looping) 
		 */
		if (strstr(loop_buf, "Delivered-To") && strstr(loop_buf, address))
			return (1);
		else
		{
			/*
			 * check for the start of the body, we only need
			 * to check the headers. 
			 *
			 * walk through the charaters in the body 
			 */
			for (i = 0, found = 0; loop_buf[i] && !found; ++i)
			{
				switch (loop_buf[i])
				{
					/*- skip blank spaces and new lines */
				case ' ':
				case '\n':
				case '\t':
				case '\r':
					break;
				default:
					/*
					 * found a non blank, so we are still
					 * in the headers
					 * set the found non blank char flag 
					 */
					found = 1;
					break;
				}
			}
			/*
			 * if the line only had blanks, then it is the
			 * delimiting line between the headers and the
			 * body. We don't need to check the body for
			 * the duplicate Delivered-To: line. Hence, we
			 * are done with our search and can return the
			 * looping not found value return not found looping 
			 * message value 
			 */
			if (found == 0)
				return (0);
		}
	}
	/*
	 * if we get here then the there is either no body 
	 * or the logic above failed and we scanned
	 * the whole email, headers and body. 
	 */
	return (0);
}

void
getversion_deliver_mail_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	printf("%s\n", sccsidisduph);
}
