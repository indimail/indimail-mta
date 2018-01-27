/*
 * $Log: lockfile.c,v $
 * Revision 2.6  2012-08-16 07:53:16+05:30  Cprogrammer
 * removed spurios return statement
 *
 * Revision 2.5  2009-11-17 20:15:01+05:30  Cprogrammer
 * struct flock members have different order on Mac OS X
 *
 * Revision 2.4  2009-06-03 09:29:20+05:30  Cprogrammer
 * added lock code to use fcntl
 *
 * Revision 2.3  2008-07-13 19:44:52+05:30  Cprogrammer
 * use ERESTART only if available
 *
 * Revision 2.2  2002-12-11 10:28:15+05:30  Cprogrammer
 * added ERESTART check for errno
 *
 * Revision 2.1  2002-05-04 20:40:20+05:30  Cprogrammer
 * allow locking if original file does not exist
 *
 * Revision 1.13  2002-04-06 23:52:15+05:30  Cprogrammer
 * added flock() code
 *
 * Revision 1.12  2002-04-04 17:01:59+05:30  Cprogrammer
 * changed lockfile names to avoid being overwritten with pid of process
 * do not return EAGAIN if link fails
 *
 * Revision 1.11  2002-03-31 21:48:11+05:30  Cprogrammer
 * used creat() and link() for locking at filesystem level
 *
 * Revision 1.10  2002-03-28 01:59:25+05:30  Cprogrammer
 * added alarm to timeout semop() and return EAGAIN
 *
 * Revision 1.9  2002-03-27 01:50:47+05:30  Cprogrammer
 * function interface made identical for both semaphores and traditional file locking
 *
 * Revision 1.8  2002-03-27 00:56:54+05:30  Cprogrammer
 * return error if filename is not present
 *
 * Revision 1.7  2002-03-25 00:34:14+05:30  Cprogrammer
 * used semaphores for locking
 *
 * Revision 1.6  2002-03-13 09:52:31+05:30  Cprogrammer
 * fixed memory leak in ReleaseLock()
 *
 * Revision 1.5  2002-03-03 15:38:34+05:30  Cprogrammer
 * removed global variable Lockfile
 * added lock filename argument to ReleaseLock()
 *
 * Revision 1.4  2001-12-03 01:57:04+05:30  Cprogrammer
 * replaced sprintf with snprintf
 *
 * Revision 1.3  2001-11-24 12:19:22+05:30  Cprogrammer
 * version information added
 *
 * Revision 1.2  2001-11-20 10:55:18+05:30  Cprogrammer
 * *** empty log message ***
 *
 * Revision 1.1  2001-10-24 18:15:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"

#ifndef	lint
static char     sccsid[] = "$Id: lockfile.c,v 2.6 2012-08-16 07:53:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#ifdef FILE_LOCKING
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#ifdef USE_SEMAPHORES
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

static struct sembuf op_lock[2] = {
	{0, 0, 0}, /*- wait for sem#0 to become zero */
	{0, 1, SEM_UNDO} /*- then increment sem#0 by 1 */
};

static struct sembuf op_unlock[1] = {
	{0, -1, (IPC_NOWAIT | SEM_UNDO)} /*- decrement sem#0 by 1 (sets it to 0) */
};

static void     SigAlarm();

int
lockcreate(char *filename, char proj)
{
	key_t           key;
	int             semid;

	key = ftok(filename, proj);
	for(;;)
	{
		if ((semid = semget(key, 1, IPC_CREAT | 0666)) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		} else
			break;
	}
	return (semid);
}

int
lockremove(int semid)
{
	return (semctl(semid, 0, IPC_RMID, 0));
}

int             GotAlarm;
int
get_write_lock(int semid)
{
	void            (*pstat) ();

	GotAlarm = 0;
	if ((pstat = signal(SIGALRM, SigAlarm)) == SIG_ERR)
		return (-1);
	alarm(1);
	for(;;)
	{
		if (semop(semid, &op_lock[0], 2) == -1)
		{
#ifdef ERESTART
			if ((errno == EINTR || errno == ERESTART) && GotAlarm)
#else
			if (errno == EINTR && GotAlarm)
#endif
			{
				(void) signal(SIGALRM, pstat);
				GotAlarm = 0;
				errno = EAGAIN;
				return (-1);
			} else
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		} else
			break;
	}
	alarm(0);
	return (semid);
}

int
ReleaseLock(int fd)
{
	return (semop(fd, &op_unlock[0], 1));
}

/*- Dummy Function */
int
RemoveLock(char *filename, char proj)
{
	return (0);
}

static void
SigAlarm()
{
	GotAlarm = 1;
	return;
}
#elif defined(USE_LINKLOCK)
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

