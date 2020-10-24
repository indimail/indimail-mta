/*
 * $Log: hier.c,v $
 * Revision 1.1  2020-10-23 20:59:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <str.h>
#include <unistd.h>
#include <strerr.h>
#include <stralloc.h>
#include "auto_home.h"
#include "auto_shared.h"

stralloc        a = { 0 };

void            h(char *, int, int, int);
void            d(char *, char *, int, int, int);
void            c(char *, char *, char *, int, int, int);
char           *getdirname(char *, char **);

char           *auto_tools_home = auto_home;
extern char    *sharedir;

void
hier(inst_dir, fatal)
	char           *inst_dir;
	char           *fatal;
{
	char           *mandir_base;

	if (inst_dir && *inst_dir)
		auto_tools_home = inst_dir;

	if (!sharedir)
		sharedir = auto_shared; /*- /usr/share/indimail */
	/*- shared directory for boot, doc, man */
	if (str_diff(auto_home, auto_shared)) {
		mandir_base = auto_tools_home; /* /usr/local */
		if (!stralloc_copys(&a, auto_tools_home))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_catb(&a, "/man", 4))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_0(&a))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!access(a.s, F_OK))
			mandir_base = auto_tools_home; /* /usr/local/man, /usr/man */
		else {
			mandir_base = getdirname(auto_shared, 0);
			if (!stralloc_copys(&a, mandir_base))
				strerr_die2sys(111, fatal, "out of memory: ");
			if (!stralloc_0(&a))
				strerr_die2sys(111, fatal, "out of memory: ");
			mandir_base = a.s; /* /usr/share */
		}
		h(auto_shared, -1, -1, -1);
	} else
		mandir_base = auto_tools_home;

	h(auto_tools_home, -1, -1, -1);
	d(auto_tools_home, "bin", -1, -1, -1);
	d(auto_tools_home, "sbin", -1, -1, -1);
	d(auto_shared,     "doc", -1, -1, -1);
	d(auto_shared,     "boot", -1, -1, -1);
	d(mandir_base,     "man", -1, -1, -1);
	d(mandir_base,     "man/man1", -1, -1, -1);
	d(mandir_base,     "man/man8", -1, -1, -1);
	c(auto_tools_home, "bin",      "envdir", -1, -1, 0755);
	c(auto_tools_home, "bin",      "envuidgid", -1, -1, 0755);
	c(auto_tools_home, "bin",      "logselect", -1, -1, 0755);
	c(auto_tools_home, "bin",      "multipipe", -1, -1, 0755);
	c(auto_tools_home, "bin",      "qfilelog", -1, -1, 0755);
	c(auto_tools_home, "bin",      "qlogselect", -1, -1, 0755);
	c(auto_tools_home, "bin",      "qmailctl", -1, -1, 0755);
	c(auto_tools_home, "bin",      "setlock", -1, -1, 0755);
	c(auto_tools_home, "bin",      "setuidgid", -1, -1, 0755);
	c(auto_tools_home, "bin",      "softlimit", -1, -1, 0755);
	c(auto_tools_home, "bin",      "spipe", -1, -1, 0755);
	c(auto_tools_home, "bin",      "svc", -1, -1, 0755);
	c(auto_tools_home, "bin",      "svok", -1, -1, 0755);
	c(auto_tools_home, "bin",      "svstat", -1, -1, 0755);
	c(auto_tools_home, "bin",      "tai2tai64n", -1, -1, 0755);
	c(auto_tools_home, "bin",      "tai64n", -1, -1, 0755);
	c(auto_tools_home, "bin",      "tai64n2tai", -1, -1, 0755);
	c(auto_tools_home, "bin",      "tai64nlocal", -1, -1, 0755);
	c(auto_tools_home, "bin",      "tai64nunix", -1, -1, 0755);
	c(auto_tools_home, "bin",      "teepipe", -1, -1, 0755);
#ifdef LINUX
	c(auto_tools_home, "sbin",     "docker-entrypoint", -1, -1, 0755);
#endif
	c(auto_tools_home, "sbin",     "fghack", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "minisvc", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "multilog", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "pgrphack", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "readproctitle", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "supervise", -1, -1, 0755);
	c(auto_tools_home, "sbin",     "svscan", -1, -1, 0751);
	c(mandir_base,     "man/man1", "multipipe.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "qfilelog.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "qlogselect.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "spipe.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tai2tai64n.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tai64n2tai.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "teepipe.1", -1, -1, 0644);
	c(mandir_base,     "man/man8", "docker-entrypoint.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "envdir.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "envuidgid.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "fghack.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "logselect.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "minisvc.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "multilog.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "pgrphack.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "qmailctl.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "readproctitle.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "setlock.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "setuidgid.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "softlimit.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "supervise.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "svc.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "svok.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "svscan.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "svscanboot.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "tai64n.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "tai64nlocal.8", -1, -1, 0644);
	c(mandir_base,     "man/man8", "tai64nunix.8", -1, -1, 0644);
#ifdef LINUX
	c(auto_shared,     "boot",     "systemd", -1, -1, 0644);
	c(auto_shared,     "boot",     "upstart", -1, -1, 0644);
#endif
#ifdef FREEBSD
	c(auto_shared,     "boot",     "svscan_rc", -1, -1, 0644);
#endif
#ifdef DARWIN
	c(auto_shared,     "boot",     "StartupParameters.plist", auto_uido, 0, 0444);
	c(auto_shared,     "boot",     "svscan.plist", auto_uido, 0, 0444);
#endif
	c(auto_shared,     "doc",      "README.logselect", -1, -1, 0644);
}
