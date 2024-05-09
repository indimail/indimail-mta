/*
 * $Log: teepipe.c,v $
 * Revision 1.6  2022-09-03 22:24:33+05:30  Cprogrammer
 * refactored teepipe
 *
 * Revision 1.5  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.4  2020-09-16 19:08:05+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.3  2020-06-08 22:52:20+05:30  Cprogrammer
 * quench compiler warning
 *
 * Revision 1.2  2004-10-22 20:31:43+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 22:56:01+05:30  Cprogrammer
 * Initial revision
 *
 *
 * teepipe -- tee output to both stdout and a program
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
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <strerr.h>
#include <noreturn.h>

#define WARN   "teepipe: warning: "
#define FATAL  "teepipe: fatal: "


#define BUFSIZE 4096

int
main(int argc, char **argv)
{
	int             fd[2];
	int             status;
	ssize_t         rd;
	char            buf[BUFSIZE];
	pid_t           pid;

	if (argc < 2)
		strerr_die2x(100, WARN, "usage: teepipe program [args ...]: ");
	if (pipe(fd) == -1)
		strerr_die2sys(111, FATAL, "Could not create pipe: ");
	signal(SIGPIPE, SIG_IGN);
	switch ((pid = fork()))
	{
	case -1:
		strerr_die2sys(11, FATAL, "Could not fork: ");
		break;
	case 0: /*- child */
		if (close(fd[1]) || close(0) || dup2(fd[0], 0) || close(fd[0]))
			strerr_die2sys(111, FATAL, "Error setting up pipe as standard input: ");
		if (close(1) || dup2(2, 1) != 1)
			strerr_die2sys(111, FATAL, "Error setting up standard output: ");
		execvp(argv[1], argv + 1);
		strerr_die4sys(111, FATAL, "Error executing ", argv[1], ": ");
		break;
	}
	close(fd[0]);
	for (;;) {
		rd = read(0, buf, BUFSIZE);
		if (rd == 0 || rd == -1)
			break;
		if (write(fd[1], buf, rd) != rd)
			strerr_die2sys(111, FATAL, "Error writing to program: ");
		if (write(1, buf, rd) != rd)
			strerr_die2sys(111, FATAL, "Error writing to standard out: ");
	}
	if (close(fd[1])) /*- close pipe so that child reads 0, and exits */
		strerr_die2sys(111, FATAL, "Error closing output pipe: ");
	while (waitpid(pid, &status, WUNTRACED) != pid) {
		if (errno != EINTR)
			strerr_die2sys(111, FATAL, "failed to wait for child: ");
	}
	if (!WIFEXITED(status))
		strerr_die2x(111, FATAL, "child crashed: ");
	return WEXITSTATUS(status);
}

void
getversion_teepipe_c()
{
	const char     *x = "$Id: teepipe.c,v 1.6 2022-09-03 22:24:33+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
