/*
 * $Log: CopyEmailFile.c,v $
 * Revision 2.10  2009-02-06 11:37:06+05:30  Cprogrammer
 * check return value of writes
 *
 * Revision 2.9  2008-07-13 19:09:26+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.8  2003-03-30 23:34:58+05:30  Cprogrammer
 * removed extra '\n' in Subject field
 *
 * Revision 2.7  2003-02-01 14:07:28+05:30  Cprogrammer
 * added option to put 'Date:' field and set the type of copy (link or copy)
 *
 * Revision 2.6  2003-01-16 23:59:46+05:30  Cprogrammer
 * added variable counter to prevent bulk mails getting overwritten
 * bug fix - count was getting reset to 0 leading to incorrect filesize
 *
 * Revision 2.5  2003-01-12 01:59:43+05:30  Cprogrammer
 * added missing unlink
 *
 * Revision 2.4  2002-12-11 10:27:31+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.3  2002-11-18 20:03:03+05:30  Cprogrammer
 * used MAX_LINK_COUNT instead of hardcoding
 *
 * Revision 2.2  2002-10-24 01:31:49+05:30  Cprogrammer
 * remove link in bulk_mail folder when link count exceeds 32000
 *
 * Revision 2.1  2002-07-11 18:17:13+05:30  Cprogrammer
 * added code to update quota when delivering mail.
 *
 * Revision 1.6  2002-03-18 19:03:39+05:30  Cprogrammer
 * avoid linking quota warn messages in Maildir/new with bulk_mail
 *
 * Revision 1.5  2002-03-15 21:11:48+05:30  Cprogrammer
 * change for linking bulkmail to a single file
 *
 * Revision 1.4  2002-03-03 18:54:22+05:30  Cprogrammer
 * Removed Checking for '@' in email as already fully qualified email is passed
 *
 * Revision 1.3  2001-11-24 12:16:15+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:53:11+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:14:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#ifndef	lint
static char     sccsid[] = "$Id: CopyEmailFile.c,v 2.10 2009-02-06 11:37:06+05:30 Cprogrammer Stab mbhangui $";
#endif

/*
 * copy_method 
 * 0 - Copy File
 * 1 - Link email to file in bulk_mail
 */
