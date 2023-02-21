/*
 * $Log: multipipe.c,v $
 * Revision 1.4  2020-10-08 12:07:42+05:30  Cprogrammer
 * formatted code
 *
 * Revision 1.3  2011-05-07 15:57:11+05:30  Cprogrammer
 * added error checks
 *
 * Revision 1.2  2004-10-22 20:27:36+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 22:56:10+05:30  Cprogrammer
 * Initial revision
 *
 *
 * multipipe -- pipe output to multiple programs
 * Copyright (C) 2000 Bruce Guenter <bruceg@em.ca>
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
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "select.h"
#include "direntry.h"
#include "bool.h"
#include "svcfns.h"

void
err(const char *msg)
{
	if (write(2, "multipipe: Error: ", 18) == -1)
		_exit(111);
	if (write(2, msg, strlen(msg)) == -1)
		_exit(111);
	if (write(2, "\n", 1) == -1)
		_exit(111);
}

void
err2(const char *msg1, const char *msg2)
{
	if (write(2, "multipipe: Error: ", 18) == -1)
		_exit(111);
	if (write(2, msg1, strlen(msg1)) == -1)
		_exit(111);
	if (write(2, msg2, strlen(msg2)) == -1)
		_exit(111);
	if (write(2, "\n", 1) == -1)
		_exit(111);
}

void
set_ndelay(int fd)
{
	int             flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

#define BUFSIZE 8192
static char     buffer[BUFSIZE];
static unsigned buf_start = 0;
static unsigned buf_end = 0;
static bool     buf_eof = false;
#define buf_wrapped (buf_end < buf_start)
#define buf_left (buf_start-buf_end + (buf_wrapped ? -1 : BUFSIZE-1))
#define buf_len (buf_end-buf_start + (buf_wrapped ? BUFSIZE : 0))

struct reader
{
	ino_t           inode;
	const char     *name;
	pid_t           pid;
	int             fd;
	unsigned        buf_pos;
	bool            marked;
	struct reader  *next;
};

static struct reader *readers = 0;

void
reset_buf_start(void)
{
	struct reader  *reader;
	bool            wrapped = (buf_end < buf_start);

	buf_start = buf_end;
	for (reader = readers; reader; reader = reader->next) {
		unsigned        bp = reader->buf_pos;
		if (bp < buf_start && (!wrapped || bp >= buf_end))
			buf_start = bp;
	}
	if (buf_start == buf_end) {
		buf_end = buf_start = 0;
		for (reader = readers; reader; reader = reader->next)
			reader->buf_pos = 0;
	}
}

void
read_input(void)
{
	unsigned        readable = buf_wrapped ? buf_left : BUFSIZE - buf_end;
	ssize_t         rd;
	if (readable >= buf_left)
		readable = buf_left;
	if ((rd = read(FD_STDIN, buffer + buf_end, readable)) <= 0)
		buf_eof = true;
	else
		buf_end = (buf_end + rd) % BUFSIZE;
}

void
write_output(struct reader *reader)
{
	unsigned        writable = buf_wrapped ? BUFSIZE - buf_end : buf_len;
	ssize_t         wr = write(reader->fd, buffer + reader->buf_pos, writable);
	if (wr > 0) {
		reader->buf_pos = (reader->buf_pos + wr) % BUFSIZE;
		reset_buf_start();
	}
}

void
add_reader(const char *name, ino_t inode)
{
	struct reader  *r = malloc(sizeof(struct reader));
	r->name = strdup(name);
	r->inode = inode;
	r->pid = 0;
	r->fd = -1;
	r->buf_pos = buf_end;
	r->next = readers;
	readers = r;
}

bool
del_reader(pid_t pid)
{
	struct reader  *curr = readers;
	struct reader  *prev = 0;
	while (curr) {
		struct reader  *next = curr->next;
		if (curr->pid == pid) {
			if (prev)
				prev->next = next;
			else
				readers = next;
			free((char *) curr->name);
			free(curr);
			return true;
		}
		prev = curr;
		curr = next;
	}
	return false;
}

void
start_reader(struct reader *reader)
{
	int             fd[2];

	if (pipe(fd)) {
		err2("Could not create pipe to reader ", reader->name);
		_exit(111);
	}
	reader->pid = start_supervise(reader->name, fd[0], FD_STDOUT);
	close(fd[0]);
	reader->fd = fd[1];
	set_ndelay(reader->fd);
}

void
stop_reader(struct reader *reader)
{
	stop_supervise(reader->name, reader->pid);
}

void
stop_readers(void)
{
	struct reader  *reader;
	for (reader = readers; reader; reader = reader->next)
		stop_reader(reader);
}

void
reap_children(void)
{
	pid_t           pid;
	int             status;
	while ((pid = waitpid(0, &status, WNOHANG)) > 0) {
		if (!del_reader(pid))
			err("Caught exit of unknown process");
	}
}

void
scan_dirs(void)
{
	direntry       *entry;
	DIR            *dir = opendir(".");
	struct reader  *reader;
	struct reader  *prev;

	if (!dir) {
		err("Unable to read directory");
		return;
	}

	/*- Clear all the marked flags */
	for (reader = readers; reader; reader = reader->next)
		reader->marked = false;

	/*-
	 * For each directory entry, mark the corresponding reader.
	 * If a matching reader is not found, make one.
	 */
	while ((entry = readdir(dir)) != 0) {
		struct stat     statbuf;
		if (entry->d_name[0] == '.' || !strcmp(entry->d_name, "supervise"))
			continue;
		if (stat(entry->d_name, &statbuf))
			continue;
		if (!S_ISDIR(statbuf.st_mode))
			continue;
		for (reader = readers; reader; reader = reader->next) {
			if (reader->inode == statbuf.st_ino) {
				reader->marked = true;
				break;
			}
		}
		if (!reader) {
			add_reader(entry->d_name, statbuf.st_ino);
			start_reader(readers);
			readers->marked = true;
		}
	}
	closedir(dir);

	reap_children();

	/*- Clean up any reader that was removed from the directory */
	prev = 0;
	reader = readers;
	while (reader) {
		struct reader  *next = reader->next;

		if (!reader->marked) {
			/*-
			 * Don't stop it, since the directory is no longer there
			 * stop_reader(reader);
			 */
			if (prev)
				prev->next = next;
			else
				readers = next;
			close(reader->fd);
			free((char *) reader->name);
			free(reader);
		}
		prev = reader;
		reader = next;
	}
}

