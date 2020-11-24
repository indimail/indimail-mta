/*
 * $Log: maildirdeliver.c,v $
 * Revision 1.8  2020-11-24 13:45:44+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2004-10-22 20:26:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:35:38+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.5  2004-10-11 13:55:03+05:30  Cprogrammer
 * added function prototypes
 *
 * Revision 1.4  2003-10-23 01:22:05+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.3  2003-08-22 23:55:42+05:30  Cprogrammer
 * deliver to maildir
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "byte.h"
#include "error.h"
#include "fmt.h"
#include "now.h"
#include "open.h"
#include "sig.h"
#include "str.h"
#include "strerr.h"
#include "substdio.h"
#include "env.h"

#define FATAL "maildirdeliver: fatal: "
#define PERM 100
#define TEMP 111

static char     inbuf[1024];
static char     outbuf[1024];

static char     fntmptph[80 + FMT_ULONG * 2];
static char     fnnewtph[80 + FMT_ULONG * 2];

static void
exitnicely(int code)
{
	unlink(fntmptph);
	_exit(code);
}

static void
sigalrm(void)
{
	strerr_warn2(FATAL, "timeout on delivery", (struct strerr *) 0);
	exitnicely(TEMP);
}

static void
die_read(void)
{
	strerr_warn2(FATAL, "unable to read message: ", &strerr_sys);
	exitnicely(TEMP);
}

int
main(int argc, char **argv)
{
	char           *dir, *ptr, *rpline, *dtline;
	int             code, loop, r;
	unsigned long   pid, time;
	char            host[64];
	struct stat     st;
	int             fd;
	substdio        ss;
	substdio        ssout;
	char            c;

	++argv;

	dir = *argv++;
	if (!dir)
		strerr_die1x(PERM, "maildirdeliver: usage: maildirdeliver dir");
	umask(077);
	sig_alarmcatch(sigalrm);
	if (chdir(dir) == -1)
	{
		code = error_temp(errno) ? TEMP : PERM;
		strerr_die4sys(code, FATAL, "unable to chdir to ", dir, ": ");
	}
	pid = getpid();
	host[0] = '\0';
	gethostname(host, sizeof(host));
	for (loop = 0;; ++loop)
	{
		time = now();
		ptr = fntmptph;
		ptr += fmt_str(ptr, "tmp/");
		ptr += fmt_ulong(ptr, time);
		*ptr++ = '.';
		ptr += fmt_ulong(ptr, pid);
		*ptr++ = '.';
		ptr += fmt_strn(ptr, host, sizeof(host));
		*ptr = 0;
		if (stat(fntmptph, &st) == -1)
			if (errno == error_noent)
				break;
		if (loop == 2)
			strerr_die2x(TEMP, FATAL, "unable to get temporary file name");
		sleep(2);
	}
	ptr = fnnewtph;
	ptr += fmt_str(ptr, fntmptph);
	/*- str_copy(fnnewtph, fntmptph); -*/
	byte_copy(fnnewtph, 3, "new");
	alarm(86400);
	if((fd = open_excl(fntmptph)) == -1)
		strerr_die4sys(TEMP, FATAL, "unable to open ", fntmptph, ": ");
	substdio_fdbuf(&ss, read, 0, inbuf, sizeof(inbuf));
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof(outbuf));
	if((rpline = env_get("RPLINE")))
	{
		if(substdio_bputs(&ssout, rpline))
		{
			strerr_warn2(FATAL, "unable to write message: ", &strerr_sys);
			exitnicely(TEMP);
		}
	}
	if((dtline = env_get("DTLINE")))
	{
		if(substdio_bputs(&ssout, dtline))
		{
			strerr_warn2(FATAL, "unable to write message: ", &strerr_sys);
			exitnicely(TEMP);
		}
	}
	/*
	 * discard initial From line, if any. 
	 */
	for (;;)
	{
		if((r = substdio_feed(&ss)) == -1)
			die_read();
		if (r == 0)
			break;
		if (r >= 5)
			break;
	}
	if ((r >= 5) && (byte_equal(substdio_PEEK(&ss), 5, "From ")))
	{
		for (;;)
		{
			if((r = substdio_get(&ss, &c, 1)) == -1)
				die_read();
			if (r == 0)
				break;
			if (c == '\n')
				break;
		}
	}
	switch (substdio_copy(&ssout, &ss))
	{
	case -2:
		die_read();
	case -3:
		strerr_warn2(FATAL, "unable to write message: ", &strerr_sys);
		exitnicely(TEMP);
	}
	if((r = substdio_flush(&ssout)) != -1)
		r = fsync(fd);
	if (r != -1)
	{
		if(fstat(fd, &st) == -1)
		{
			strerr_warn2(FATAL, "unable to fstat file: ", &strerr_sys);
			exitnicely(TEMP);
		}
		/*- for(ptr = fnnewtph;*ptr;ptr++); -*/
		ptr += fmt_str(ptr, ",S=");
		ptr += fmt_ulong(ptr, st.st_size);
		*ptr++ = 0;
		r = close(fd);
	}
	if (r == -1)
	{
		strerr_warn2(FATAL, "unable to write message: ", &strerr_sys);
		exitnicely(TEMP);
	}
	if (link(fntmptph, fnnewtph) == -1)
	{
		strerr_warn6(FATAL, "unable to link ", fntmptph, ": ", fnnewtph, ": ", &strerr_sys);
		exitnicely(TEMP);
	}
	exitnicely(0);
	return 0;	/*- shut up gcc -Wall */
}

void
getversion_maildirdeliver_c()
{
	static char    *x = "$Id: maildirdeliver.c,v 1.8 2020-11-24 13:45:44+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
