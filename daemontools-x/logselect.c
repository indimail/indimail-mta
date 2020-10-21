/*
 * $Log: logselect.c,v $
 * Revision 1.1  2008-06-03 23:23:48+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "byte.h"
#include "open.h"
#include "error.h"
#include "direntry.h"
#include "stralloc.h"
#include "substdio.h"
#include "str.h"
#include "subfd.h"
#include "sorted.h"
#include "strerr.h"
#include "pathexec.h"
#include "taia.h"
#include "timestamp.h"

#define FATAL "logselect: fatal: "

void
die_usage(void)
{
	strerr_die1x(111, "logselect: usage: logselect dir start stop");
}

void
die_dot(void)
{
	strerr_die2x(111, FATAL, "dir must not have '/' in it.");
}

void
die_nofile(void)
{
	strerr_die2x(111, FATAL, "a timestamp file does not exist.");
}

void
nomem(void)
{
	strerr_die2x(111, FATAL, "out of memory");
}

char            inbuf[1024];
substdio        ssin;

int
get(char *ch)
{
	int             r;

	r = substdio_get(&ssin, ch, 1);
	if (r == 1)
		return 1;
	if (r == 0)
		return 0;
	_exit(111);
}

static stralloc sa;
static char     starttime[TIMESTAMP];
static char     stoptime[TIMESTAMP];

void
out(char *buf, int len)
{
	if (substdio_put(subfdout, buf, len) == -1)
		_exit(111);
}

void
do_file(char *dirname, char *fn)
{
	int             fd;
	char            ch;
	char            tai64[25 + 1];
	int             i;
	int             r;

	if ((fd = open_read(fn)) == -1)
		strerr_die6sys(111, FATAL, "unable to read ", dirname, "/", fn, ": ");
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);
	for (;;)
	{
		if (!(r = get(&ch)))
			break;
		if (ch == '@')
		{
			i = 0;
			tai64[i++] = ch;
			for (;;)
			{
				if (!(r = get(&ch)))
					break;
				if (ch == '\n')
					break;
				if (i >= 25)
					break;
				tai64[i++] = ch;
			}
			if (i == 25)
			{
				tai64[25] = '\0';
				if (str_diff(tai64, starttime) < 0 || str_diff(tai64, stoptime) >= 0)
				{
					while (ch != '\n')
					{
						if (!(r = get(&ch)))
							break;
					}
					if (!r)
						break;
					continue;
				}
			}
			out(tai64, i);
		}
		if (!r)
			break;
		for (;;)
		{
			out(&ch, 1);
			if (ch == '\n')
				break;
			if (!(r = get(&ch)))
				break;
		}
		if (!r)
			break;
	}
	close(fd);
}

static char     hex[16] = "0123456789abcdef";

void
fmt_timestamp(struct taia *t, char s[TIMESTAMP])
{
	char            nowpack[TAIA_PACK];
	int             i;

	taia_pack(nowpack, t);

	s[0] = '@';
	for (i = 0; i < 12; ++i)
	{
		s[i * 2 + 1] = hex[(nowpack[i] >> 4) & 15];
		s[i * 2 + 2] = hex[nowpack[i] & 15];
	}
}

int
main(int argc, char **argv)
{
	char           *dirname, *fn = 0;
	struct stat     st;
	struct taia     t;
	DIR            *dir;
	direntry       *d;
	int             i;
	sorted          sl = { 0 };

	if (!*argv++)
		die_usage();
	if (!*argv)
		die_usage();
	dirname = *argv++;
	for (i = 0; dirname[i]; i++)
		if (dirname[i] == '/')
			die_dot();
	if (*argv)
		fn = *argv++;
	if (stat(fn, &st) == -1)
		die_nofile();
	tai_unix(&t.sec, st.st_mtime);
	t.nano = 0;
	t.atto = 0;
	fmt_timestamp(&t, starttime);
	if (*argv)
		fn = *argv++;
	if (stat(fn, &st) == -1)
		die_nofile();
	tai_unix(&t.sec, st.st_mtime);
	t.nano = 0;
	t.atto = 0;
	fmt_timestamp(&t, stoptime);
	if (*argv)
		die_usage();
	if (chdir(dirname) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to directory ", dirname, ": ");
	if (!(dir = opendir(".")))
		strerr_die4sys(111, FATAL, "unable to read directory ", dirname, ": ");
	for (;;)
	{
		errno = 0;
		if (!(d = readdir(dir)))
		{
			if (errno)
				strerr_die4sys(111, FATAL, "unable to read directory ", dirname, ": ");
			break;
		}
		if (d->d_name[0] == '@')
		{
			if (!stralloc_copys(&sa, d->d_name))
				nomem();
			if (!stralloc_0(&sa))
				nomem();
			if (!sorted_insert(&sl, &sa))
				nomem();
		}
	}
	closedir(dir);
	if (!stralloc_copys(&sa, "current"))
		nomem();
	if (!stralloc_0(&sa))
		nomem();
	if (!sorted_insert(&sl, &sa))
		nomem();
	i = sl.len - 1;
	for (;;)
	{
		if (str_diff(sl.p[i].s, starttime) < 0)
		{
			i++;
			break;
		}
		if (i == 0)
			break;
		i--;
	}
	for (;;)
	{
		do_file(dirname, sl.p[i].s);
		if (str_diff(sl.p[i].s, stoptime) > 0)
			break;
		i++;
		if (i >= sl.len)
			strerr_die2sys(100, FATAL, "examined too many files.");
	}
	substdio_flush(subfdout);
	return(0);
}

void
getversion_logselect_c()
{
	static char    *x = "$Id: logselect.c,v 1.1 2008-06-03 23:23:48+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