#define EVENT_INTR 0
#define EVENT_ALRM 1
static int      selfpipe[2];

void
read_event(void)
{
	char            buf[1];

	if (read(selfpipe[0], buf, 1) != 1)
		return;
	switch (buf[0])
	{
	case EVENT_INTR:
		buf_eof = true;
		break;
	case EVENT_ALRM:
		scan_dirs();
		alarm(5);
		break;
	default:
		err("Unknown event sent to self?!?");
	}
}

void
write_event(int event)
{
	char            buf[1];
	buf[0] = event;
	if (write(selfpipe[1], buf, 1) != 1)
		err("Could not send event to self");
}

void
handle_signal(int sig)
{
	int             event;
	switch (sig)
	{
	case SIGALRM:
		event = EVENT_ALRM;
		break;
	default:
		event = EVENT_INTR;
		break;
	}
	write_event(event);
}

void
main_loop(void)
{
	for (;;) {
		struct reader  *reader;
		fd_set          readfds;
		fd_set          writefds;
		int             fdmax = selfpipe[0];
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_SET(selfpipe[0], &readfds);
		if (buf_eof) {
			if (buf_start == buf_end)
				return;
		} else
		if (buf_left)
			FD_SET(FD_STDIN, &readfds);
		for (reader = readers; reader; reader = reader->next) {
			if (reader->buf_pos != buf_end) {
				int             fd = reader->fd;
				FD_SET(fd, &writefds);
				if (fd > fdmax)
					fdmax = fd;
			}
		}
		if (select(fdmax + 1, &readfds, &writefds, 0, 0) == -1)
			continue;
		/*
		 * If an event arrived, skip all other I/O
		 */
		if (FD_ISSET(selfpipe[0], &readfds)) {
			read_event();
			continue;
		}
		if (FD_ISSET(FD_STDIN, &readfds))
			read_input();
		for (reader = readers; reader; reader = reader->next)
			if (FD_ISSET(reader->fd, &writefds))
				write_output(reader);
	}
}

int
main(int argc, char **argv)
{
	if (argc > 1 && chdir(argv[1]) != 0) {
		err2("Couldn't chdir to ", argv[1]);
		return 1;
	}
	if (pipe(selfpipe)) {
		err("Couldn't create self pipe");
		return 1;
	}
	scan_dirs();
	signal(SIGALRM, handle_signal);
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	signal(SIGQUIT, handle_signal);
	alarm(5);
	set_ndelay(FD_STDIN);
	main_loop();
	stop_readers();
	return 0;
}

void
getversion_multipipe_c()
{
	static char    *x = "$Id: multipipe.c,v 1.4 2020-10-08 12:07:42+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
