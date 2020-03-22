/*
 * $Log: svcfns.c,v $
 * Revision 1.3  2005-08-23 17:39:17+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.2  2004-10-22 20:31:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 22:56:25+05:30  Cprogrammer
 * Initial revision
 *
 *
 * spipe -- supervise a pipeline of programs
 * Copyright (C) 2000 Bruce Guenter <bruceg@em.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "bool.h"
#include "svcfns.h"

void
exec_supervise(const char *dir, int fdin, int fdout)
{
	if (fdin != FD_STDIN)
	{
		close(FD_STDIN);
		dup2(fdin, FD_STDIN);
		close(fdin);
	}
	if (fdout != FD_STDOUT)
	{
		close(FD_STDOUT);
		dup2(fdout, FD_STDOUT);
		close(fdout);
	}
	execlp("supervise", "supervise", dir, (char *) 0);
	err("Could not exec supervise");
	exit(1);
}

pid_t
start_supervise(const char *dir, int fdin, int fdout)
{
	pid_t           pid = fork();
	switch (pid)
	{
	case -1:
		err("fork() failed while trying to run supervise");
		return 0;
	case 0:
		exec_supervise(dir, fdin, fdout);
	default:
		return pid;
	}
}

bool
stop_supervise(const char *dir, pid_t svcpid)
{
	int             status;
	pid_t           pid = fork();
	if (pid == -1)
	{
		err("fork failed while trying to run svc");
		return false;
	}
	if (pid == 0)
	{
		execlp("svc", "svc", "-dx", dir, (char *) 0);
		err("exec of svc failed");
		return false;
	}
	while (waitpid(pid, &status, WUNTRACED) != pid)
	{
		if (errno != EINTR)
		{
			err("Could not wait for svc to exit");
			return false;
		}
	}
	if (!WIFEXITED(status))
	{
		err("svc crashed");
		return false;
	}
	if (WEXITSTATUS(status))
	{
		err("svc failed");
		return false;
	}
	while (waitpid(svcpid, &status, WUNTRACED) != svcpid)
		if (errno != EINTR)
		{
			err("Could not wait for supervise to exit");
			return false;
		}
	if (!WIFEXITED(status))
	{
		err("supervise crashed");
		return false;
	}
	if (WEXITSTATUS(status))
	{
		err("supervise failed");
		return false;
	}
	return true;
}

void
getversion_svcfns_c()
{
	static char    *x = "$Id: svcfns.c,v 1.3 2005-08-23 17:39:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
