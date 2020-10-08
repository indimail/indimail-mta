/*
 * $Log: run_init.c,v $
 * Revision 1.1  2020-10-08 18:25:19+05:30  Cprogrammer
 * Initial revision
 *
 */
#ifdef USE_RUNFS
#include <unistd.h>
#include <strerr.h>
#include <str.h>
#include <fmt.h>
#include <libgen.h>
void
run_init(char *service_dir, char *fatal)
{
	char           *run_dir, *p, *s;
	char            buf[256], dirbuf[256];
	int             i;

	if (!access("/run", F_OK)) {
		run_dir = "/run";
	} else
	if (!access("/var/run", F_OK)) {
		run_dir = "/var/run";
	} else
		return;
	if ((i = str_len(service_dir)) > 255)
		return;
	s = buf;
	s += fmt_str(s, service_dir);
	*s++ = 0;
	p = basename(buf);
	if (!str_diff(p, "log")) {
		if (!getcwd(buf, 255))
			strerr_die2sys(111, fatal, "unable to get current working directory: ");
		/*-
		 * dirname needs to be called
		 * this will put a null before the last component.
		 * Next call to basename will get the 2nd last
		 * component
		 */
		s = dirname(buf);
		p = basename(buf);
		i = fmt_str(0, run_dir) + 13 + fmt_str(0, p);
		if (i > 255)
			return;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		s += fmt_strn(s, "/log", 4);
		*s++ = 0;
	} else {
		i = fmt_str(0, run_dir) + 9 + fmt_str(0, p);
		if (i > 255)
			return;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		*s++ = 0;
	}
	if (chdir(dirbuf) == -1)
		strerr_die4sys(111, fatal, "unable to chdir to ", dirbuf, ": ");
	return;
}
#endif

void
getversion_svrun_c()
{
	static char    *x = "$Id: run_init.c,v 1.1 2020-10-08 18:25:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
