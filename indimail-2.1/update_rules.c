/*
 * $Log: update_rules.c,v $
 * Revision 2.9  2016-06-13 14:24:23+05:30  Cprogrammer
 * set umask in the child process instead of parent
 *
 * Revision 2.8  2010-02-28 11:39:42+05:30  Cprogrammer
 * renamed OPEN_SMTP_CUR_FILE to OPEN_SMTP
 *
 * Revision 2.7  2009-02-18 21:37:35+05:30  Cprogrammer
 * check chown, write for errors
 *
 * Revision 2.6  2008-07-13 19:48:38+05:30  Cprogrammer
 * compilation on Mac OS X
 *
 * Revision 2.5  2008-06-13 10:16:37+05:30  Cprogrammer
 * fix compilation error if POP_AUTH_OPEN_RELAY was not defined
 *
 * Revision 2.4  2008-05-28 15:18:05+05:30  Cprogrammer
 * removed cdb module
 *
 * Revision 2.3  2005-12-29 22:50:45+05:30  Cprogrammer
 * use getEnvConfigStr to set variables from environment variables
 *
 * Revision 2.2  2005-01-22 00:42:06+05:30  Cprogrammer
 * replaced vfd_move() with dup2()
 *
 * Revision 2.1  2004-05-17 14:02:17+05:30  Cprogrammer
 * changed VPOPMAIL to INDIMAIL
 *
 * Revision 1.11  2002-04-04 16:40:50+05:30  Cprogrammer
 * replaced lockcreate and get_write_lock with getDbLock()
 *
 * Revision 1.10  2002-04-01 02:11:10+05:30  Cprogrammer
 * replaced ReleaseLock() and RemoveLock() with delDbLock()
 *
 * Revision 1.9  2002-03-31 21:51:06+05:30  Cprogrammer
 * RemoveLock() after releasing lock
 *
 * Revision 1.8  2002-03-27 01:53:14+05:30  Cprogrammer
 * removed redundant USE_SEMAPHORE definition
 *
 * Revision 1.7  2002-03-25 00:36:27+05:30  Cprogrammer
 * added semaphore update code
 *
 * Revision 1.6  2002-03-24 19:18:12+05:30  Cprogrammer
 * additional argument to get_write_lock()
 *
 * Revision 1.5  2002-03-03 15:41:35+05:30  Cprogrammer
 * Change in ReleaseLock() function
 *
 * Revision 1.4  2001-12-30 09:53:10+05:30  Cprogrammer
 * sizeof operator use in snprintf
 *
 * Revision 1.3  2001-11-24 12:20:15+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:56:12+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:13+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: update_rules.c,v 2.9 2016-06-13 14:24:23+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef POP_AUTH_OPEN_RELAY
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>


static unsigned long tcprules_open(void);

int
update_rules(lock)
	int             lock;
{
	FILE           *fs;
#ifdef FILE_LOCKING
	int             fd = -1;
#endif
	unsigned long   pid;
	int             wstat;
	char           *tcp_file, *open_smtp;
	char            TmpBuf1[MAX_BUFF];

	getEnvConfigStr(&open_smtp, "OPEN_SMTP", OPEN_SMTP);
#ifdef FILE_LOCKING
	if (lock && ((fd = getDbLock(open_smtp, 1)) == -1))
		return(-1);
#endif
	if ((pid = tcprules_open()) < 0)
	{
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(fd, open_smtp, 1);
#endif
		return(1);
	}
	if (vupdate_rules(fdm)) /*- write rules from RELAY table */
	{
		close(fdm);
#ifdef FILE_LOCKING
		if (lock)
			delDbLock(fd, open_smtp, 1);
#endif
		while (wait(&wstat) != pid);
		return(1);
	}
	getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
	if ((fs = fopen(tcp_file, "r")) != NULL)
	{
		while (fgets(TmpBuf1, 100, fs) != NULL)
		{
			if (write(fdm, TmpBuf1, slen(TmpBuf1)) == -1) /*- write rules in the file tcp.smtp */
			{
#ifdef FILE_LOCKING
				if (lock)
					delDbLock(fd, open_smtp, 1);
#endif
				break;
			}
		}
		fclose(fs);
	}
	close(fdm);
#ifdef FILE_LOCKING
	if (lock)
		delDbLock(fd, open_smtp, 1);
#endif
	/*
	 * wait until tcprules finishes so we don't have zombies 
	 */
	while (wait(&wstat) != pid);
	/*
	 * Set the ownership of the file 
	 */
	snprintf(TmpBuf1, MAX_BUFF, "%s.cdb", tcp_file);
	if (indimailuid == -1 || indimailgid == -1)
		GetIndiId(&indimailuid, &indimailgid);
	if (!getuid() || !geteuid())
		if (chown(TmpBuf1, indimailuid, indimailgid) == -1)
		{
			perror("chown: update_rules");
			return (-1);
		}
	return(0);
}

static unsigned long
tcprules_open()
{
	int             pim[2];
	unsigned long   pid;
	char            template[20];
	char           *tcp_file;
	char           *binqqargs[4];
	char            bin0[MAX_BUFF], bin1[MAX_BUFF], bin2[MAX_BUFF];

	memset(bin0, 0, MAX_BUFF);
	memset(bin1, 0, MAX_BUFF);
	memset(bin2, 0, MAX_BUFF);
	snprintf(template, sizeof(template), ".tmp.%d", (int) getpid());
	fdm = -1;
	if (pipe(pim) == -1)
		return (-1);
	switch (pid = vfork())
	{
	case -1:
		close(pim[0]);
		close(pim[1]);
		return (-1);
	case 0:
		close(pim[1]);
		if (dup2(pim[0], 0) == -1)
		{
			close(pim[0]);
			_exit(120);
		}
		umask(INDIMAIL_TCPRULES_UMASK);
		getEnvConfigStr(&tcp_file, "TCP_FILE", TCP_FILE);
		scopy(bin0, TCPRULES_PROG, MAX_BUFF);
		scopy(bin1, tcp_file, MAX_BUFF);
		scat(bin1, ".cdb", MAX_BUFF);
		scopy(bin2, tcp_file, MAX_BUFF);
		scat(bin2, template, MAX_BUFF);
		binqqargs[0] = bin0;
		binqqargs[1] = bin1;
		binqqargs[2] = bin2;
		binqqargs[3] = 0;
		execv(*binqqargs, binqqargs);
	}
	fdm = pim[1];
	close(pim[0]);
	return (pid);
}
#endif

void
getversion_update_rules_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