int
CopyEmailFile(homedir, fname, email, To, From, Subject, setDate, copy_method, message_size)
	const char     *homedir;
	char           *fname;
	const char     *email;
	char           *To, *From, *Subject;
	int             setDate, copy_method;
	long            message_size;
{
	time_t          tm;
	int             wfd, rfd, count, Fromlen, Tolen, Subjectlen, dlen, Datelen;
	static int      counter;
	pid_t           pid;
	char            hostname[MAX_BUFF], TmpFile[MAX_BUFF], buffer[MAX_BUFF], MailFile[MAX_BUFF];
	char            DeliveredTo[MAX_BUFF], Tolist[MAX_BUFF], Fromlist[MAX_BUFF], SubJect[MAX_BUFF], Date[MAX_BUFF];
	char           *ptr, *cptr;
	struct stat     statbuf;

	tm = time(0);
	pid = getpid();
	gethostname(hostname, sizeof(hostname));
	if (copy_method == 1 && (ptr = GetPrefix((char *) email, (char *) homedir)))
	{
		if (*ptr == '_')
			*ptr = '/';
		for (cptr = ptr + 1;*cptr && *cptr != '_';cptr++);
		*cptr = 0;
		snprintf(TmpFile, MAX_BUFF, "%s/bulk_mail", ptr);
		if (indimailuid == -1 || indimailgid == -1)
			GetIndiId(&indimailuid, &indimailgid);
		if (!r_mkdir(TmpFile, 0755, indimailuid, indimailgid))
		{
			if ((cptr = strrchr(fname, '/')))
				cptr++;
			else
				cptr = fname;
			snprintf(TmpFile, MAX_BUFF, "%s/bulk_mail/%s", ptr, cptr);
			snprintf(MailFile, MAX_BUFF, "%s/Maildir/new/%lu.%lu.%s,S=%lu", 
				homedir, (long unsigned) tm, (long unsigned) pid, hostname, message_size);
			if (stat(TmpFile, &statbuf))
			{
				if (errno == ENOENT && !fappend(fname, TmpFile, "w", 0644, indimailuid, indimailgid) && !link(TmpFile, MailFile))
				{
					snprintf(buffer, sizeof(buffer), "%s/Maildir", homedir);
					(void) update_quota(buffer, message_size);
					return(0);
				}
			} else
			if (statbuf.st_nlink > MAX_LINK_COUNT)
			{
				if (!unlink(TmpFile) && !fappend(fname, TmpFile, "w", 0644, indimailuid, indimailgid) && !link(TmpFile, MailFile))
				{
					snprintf(buffer, sizeof(buffer), "%s/Maildir", homedir);
					(void) update_quota(buffer, message_size);
					return(0);
				}
			} else
			if (!link(TmpFile, MailFile))
			{
				snprintf(buffer, sizeof(buffer), "%s/Maildir", homedir);
				(void) update_quota(buffer, message_size);
				return(0);
			}
		}
		/*- Some failure occurred. So try Copy Method */
	}
	Fromlen = Tolen = Subjectlen = dlen = Datelen = 0;
	snprintf(DeliveredTo, MAX_BUFF, "Delivered-To: %s\n", email);
	count = (dlen = slen(DeliveredTo));
	if (setDate)
	{
		snprintf(Date, MAX_BUFF, "Date: %s", ctime(&tm));
		count += (Datelen = slen(Date));
	}
	tm += counter++;
	if (To && *To)
	{
		snprintf(Tolist, MAX_BUFF, "To: %s\n", To);
		count += (Tolen = slen(Tolist));
	}
	if (From && *From)
	{
		snprintf(Fromlist, MAX_BUFF, "From: %s\n", From);
		count += (Fromlen = slen(Fromlist));
	}
	if (Subject && *Subject)
	{
		snprintf(SubJect, MAX_BUFF, "Subject: %s\n", Subject);
		count += (Subjectlen = slen(SubJect));
	}
	snprintf(TmpFile, MAX_BUFF, "%s/Maildir/tmp/%lu.%lu.%s,S=%lu", homedir, (long unsigned) tm, (long unsigned) pid, hostname,
			 message_size + count);
	snprintf(MailFile, MAX_BUFF, "%s/Maildir/new/%lu.%lu.%s,S=%lu", homedir, (long unsigned) tm, (long unsigned) pid, hostname,
			 message_size + count);
	if ((rfd = open(fname, O_RDONLY)) == -1)
	{
		fprintf(stderr, "%s: %s\n", fname, strerror(errno));
		return (-2);
	}
	if ((wfd = open(TmpFile, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)) == -1)
	{
		fprintf(stderr, "%s: %s\n", TmpFile, strerror(errno));
		close(rfd);
		return (-2);
	}
	if (write(wfd, DeliveredTo, dlen) != dlen)
	{
		close(wfd);
		close(rfd);
		unlink(TmpFile);
		fprintf(stderr, "write-header: %s\n", strerror(errno));
		return(-2);
	}
	if (setDate && write(wfd,  Date, Datelen) != Datelen)
	{
		fprintf(stderr, "write-header: %s\n", strerror(errno));
		return(-2);
	}
	if (Tolen && write(wfd,  Tolist, Tolen) != Tolen)
	{
		fprintf(stderr, "write-header: %s\n", strerror(errno));
		return(-2);
	}
	if (Fromlen && write(wfd,  Fromlist, Fromlen) != Fromlen)
	{
		fprintf(stderr, "write-header: %s\n", strerror(errno));
		return(-2);
	}
	if (Subjectlen && write(wfd,  SubJect, Subjectlen) != Subjectlen)
	{
		fprintf(stderr, "write-header: %s\n", strerror(errno));
		return(-2);
	}
	for (;;)
	{
		if (!(count = read(rfd, buffer, MAX_BUFF)))
			break;
		else
		if (count == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			perror("read");
			close(wfd);
			close(rfd);
			unlink(TmpFile);
			return (-2);
		}
		if (write(wfd, buffer, count) != count)
		{
			perror("write");
			close(wfd);
			close(rfd);
			unlink(TmpFile);
			return (-2);
		}
	} /*- for (;;) */
	close(wfd);
	close(rfd);
	snprintf(buffer, sizeof(buffer), "%s/Maildir", homedir);
	(void) update_quota(buffer, message_size + count);
	if (rename(TmpFile, MailFile))
	{
		fprintf(stderr, "rename %s %s: %s\n", TmpFile, MailFile, strerror(errno));
		unlink(TmpFile);
		return(-2);
	}
	return (0);
}

void
getversion_CopyEmailFile_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