int
lockcreate(char *filename, char proj)
{
	char            TmpFil[MAX_BUFF], NewFil[MAX_BUFF];
	struct stat     statbuf;
	int             fd, tmperrno;
	pid_t           pid, mypid;
	time_t          file_age, start_time, secs;
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif

	start_time = time(0);
	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if ((fd = open(TmpFil, O_CREAT, 0666)) == -1)
		return (-1);
	close(fd);
	snprintf(NewFil, sizeof(NewFil), "%s.lck.%d", filename, proj);
	for(mypid = getpid();;)
	{
		if (!link(TmpFil, NewFil))
		{
			if ((fd = open(NewFil, O_WRONLY, 0)) == -1) /*- this should never happen */
			{
				tmperrno = errno;
				secs = time(0) - start_time;
				if (tmperrno == ENOENT)
					continue;
				else
				{
					errno = tmperrno;
					return (-1);
				}
			}
			secs = time(0) - start_time;
			if (secs)
				fprintf(stderr, "%d: lockfile %ld\n", getpid(), secs);
			return (fd);
		} else
		if (errno == EEXIST)
		{
			if (stat(NewFil, &statbuf))
			{
				usleep(500);
				continue;
			} else
			{
				file_age = time(0) - statbuf.st_atime;
				if (file_age > 5)
				{
					if ((fd = open(NewFil, O_RDWR, 0)) == -1)
						return (-1);
					if (read(fd, (char *) &pid, sizeof(pid_t)) == -1)
					{
						close(fd);
						return (-1);
					}
					if (pid == mypid)
					{
						close(fd);
						errno = EDEADLK;
						return (-1);
					} else
					if (!pid || kill(pid, 0))
					{
						fl.l_pid = getpid();
						for (;;)
						{
							if (fcntl(fd, F_SETLKW, &fl) == -1)
							{
#ifdef ERESTART
								if (errno == EINTR || errno == ERESTART)
#else
								if (errno == EINTR)
#endif
									continue;
								secs = time(0) - start_time;
								if (secs)
									fprintf(stderr, "%d: lockfile %ld\n", getpid(), secs);
								close(fd);
								errno = EAGAIN;
								return (-1);
							}
							break;
						}
						fl.l_type = F_UNLCK;
						if (unlink(NewFil))
						{
							if (fcntl(fd, F_SETLK, &fl) == -1);
							close(fd);
							return (-1);
						}
						if (fcntl(fd, F_SETLK, &fl) == -1)
						{
							close(fd);
							return (-1);
						}
						close(fd);
						continue;
					} else /*- some process still holds lock */
					{
						secs = time(0) - start_time;
						if (secs)
							fprintf(stderr, "%d: lockfile %ld\n", getpid(), secs);
						close(fd);
						errno = EAGAIN;
						/*- return (-1); -*/
					}
				} /*- if (file_age > 5) */
			} /* if (!stat(NewFil, &statbuf)) */
			usleep(500);
		} else
		if (errno == ENOENT)
		{
			if ((fd = open(TmpFil, O_CREAT, 0666)) == -1)
				return (-1);
			close(fd);
		} else	
			return (-1);
	}
}

int
get_write_lock(int fd)
{
	pid_t           pid;

	pid = getpid();
	if (write(fd, (char *) &pid, sizeof(pid_t)) == -1)
	{
		close(fd);
		return (-1);
	}
	return (fd);
}

int
ReleaseLock(int fd)
{
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	char            NewFil[MAX_BUFF];

	snprintf(NewFil, sizeof(NewFil), "%s.lck.%d", filename, proj);
	return (access(NewFil, F_OK) ? 0 : unlink(NewFil));
}
#elif defined(USE_FCNTLLOCK)
int
lockcreate(char *filename, char proj)
{
	int             fd;
	char            TmpFil[MAX_BUFF];
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_WRLCK, SEEK_SET};
#else
	struct flock    fl = {F_WRLCK, SEEK_SET, 0, 0, 0};
#endif

	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if ((fd = open(TmpFil, O_CREAT|O_WRONLY, 0644)) == -1)
		return (-1);
	fl.l_pid = getpid();
	for (;;)
	{
		if (fcntl(fd, F_SETLKW, &fl) == -1)
		{
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			return (-1);
		}
		break;
	}
	return (fd);
}

int
get_write_lock(int fd)
{
	return (fd);
}

int
ReleaseLock(int fd)
{
#ifdef DARWIN
	struct flock    fl = {0, 0, 0, F_UNLCK, SEEK_SET};
#else
	struct flock    fl = {F_UNLCK, SEEK_SET, 0, 0, 0};
#endif

	if (fd == -1)
		return (-1);
	fl.l_pid = getpid();
	if (fcntl(fd, F_SETLK, &fl) == -1);
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	char            TmpFil[MAX_BUFF];

	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if (!access(TmpFil, F_OK))
		return (unlink(TmpFil));
	else
		return (0);
}
#elif defined(USE_FLOCK)
int
lockcreate(char *filename, char proj)
{
	int             fd;
	char            TmpFil[MAX_BUFF];

	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if ((fd = open(TmpFil, O_CREAT|O_WRONLY, 0644)) == -1)
		return (-1);
	if (lockf(fd, F_LOCK, 0))
	{
		close(fd);
		return (-1);
	}
	return (fd);
}

int
get_write_lock(int fd)
{
	return (fd);
}

int
ReleaseLock(int fd)
{
	if (fd == -1)
		return (-1);
	if (!lockf(fd, F_ULOCK, 0))
		return (close(fd));
	return (close(fd));
}

int
RemoveLock(char *filename, char proj)
{
	char            TmpFil[MAX_BUFF];

	snprintf(TmpFil, sizeof(TmpFil), "%s.pre.%d", filename, proj);
	if (!access(TmpFil, F_OK))
		return (unlink(TmpFil));
	else
		return (0);
}
#endif /*- #ifdef USE_FLOCK */
#endif /*- #ifdef FILE_LOCKING */


void
getversion_lockfile_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
