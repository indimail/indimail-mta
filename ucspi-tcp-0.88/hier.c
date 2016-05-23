/*
 * $Log: hier.c,v $
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
#include "auto_home.h"
#include "str.h"

void            h(char *, int, int, int);
void            d(char *, char *, int, int, int);
void            c(char *, char *, char *, int, int, int);
char           *getdirname(char *, char **);

char           *auto_ucspi_home = auto_home;
extern char    *sharedir;

void
hier(inst_dir)
	char           *inst_dir;
{
	char           *mandir;

	if (inst_dir && *inst_dir)
		auto_ucspi_home = inst_dir;

	if (sharedir && *sharedir && str_diff(auto_ucspi_home, sharedir)) {
		mandir = getdirname(sharedir, 0);
		h(sharedir, -1, -1, 0755);
	} else
		mandir = auto_ucspi_home;
	h(auto_ucspi_home, -1, -1, 0555);
	d(auto_ucspi_home, "bin", -1, -1, 0555);
	d(auto_ucspi_home, "lib/indimail/plugins", -1, -1, 0555);
	d(mandir,          "man", -1, -1, 0555);
	d(mandir,          "man/man1", -1, -1, 0555);

	c(auto_ucspi_home, "bin", "tcpserver", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "tcprules", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "tcprulescheck", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "tcpclient", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "who@", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "date@", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "finger@", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "http@", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "tcpcat", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "mconnect", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "mconnect-io", -1, -1, 0555);
	c(auto_ucspi_home, "bin", "rblsmtpd", -1, -1, 0555);
	c(mandir,          "man/man1", "tcpserver.1", -1, -1, 0644);
	c(mandir,          "man/man1", "tcprules.1", -1, -1, 0644);
	c(mandir,          "man/man1", "tcprulescheck.1", -1, -1, 0644);
	c(mandir,          "man/man1", "tcpclient.1", -1, -1, 0644);
	c(mandir,          "man/man1", "who@.1", -1, -1, 0644);
	c(mandir,          "man/man1", "date@.1", -1, -1, 0644);
	c(mandir,          "man/man1", "finger@.1", -1, -1, 0644);
	c(mandir,          "man/man1", "http@.1", -1, -1, 0644);
	c(mandir,          "man/man1", "tcpcat.1", -1, -1, 0644);
	c(mandir,          "man/man1", "mconnect.1", -1, -1, 0644);
	c(mandir,          "man/man1", "rblsmtpd.1", -1, -1, 0644);
#ifdef LOAD_SHARED_OBJECTS
	c(auto_ucspi_home, "lib/indimail/plugins", "rblsmtpd.so", -1, -1, 0555);
#endif
}
