/*
 * $Log: hier.c,v $
 * Revision 1.15  2020-10-19 22:32:01+05:30  Cprogrammer
 * moved argv0, addcr, delcr, fixcrio, recordio from indimail-mta to ucspi-tcp
 *
 * Revision 1.14  2020-09-19 17:35:03+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.13  2020-09-13 17:32:28+05:30  Cprogrammer
 * leave permissions of directories alone as they are owned by indimail-mta
 *
 * Revision 1.12  2020-08-03 17:23:42+05:30  Cprogrammer
 * use qmail library
 *
 * Revision 1.11  2017-04-13 00:40:49+05:30  Cprogrammer
 * added README.ucspi-tcp
 *
 * Revision 1.10  2016-06-20 08:30:59+05:30  Cprogrammer
 * added man page for mconnect-io
 *
 * Revision 1.9  2016-05-27 20:46:06+05:30  Cprogrammer
 * fixed setting of permissions for FHS
 *
 * Revision 1.8  2016-05-23 20:18:47+05:30  Cprogrammer
 * use /usr/lib/indimail/plugins for .so plugins
 *
 * Revision 1.7  2016-05-23 04:42:32+05:30  Cprogrammer
 * fhs compliance
 *
 * Revision 1.6  2016-05-15 22:39:14+05:30  Cprogrammer
 * added rblsmtpd.so
 *
 * Revision 1.5  2008-06-30 09:39:14+05:30  Cprogrammer
 * changed auto_home to auto_uspi_home
 *
 * Revision 1.4  2008-06-12 14:28:45+05:30  Cprogrammer
 * changed perms of directories
 *
 * Revision 1.3  2005-06-03 09:07:01+05:30  Cprogrammer
 * added creation of man page directory
 *
 * Revision 1.2  2005-05-13 23:33:02+05:30  Cprogrammer
 * moved argv0, recordio, addcr, delcr, fixcrio to qmail
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
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

char           *auto_ucspi_home = auto_home;
extern char    *sharedir;

void
hier(inst_dir, fatal)
	char           *inst_dir;
	char           *fatal;
{
	char           *mandir_base;

	if (inst_dir && *inst_dir)
		auto_ucspi_home = inst_dir;

	if (!sharedir)
		sharedir = auto_shared;
	/*- shared directory for boot, doc, man */
	if (str_diff(auto_home, auto_shared)) {
		mandir_base = auto_ucspi_home; /* /usr/local */
		if (!stralloc_copys(&a, auto_ucspi_home))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_catb(&a, "/man", 4))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!stralloc_0(&a))
			strerr_die2sys(111, fatal, "out of memory: ");
		if (!access(a.s, F_OK))
			mandir_base = auto_ucspi_home; /* /usr/local/man, /usr/man */
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
		mandir_base = auto_ucspi_home;

	h(auto_ucspi_home, -1, -1, -1);
	d(auto_ucspi_home, "bin", -1, -1, -1);
	d(auto_shared,     "doc", -1, -1, -1);
	d(mandir_base,     "man", -1, -1, -1);
	d(mandir_base,     "man/man1", -1, -1, -1);
	c(auto_ucspi_home, "bin",      "tcpserver", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "tcprules", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "tcprulescheck", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "tcpclient", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "who@", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "date@", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "finger@", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "http@", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "tcpcat", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "mconnect", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "mconnect-io", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "rblsmtpd", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "argv0", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "addcr", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "delcr", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "fixcrio", -1, -1, 0755);
	c(auto_ucspi_home, "bin",      "recordio", -1, -1, 0755);
	c(mandir_base,     "man/man1", "tcpserver.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tcprules.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tcprulescheck.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tcpclient.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "who@.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "date@.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "finger@.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "http@.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "tcpcat.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "mconnect.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "mconnect-io.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "rblsmtpd.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "argv0.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "addcr.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "delcr.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "fixcrio.1", -1, -1, 0644);
	c(mandir_base,     "man/man1", "recordio.1", -1, -1, 0644);
	c(auto_shared,     "doc",      "README.ucspi-tcp", -1, -1, 0644);
#ifdef LOAD_SHARED_OBJECTS
	d(auto_ucspi_home, "lib/indimail/plugins", -1, -1, -1);
	c(auto_ucspi_home, "lib/indimail/plugins", "rblsmtpd.so", -1, -1, 0755);
#endif
}
