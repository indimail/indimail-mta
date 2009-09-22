/*
 * $Log: misc.c,v $
 * Revision 1.4  2008-07-17 21:38:20+05:30  Cprogrammer
 * moved progname to variables.h
 *
 * Revision 1.3  2008-06-09 15:31:01+05:30  Cprogrammer
 * added GPL Copyright notice
 *
 * Revision 1.2  2002-12-21 19:08:14+05:30  Manny
 * added function unsetenv()
 *
 * Revision 1.1  2002-12-16 01:55:02+05:30  Manny
 * Initial revision
 *
 * Revision 1.1  2002-12-16 01:48:38+05:30  Manny
 * Initial revision
 *
 * Copyright (C) 1996  Stephen Fegan
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * please send patches or advice to: `flash@netsoc.ucd.ie'
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "exec.h"
#include "variables.h"

extern char   **environ;

/*
 * Print an error message (str) on stderr, print any applicable system error,
 * * wait for 1 sec (NCSA telnet) and exit 
 */

void
error(char *str)
{
	fprintf(stderr, "Error: %s\n", str);
	if (errno)
		fprintf(stderr, "%s\n", strerror(errno));
	sleep(1);
	exit(1);
}

/*
 * xmalloc()
 *
 * Input - size - number of bytes to allocate.
 * Returns - pointer to allocated space.
 *
 * This is a simple wrapper for malloc, so that we don't
 * need to worry about the return value of malloc being NULL
 */

void           *
xmalloc(size_t size)
{
	void           *retval;

	if ((retval = malloc(size)) == NULL)
		error("Out Of Memory");

	return (retval);
}

void           *
xrealloc(void *ptr, size_t size)
{
	void           *retval;

	if ((retval = realloc(ptr, size)) == NULL)
		error("Out of Memory");

	return (retval);
}

/*
 * check_type: describes the actions we take
 *  
 *      =0   : Full check. We pretend that we have seen new mail if
 *             there is some there before flash starts. Returns: 
 *             -1 error, 0 no new mail, 1 mail there (unread), 2 new mail.
 *
 *      =1   : Just peek to see if the mail status has changed.
 *             1 it has, 0 it has not;
 */

int
mail_check(int check_type)
{
	static time_t   mail_notify_time = 0;
	static int      was_there_mail_last_time = 0;
#ifdef MAILDIR					/*- mailbox is in MAILDIR format */
	struct stat     cur_file_stat, new_file_stat;
	int             namelen = 20;
	char            cur_name[namelen + 1], new_name[namelen + 1];
#else /*- mailbox or other */
	struct stat     mail_file_stat;
#endif /*- MAILDIR */
	time_t          at, mt;
	char           *mail_file_name;
	int             retval;

	mail_file_name = getenv("MAIL");
	if (mail_file_name == NULL)
	{
		was_there_mail_last_time = 0;
		return (check_type != 0) ? 0 : -1;
	}
#ifdef MAILDIR
	/*- assumes 'mail_file_name' is Maildir/ */
	snprintf(cur_name, namelen, "%s%s", mail_file_name, "cur/");
	snprintf(new_name, namelen, "%s%s", mail_file_name, "new/");

	if (stat(cur_name, &cur_file_stat) < 0)
	{
		was_there_mail_last_time = 0;
		return (check_type != 0) ? 0 : -1;
	}
	if (stat(new_name, &new_file_stat) < 0)
	{
		was_there_mail_last_time = 0;
		return (check_type != 0) ? 0 : -1;
	}

	at = cur_file_stat.st_mtime;
	mt = new_file_stat.st_mtime;
#else
	if (stat(mail_file_name, &mail_file_stat) < 0)
	{
		was_there_mail_last_time = 0;
		return (check_type != 0) ? 0 : -1;
	}

	at = mail_file_stat.st_atime;
	mt = mail_file_stat.st_mtime;
#endif

	/*
	 * Return the nomail status (retval=0) if the mail has been read (atime)
	 * since it was last written (mtime) OR if flash has been started since
	 * it was written to ie. no notification is sent on login.
	 *
	 * Otherwise return mail present status (retval=1 or 2). Return 2 if
	 * there was no mail the last time we checked; this status might invoke
	 * a beep or something while retval=1 should only make the system keep
	 * displaying mail notification.
	 */

	if ((at >= mt) || (mt < start_time))
		retval = 0;
	else
	{
		if ((mt > mail_notify_time) && (was_there_mail_last_time == 0))
			retval = 2;
		else
			retval = 1;

		if (check_type == 0)
			time(&mail_notify_time);
	}

	if (check_type != 0)
		retval = ((retval > 0) != was_there_mail_last_time);
	else
		was_there_mail_last_time = (retval > 0);

	return retval;
}

