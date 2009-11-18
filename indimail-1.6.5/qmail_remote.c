/*
 * $Log: qmail_remote.c,v $
 * Revision 2.9  2009-11-09 10:42:50+05:30  Cprogrammer
 * changed BUFF_SIZE to MAX_BUFF
 *
 * Revision 2.8  2008-07-13 19:46:41+05:30  Cprogrammer
 * use ERESTART only if available
 * 64 bit compilation
 *
 * Revision 2.7  2005-12-29 18:51:49+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.6  2005-01-22 00:41:05+05:30  Cprogrammer
 * replaced vfd_move with dup2()
 *
 * Revision 2.5  2004-11-05 15:52:00+05:30  Cprogrammer
 * make qmail-remote read directly on fd 0 instead from pipe
 * this solves write: broken pipe error
 *
 * Revision 2.4  2004-10-27 14:48:47+05:30  Cprogrammer
 * BUG - qmail-remote called with incorrect no of arguments when SENDER or QQEH was not
 * defined
 *
 * Revision 2.3  2004-07-27 23:13:32+05:30  Cprogrammer
 * change due to qqeh
 * put errors on stderr
 *
 * Revision 2.2  2003-12-07 00:20:38+05:30  Cprogrammer
 * BUG. Size was not passed as the third argument
 *
 * Revision 2.1  2002-12-11 10:28:35+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 1.5  2001-12-19 20:38:38+05:30  Cprogrammer
 * close write end of pipe before reading to prevent deadlock
 *
 * Revision 1.4  2001-12-01 23:10:08+05:30  Cprogrammer
 * removed unecessary code
 *
 * Revision 1.3  2001-12-01 11:15:25+05:30  Cprogrammer
 * replaced msgbuf with ptr (bug correction)
 *
 * Revision 1.2  2001-12-01 02:15:12+05:30  Cprogrammer
 * replaced exit with the appropriate codes
 *
 * Revision 1.1  2001-12-01 01:44:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: qmail_remote.c,v 2.9 2009-11-09 10:42:50+05:30 Cprogrammer Stab mbhangui $";
#endif

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
/* 
 * Deliver an email to an address
 * Return 0 on success
 * Return less than zero on failure
 * 
 */
static int  decode(int);

int
qmail_remote(char *user, char *domain)
{
	int             pim1[2], wait_status, err, tmperrno;
	pid_t           pid;
	char           *ptr, *qmaildir, *binqqargs[7];
	char            recipient[AUTH_SIZE], mail_size[28];
	char            bin0[MAX_BUFF];

	if (pipe(pim1) == -1)
		return (-2);
	snprintf(recipient, AUTH_SIZE, "%s@%s", user, domain);
	snprintf(mail_size, sizeof(mail_size), "%"PRIu64, get_message_size());
	switch (pid = vfork())
	{
	case -1:
		tmperrno = errno;
		close(pim1[0]);
		close(pim1[1]);
		errno = tmperrno;
		return (-2);
	case 0:
		close(pim1[0]);
		if (dup2(pim1[1], 1) == -1)
			exit(111);
		if (dup2(1, 2) == -1)
			exit(111);
		getEnvConfigStr(&qmaildir, "QMAILDIR", QMAILDIR);
		snprintf(bin0, MAX_BUFF, "%s/bin/qmail-remote", qmaildir);
		binqqargs[0] = "qmail-remote";
		binqqargs[1] = domain;
		getEnvConfigStr(&ptr, "SENDER", "");
		binqqargs[2] = ptr;
		getEnvConfigStr(&ptr, "QQEH", "");
		binqqargs[3] = ptr;
		binqqargs[4] = mail_size;
		binqqargs[5] = recipient;
		binqqargs[6] = 0;
		execv(bin0, binqqargs);
		if(error_temp(errno))
			exit(111);
		exit(100);
	}
	close(pim1[1]);
	err = decode(pim1[0]);
	close(pim1[0]);
	for(;;)
	{
		pid = wait(&wait_status);
#ifdef ERESTART
		if(pid == -1 && (errno == EINTR || errno == ERESTART))
#else
		if(pid == -1 && errno == EINTR)
#endif
			continue;
		else
		if(pid == -1)
			return(-2);
		break;
	}
	if(WIFSTOPPED(wait_status) || WIFSIGNALED(wait_status))
	{
		fprintf(stderr, "qmail-remote crashed.\n");
		return(111);
	} else
	if(WIFEXITED(wait_status))
	{
		switch (WEXITSTATUS(wait_status))
		{
		case 0:
			break;
		case 111:
			return(111);
		default:
			return(100);
		}
	}
	return (err);
}

static int
decode(int read_fd)
{
	int             result, plen, len, bytes, j, k, orr;
	char            msgbuf[128];
	char           *ptr = 0;

	for (plen = 0, len = 1, ptr = 0;;)
	{
		bytes = read(read_fd, msgbuf, 128);
#ifdef ERESTART
		if (bytes == -1 && (errno == EINTR || errno == ERESTART))
#else
		if (bytes == -1 && errno == EINTR)
#endif
			continue;
		else
		if (bytes == -1)
		{
			fprintf(stderr, "decode: read: %s\n", strerror(errno));
			return (111);
		} else
		if (!bytes)
			break;
		len += bytes;
		if (!(ptr = (char *) realloc(ptr, len)))
		{
			fprintf(stderr, "decode: realloc: %s\n", strerror(errno));
			return (111);
		}
		memcpy(ptr + plen, msgbuf, bytes);
		plen += bytes;
	}
	if (!len)
	{
		fprintf(stderr, "qmail-remote produced no output.\n");
		return (111);
	}
	ptr[len] = 0;
	result = -1;
	j = 0;
	for (k = 0; k < len; ++k)
	{
		if (!ptr[k])
		{
			if (ptr[j] == 'K')
			{
				result = 1;
				break;
			}
			if (ptr[j] == 'Z')
			{
				result = 0;
				break;
			}
			if (ptr[j] == 'D')
				break;
			j = k + 1;
		}
	}
	orr = result;
	switch (ptr[0])
	{
	case 's':
		orr = 0;
		break;
	case 'h':
		orr = -1;
	}
	for (k = 0; k < len;)
	{
		if (!ptr[k++])
		{
			printf("%s", ptr + 1);
			if (result <= orr && k < len)
			{
				switch (ptr[k])
				{
				case 'Z':
				case 'D':
				case 'K':
					printf("%s", ptr + k + 1);
				}
			}
			break;
		}
	} /*- for (k = 1; k < len;) */
	fflush(stdout);
	switch (orr)
	{
	case 1:
		return (0);
		break;
	case 0:
		return (111);
		break;
	case -1:
		return (100);
		break;
	}
	return (111);
}

void
getversion_qmail_remote_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
