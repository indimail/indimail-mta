/*
 * $Log: etrn.c,v $
 * Revision 1.15  2019-05-27 20:26:33+05:30  Cprogrammer
 * use VIRTUAL_PKG_LIB env variable if defined
 *
 * Revision 1.14  2019-05-27 12:24:33+05:30  Cprogrammer
 * set full path to libindimail control file
 *
 * Revision 1.13  2019-05-26 12:27:30+05:30  Cprogrammer
 * use libindimail control file to load libindimail if VIRTUAL_PKG_LIB env variable not defined
 *
 * Revision 1.12  2019-04-20 19:48:42+05:30  Cprogrammer
 * change in loadLibrary() interface
 *
 * Revision 1.11  2018-07-01 11:48:57+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.10  2018-01-09 11:36:22+05:30  Cprogrammer
 * load count_dir() using loadLibrary()
 *
 * Revision 1.9  2011-07-29 09:28:21+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.8  2008-06-25 23:15:38+05:30  Cprogrammer
 * change for 64 bit port of indimail
 *
 * Revision 1.7  2007-12-20 12:43:59+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.6  2004-10-22 20:24:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2003-10-23 01:19:58+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-09-27 21:29:55+05:30  Cprogrammer
 * added code for reading morercpthosts
 *
 * Revision 1.3  2002-09-04 01:49:41+05:30  Cprogrammer
 * added function count_dir()
 *
 * Revision 1.2  2002-08-25 19:44:57+05:30  Cprogrammer
 * exitcodes logic enhanced
 *
 * Revision 1.1  2002-08-25 03:29:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "rcpthosts.h"
#include "etrn.h"
#include "case.h"
#include "sig.h"
#include "stralloc.h"
#include "constmap.h"
#include "control.h"
#include "auto_control.h"
#include "variables.h"
#include "str.h"
#include "fmt.h"
#include "auto_qmail.h"
#include "env.h"
#include "wait.h"
#include "indimail_stub.h"
#include <ctype.h>
#include <sys/types.h>
#include <unistd.h>

int             err_child();
int             err_library();
void            die_nomem();
void            die_control();

static char    *binetrnargs[3] = { 0, 0, 0 };

int
etrn_queue(char *arg, char *remoteip)
{
	int             child, r, flagetrn, len, exitcode, wstat;
	size_t          mailcount;
	static stralloc etrn = { 0 }, libfn = { 0 };
	char            maildir1[1024], maildir2[1024];
	char           *errstr, *ptr;
	struct constmap mapetrn;
	static int      flagrcpt = 1;
	size_t          (*count_dir) (char *, size_t *);
	void           *phandle = (void *) 0;

	if (flagrcpt)
		flagrcpt = rcpthosts_init();
	if ((flagetrn = control_readfile(&etrn, "etrnhosts", 0)) == -1)
		die_control();
	if (flagrcpt || !flagetrn)
		return(-2);
	if (!constmap_init(&mapetrn, etrn.s, etrn.len, 0))
		die_nomem();
	case_lowerb(arg, len = str_len(arg)); /*- convert into lower case */
	if (!constmap(&mapetrn, arg, len))
		return(-2);
	if (rcpthosts(arg, len, 1) != 1)
		return(-2);
	if ((r = fmt_strn(maildir1, auto_qmail, 1024)) > 128)
		return(-1);
	r += fmt_str(maildir1 + r, "/autoturn/");
	r += fmt_strn(maildir1 + r, arg, 119);
	if (r > 256)
		return(-1);
	r += fmt_str(maildir1 + r, "/Maildir/");
	maildir1[r] = 0;
	if ((r = fmt_strn(maildir2, auto_qmail, 1024)) > 128)
		return(-1);
	r += fmt_str(maildir2 + r, "/autoturn/");
	r += fmt_strn(maildir2 + r, remoteip, 119);
	if (r > 256)
		return(-1);
	r += fmt_str(maildir2 + r, "/Maildir/");
	maildir2[r] = 0;

	mailcount = 0;
	if (!(ptr = env_get("VIRTUAL_PKG_LIB"))) {
		if (!controldir) {
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (!stralloc_copys(&libfn, controldir))
			die_nomem();
		if (libfn.s[libfn.len - 1] != '/' && !stralloc_append(&libfn, "/"))
			die_nomem();
		if (!stralloc_catb(&libfn, "libindimail", 11) ||
				!stralloc_0(&libfn))
			die_nomem();
		ptr = libfn.s;
	} else
		ptr = "VIRTUAL_PKG_LIB";
	loadLibrary(&phandle, ptr, 0, &errstr);
	if (!phandle)
		return (err_library(errstr));
	if (!(count_dir = getlibObject(ptr, &phandle, "count_dir", &errstr)))
		return (err_library(errstr));
	if (!access(maildir1, F_OK))
		count_dir(maildir1, &mailcount);
	else
	if (!access(maildir2, F_OK))
		count_dir(maildir2, &mailcount);
	closeLibrary(&phandle);
	if (!mailcount)
		return(-3);
	switch (child = fork())
	{
	case -1:
		return(-1);
	case 0:
		sig_pipedefault();
		close(1);
		dup2(2, 1);
		binetrnargs[0] = "bin/etrn";
		binetrnargs[1] = arg;
		binetrnargs[2] = remoteip;
		execvp(*binetrnargs, binetrnargs);
		_exit(1);
	}
	if (wait_pid(&wstat, child) == -1)
		return err_child();
	if (wait_crashed(wstat))
		return err_child();
	if ((exitcode = wait_exitcode(wstat)))
	{
		if (exitcode == 4)
			return(mailcount ? mailcount : -4);
		exitcode = 0 - exitcode;
		return(exitcode); /*- no */
	}
	return (0);
}

int
valid_hostname(char *name)
{
	const char     *cp;
	int             label_length = 0;
	int             label_count = 0;
	int             ch;

	if (!name || !*name)
		return (0);
	/*
	 * Find bad characters or label lengths. Find adjacent delimiters.
	 */
	for (cp = name; (ch = *(unsigned char *) cp) != 0; cp++)
	{
		if (isalnum(ch) || ch == '_')
		{
			if (label_length == 0)
				label_count++;
			label_length++;
			if (label_length > VALID_LABEL_LEN)
				return (0);
		} else
		if (ch == '.')
		{
			if (label_length == 0 || cp[1] == 0)
				return (0);
			label_length = 0;
		} else
		if (ch == '-')
		{
			label_length++;
			if (label_length == 1 || cp[1] == 0 || cp[1] == '.')
				return (0);
		} else
			return (0);
	}
	if (cp - name > VALID_HOSTNAME_LEN)
		return (0);
	return (1);
}

void
getversion_etrn_c()
{
	static char    *x = "$Id: etrn.c,v 1.15 2019-05-27 20:26:33+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
