#include	"loginexec.h"
#include	<sys/types.h>
#if	HAVE_SYS_STAT_H
#include	<sys/stat.h>
#endif
#include	<string.h>
#include	<stdio.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_SYS_WAIT_H
#include	<sys/wait.h>
#endif
#include	<stdlib.h>

#define LOGINEXEC_FILE "loginexec"

void maildir_loginexec(void)
{
#ifdef LOGINEXEC_FILE
struct stat buf;
pid_t pid;
int waitstat;

	if (stat(LOGINEXEC_FILE, &buf) != 0 || (buf.st_mode & S_IXUSR) == 0)
		return;

	while ((pid=fork()) == -1)
	{
		sleep(5);
	}
	if (pid == 0)
	{
		execl("./" LOGINEXEC_FILE, LOGINEXEC_FILE, (char *)0);
		perror("Failed to exec " LOGINEXEC_FILE);
		exit(1);
	}
	while (wait(&waitstat) != pid)
		;
	if (WIFEXITED(waitstat) && WEXITSTATUS(waitstat) == 0)
		unlink(LOGINEXEC_FILE);
#endif
}
