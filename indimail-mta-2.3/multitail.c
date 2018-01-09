/*
 * $Log: multitail.c,v $
 * Revision 1.2  2004-10-22 20:27:37+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-27 22:56:15+05:30  Cprogrammer
 * Initial revision
 *
 *
 * multitail -- Watch the output from a utility such as multilog
 * Copyright (C) 2000  Bruce Guenter <bruceg@em.ca>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "systime.h"
#include "bool.h"

#define BLKSZ 4096

const int       FD_STDIN = 0;
const int       FD_STDOUT = 1;
const int       FD_STDERR = 2;

void
print(const char *msg)
{
	write(FD_STDERR, msg, strlen(msg));
}

void
warn(const char *msg)
{
	print("multitail: ");
	print(msg);
	print("!\n");
}

void
die(const char *msg)
{
	warn(msg);
	exit(1);
}

void
warn2(const char *msg1, const char *msg2)
{
	print("multitail: ");
	print(msg1);
	print(msg2);
	print("!\n");
}

void
usage(const char *error)
{
	if (error)
		warn(error);
	print("Usage: multitail [-Ee] filename\n" "  -E  Seek to end of file before output (default)\n"
		  "  -e  Do not seek to end of file before output\n");
	exit(1);
}

static const char *opt_filename;
static bool     opt_seek_end = true;

void
parse_args(int argc, char **argv)
{
	int             ch;
	while ((ch = getopt(argc, argv, "Ee")) != -1)
	{
		switch (ch)
		{
		case 'E':
			opt_seek_end = true;
			break;
		case 'e':
			opt_seek_end = false;
			break;
		default:
			usage(0);
		}
	}
	if (optind + 1 != argc)
		usage("Must include exactly one filename");
	opt_filename = argv[optind];
}

void
copy(int fd)
{
	char            buf[BLKSZ];
	ssize_t         rd;
	while ((rd = read(fd, buf, BLKSZ)) != 0)
	{
		if (rd == -1)
			die("Could not read from file");
		while (rd)
		{
			ssize_t         wr = write(FD_STDOUT, buf, rd);
			if (wr == -1)
				die("Could not write to stdout");
			rd -= wr;
		}
	}
}

void
multitail(void)
{
	int             fd = 0;
	ino_t           inode = 0;
	bool            first = true;

	for (;; sleep(1))
	{
		struct stat     buf;
		if (!fd)
		{
			while ((fd = open(opt_filename, O_RDONLY)) == -1)
			{
				warn("Could not open file, sleeping for one minute");
				sleep(60);
			}
			if (first && opt_seek_end)
			{
				if (lseek(fd, 0, SEEK_END) == -1)
					die("Could not seek to end of file");
				first = false;
			}
			fstat(fd, &buf);
			inode = buf.st_ino;
		}
		copy(fd);
		if (stat(opt_filename, &buf) == -1)
			warn("Could not stat file");
		else
		if (buf.st_ino != inode)
		{
			close(fd);
			fd = 0;
		}
	}
}

int
main(int argc, char *argv[])
{
	parse_args(argc, argv);
	multitail();
	return 1;
}

void
getversion_multitail_c()
{
	static char    *x = "$Id: multitail.c,v 1.2 2004-10-22 20:27:37+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