int
tty_cbreak(int fd, struct termios *saved)
{
	struct termios  buf;

	if (tcgetattr(fd, saved) < 0)
		return (-1);

	buf = *saved;

	buf.c_lflag &= ~(ICANON | ECHO);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;

	if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
		return (-1);

	return (0);
}

int
tty_reset(int fd, struct termios *saved)
{
	if (tcsetattr(fd, TCSAFLUSH, saved) < 0)
		return (-1);

	return (0);
}

void
pressanykey(void)
{
	struct termios  saved;
	char            cc;
	int             olderrno, n;

	fprintf(stderr, "[----Hit the ENTER key when you're ready----] ");
	tcflush(0, TCIFLUSH);
	tty_cbreak(0, &saved);
	do
	{
		olderrno = errno;
		do
		{
			errno = 0;
			n = read(0, &cc, 1);
			if (n == 0)
				exit(0);
			if (errno == EIO)
				GRAB_BACK_TTY;
		}
		while (errno == EIO && errno != ENOTTY);
	}
	while (cc != '\n');
	tcflush(0, TCIFLUSH);
	tty_reset(0, &saved);

	return;
}

#ifndef HAVE_SETENV
static int      copied = 0;
static int      emalloc = 0;

static void
copyenv(void)
{
	char          **x, **y;
	int             n;

	for (x = environ, n = 0; *x != NULL; x++)
		n++;
	n++;
	x = environ;

	emalloc = n + 10;
	y = environ = xmalloc(emalloc * sizeof(*environ));

	while (*x != NULL)
	{
		int             n = strlen(*x) + 1;
		*y = xmalloc(n);
		memcpy(*y, *x, n);
		x++, y++;
	}
	*y = NULL;

	copied = 1;
	return;
}

int
setenv(const char *name, const char *value, int overwrite)
{
	char          **x;
	int             n, nlen;

	if (!copied)
		copyenv();

	if (strchr(name, '=') != NULL)
	{
		fprintf(stderr, "%s: setenv: Illegal variable name %s", progname, name);
		return -1;
	}

	nlen = strlen(name);

	x = environ;
	n = 0;
	while ((*x != NULL) && ((memcmp(*x, name, nlen) != 0) || (*(*x + nlen) != '=')))
		x++, n++;
	n++;

	if (*x == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "setenv: Setting NEW variable %s\n", name);
#endif
		if (emalloc < n)
		{
			emalloc = n + 10;
			x = environ = xrealloc(environ, emalloc * sizeof(*environ));
			while (*x != NULL)
				x++;
		}
		*x = xmalloc(nlen + strlen(value) + 2);
		strcpy(*x, name);
		*(*x + nlen) = '=';
		strcpy(*x + nlen + 1, value);
		*(++x) = NULL;
	} else
	if (overwrite)
	{
		int             vlen = strlen(value);
#ifdef DEBUG
		fprintf(stderr, "setenv: Resetting variable %s\n", name);
#endif
		free(*x);
		*x = xmalloc(nlen + vlen + 2);
		strcpy(*x, name);
		*(*x + nlen) = '=';
		strcpy(*x + nlen + 1, value);
	}
#ifdef DEBUG
	else
		fprintf(stderr, "setenv: NOT setting variable %s\n", name);
#endif

	return 1;
}
#endif

#ifndef HAVE_UNSETENV
void
unsetenv(const char *name)
{
	char          **x, **y;
	int             nlen;

	if (!copied)
		copyenv();
	if (strchr(name, '=') != NULL)
	{
		fprintf(stderr, "%s: unsetenv: Illegal variable name %s", progname, name);
		return;
	}
	nlen = strlen(name);
	x = environ;
	while ((*x != NULL) && ((memcmp(*x, name, nlen) != 0) || (*(*x + nlen) != '=')))
		x++;
	if (*x != NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "unsetenv: Removing variable %s\n", name);
#endif
		free(*x);
		y = x;
		x++;
		while (*y != NULL)
			*(y++) = *(x++);
	}
#ifdef DEBUG
	else
		fprintf(stderr, "unsetenv: Could not find variable %s\n", name);
#endif
	return;
}
#endif
