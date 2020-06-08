/*
 * $Log: teepipe.c,v $
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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void
err(const char *msg)
{
	if (write(2, "teepipe: Error: ", 16) == -1 || write(2, msg, strlen(msg)) == -1 || write(2, "\n", 1) == -1) ;
	exit(1);
}

void
err2(const char *msg1, const char *msg2)
{
	if (write(2, "teepipe: Error: ", 16) == -1 || write(2, msg1, strlen(msg1)) == -1 ||
		write(2, msg2, strlen(msg2)) == -1 || write(2, "\n", 1) == -1) ;
	exit(1);
}

#define BUFSIZE 4096

void
main_loop(int fd)
{
	for (;;)
	{
		char            buf[BUFSIZE];
		ssize_t         rd = read(0, buf, BUFSIZE);
		if (rd == 0 || rd == -1)
			break;
		if (write(fd, buf, rd) != rd)
			err("Error writing to program");
		if (write(1, buf, rd) != rd)
			err("Error writing to standard output");
	}
}

void
exec_child(char **argv, int fd[2])
{
	if (close(fd[1]) || close(0) || dup2(fd[0], 0) || close(fd[0]))
		err("Error setting up pipe as standard input");
	if (close(1) || dup2(2, 1) != 1)
		err("Error setting up standard output");
	execvp(argv[0], argv);
	err2("Error executing", argv[0]);
}

int
main(int argc, char **argv)
{
	int             fd[2];
	int             status;
	pid_t           pid;
	if (argc < 2)
		err("usage: teepipe program [args ...]");
	if (pipe(fd) == -1)
		err("Could not create pipe");
	pid = fork();
	if (pid == -1)
		err("Could not fork");
	if (!pid)
		exec_child(argv + 1, fd);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	close(fd[0]);
	main_loop(fd[1]);
	if (close(fd[1]))
		err("Error closing output pipe");
	if (waitpid(pid, &status, 0) != pid)
		err("Error waiting for program to exit");
	return WIFEXITED(status) ? WEXITSTATUS(status) : 255;
}

void
getversion_teepipe_c()
{
	static char    *x = "$Id: teepipe.c,v 1.3 2020-06-08 22:52:20+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
