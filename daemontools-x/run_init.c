/*
 * $Log: run_init.c,v $
 * Revision 1.6  2022-06-20 00:48:20+05:30  Cprogrammer
 * fixed using . for supervise directory
 *
 * Revision 1.5  2022-05-22 23:00:19+05:30  Cprogrammer
 * check /run/svscan to determine if svscan is using /run
 *
 * Revision 1.4  2021-04-16 18:59:28+05:30  Cprogrammer
 * added comments to explain code
 *
 * Revision 1.3  2020-11-30 22:52:32+05:30  Cprogrammer
 * changed return type to int to return error instead of doing exit
 *
 * Revision 1.2  2020-10-20 16:27:46+05:30  Cprogrammer
 * handle . as a dir argument
 *
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

/* adapt /run filesystem for supervise */
int
run_init(char *service_dir)
{
	char           *run_dir, *p, *s;
	char            buf[256], dirbuf[256];
	int             i;

	if (!access("/run/svscan", F_OK))
		run_dir = "/run";
	else
	if (!access("/private/var/run/svscan", F_OK))
		run_dir = "/private/var/run";
	else
	if (!access("/var/run/svscan", F_OK))
		run_dir = "/var/run";
	else
		return 1;
	/*- e.g. /service/qmail-smtpd.25 */
	if ((i = str_len(service_dir)) > 255)
		return 1;
	s = buf;
	s += fmt_str(s, service_dir);
	*s++ = 0;
	p = basename(buf);
	if (!str_diff(p, ".")) {
		if (!getcwd(buf, 255))
			return -1;
		i = str_rchr(buf, '/');
		if (buf[i])
			p = buf + i + 1;
		else
			return -2;
		/*- e.g. doing svstat . in
		 * 1. /run/svscan/qmail-smtpd.25
		 * 2. /run/svscan/qmail-smtpd.25/log
		 */
		i = fmt_str(0, run_dir) + 9 + fmt_str(0, p);
		if (i > 255)
			return 1;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		*s++ = 0;
		if (access(dirbuf, F_OK)) {
			for (p -= 2;*p && *p != '/';p--);
			if (*p != '/')
				return -2;
			i = fmt_str(0, run_dir) + 9 + fmt_str(0, p + 1);
			if (i > 255)
				return 1;
			s = dirbuf;
			s += fmt_str(s, run_dir);
			s += fmt_strn(s, "/svscan/", 8);
			s += fmt_str(s, p + 1);	
			*s++ = 0;
		}
	} else
	if (!str_diff(p, "log")) {
		if (!getcwd(buf, 255)) /*- /service/name/log */
			return -1;
		i = str_rchr(buf, '/'); /*- /log */
		if (!buf[i])
			return -2;
		buf[i] = 0;
		i = str_rchr(buf, '/'); /*- /name/log */
		if (!buf[i])
			return -2;
		p = buf + i + 1; /*- name/log */
		i = fmt_str(0, run_dir) + 13 + fmt_str(0, p);
		if (i > 255)
			return 1;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		s += fmt_strn(s, "/log", 4);
		*s++ = 0;
	} else {
		/*- e.g. /run/svscan/qmail-smtpd.25 */
		i = fmt_str(0, run_dir) + 9 + fmt_str(0, p);
		if (i > 255)
			return 1;
		s = dirbuf;
		s += fmt_str(s, run_dir);
		s += fmt_strn(s, "/svscan/", 8);
		s += fmt_str(s, p);	
		*s++ = 0;
	}
	/*-
	 * we do chdir to /run/svscan/qmail-smtpd.25
	 * instead of
	 * /service/qmail-stmpd.25
	 */
	if (chdir(dirbuf) == -1)
		return -2;
	return 0;
}
#endif

void
getversion_svrun_c()
{
	static char    *x = "$Id: run_init.c,v 1.6 2022-06-20 00:48:20+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
