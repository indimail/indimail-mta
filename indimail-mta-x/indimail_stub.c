/*
 * $Log: indimail_stub.c,v $
 * Revision 1.17  2023-04-01 19:26:44+05:30  Cprogrammer
 * refactored getlibObject function
 *
 * Revision 1.16  2021-06-28 16:58:47+05:30  Cprogrammer
 * fix SIGSEGV when libenv argument is NULL
 *
 * Revision 1.15  2021-02-07 23:12:39+05:30  Cprogrammer
 * refactored code
 *
 * Revision 1.14  2020-03-24 13:00:43+05:30  Cprogrammer
 * BUG Fix. Absence of control file shouldn't be treated as an error
 *
 * Revision 1.13  2019-06-07 19:17:47+05:30  Cprogrammer
 * return error if shared lib is missing
 *
 * Revision 1.12  2019-05-28 10:24:03+05:30  Cprogrammer
 * assign error to error buffer in getlibObject()
 *
 * Revision 1.11  2019-05-27 12:33:57+05:30  Cprogrammer
 * null terminate libfn
 *
 * Revision 1.10  2019-05-26 12:00:13+05:30  Cprogrammer
 * replaced control_readfile with control_readline
 *
 * Revision 1.9  2019-05-26 11:39:31+05:30  Cprogrammer
 * use control file mysql_lib to dlopen libmysqlclient
 *
 * Revision 1.8  2019-04-22 21:50:10+05:30  Cprogrammer
 * fixed errorneous hardcoding
 *
 * Revision 1.7  2019-04-20 19:49:34+05:30  Cprogrammer
 * changed interface for loadLibrary(), closeLibrary() and getlibObject()
 *
 * Revision 1.6  2019-04-16 23:57:44+05:30  Cprogrammer
 * added parse_email() function
 *
 * Revision 1.5  2018-07-01 11:49:17+05:30  Cprogrammer
 * renamed getFunction() to getlibObject()
 *
 * Revision 1.4  2018-03-31 00:00:41+05:30  Cprogrammer
 * fixed spurious chown in r_mkdir()
 *
 * Revision 1.3  2018-03-24 23:34:27+05:30  Cprogrammer
 * return success if VIRTUAL_PKG_LIB is not defined
 *
 * Revision 1.2  2018-01-10 11:59:09+05:30  Cprogrammer
 * return success if library path does not exist
 *
 * Revision 1.1  2018-01-09 10:41:16+05:30  Cprogrammer
 * Initial revision
 *
 */
/*
 * The following objects are needed to support indimail
 * indimail 2.x                 indimail 3.x
 * parse_email()
 * isvirtualdomain()
 * vauth_open()             --> iopen()
 * vauth_getpw()            --> sql_getpw()
 * vget_real_domain()       --> get_real_domain
 * is_distributed_domain()
 * findhost()
 * get_local_ip()
 * get_local_hostid()
 * vshow_atrn_map()         --> show_atrn_map()
 * atrn_access()
 * vclose()                 --> iclose()
 *
 * Following variables are required
 * userNotFound
 * is_inactive
 *
 */
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "str.h"
#include "control.h"
#include "env.h"
#include "stralloc.h"
#include "error.h"
#include "variables.h"
#include "indimail_stub.h"

#define ATCHARS                 "@%:"
#define DEFAULT_DOMAIN          "indimail.org"

static stralloc libfn = { 0 };
static char     memerr[] = "out of memory";
static char     ctlerr[] = "unable to read controls";
static stralloc errbuf = { 0 };

void *
loadLibrary(void **handle, char *libenv, int *errflag, char **errstr)
{
	char           *ptr;
	int             i;

	if (!libenv) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		if (handle && *handle)
			return (handle);
		else
			return ((void *) 0);
	}
	if (*libenv == '/') { /*- filename */
		if ((i = control_readline(&libfn, libenv)) == -1 || !i) {
			if (errstr)
				*errstr = (char *) 0;
			if (!i) {
				*errflag = 0;
				return ((void *) 0);
			}
			if (errflag)
				*errflag = errno;
			if (!stralloc_copys(&errbuf, ctlerr) ||
					!stralloc_catb(&errbuf, ": ", 2) ||
					!stralloc_copys(&errbuf, error_str(errno)) ||
					!stralloc_0(&errbuf)) {
				if (errstr)
					*errstr = memerr;
			} else
			if (errstr)
				*errstr = errbuf.s;
			return ((void *) 0);
		}
		if (!stralloc_0(&libfn)) {
			if (errstr)
				*errstr = memerr;
			return ((void *) 0);
		}
		ptr = libfn.s;
	} else
	if (!(ptr = env_get(libenv))) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return ((void *) 0);
	} else
	if (handle && *handle) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return (handle);
	}
	if (errflag)
		*errflag = -1;
	if (errstr)
		*errstr = (char *) 0;
	if (*libenv != '/' && access(ptr, R_OK)) {
		if (errflag)
			*errflag = errno;
		if (!stralloc_copys(&errbuf, error_str(errno)) ||
				!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
#ifdef RTLD_DEEPBIND
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
	if (!(*handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
		if (!stralloc_copys(&errbuf, dlerror()) ||
				!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = memerr;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
	dlerror();
	if (errflag)
		*errflag = 0;
	return (*handle);
}

void
closeLibrary(void **handle)
{
	if (*handle) {
		dlclose(*handle);
		*handle = (void *) 0;
	}
	return;
}

void *
getlibObject(char *libenv, void **handle, char *plugin_symb, char **errstr)
{
	void           *i;
	char           *ptr;

	if (!*handle)
		*handle = loadLibrary(handle, libenv, 0, errstr);
	if (!*handle)
		return ((void *) 0);
	if (!(i = dlsym(*handle, plugin_symb)) && errstr) {
		if (!stralloc_copyb(&errbuf, "getlibObject: ", 14) ||
				!stralloc_cats(&errbuf, plugin_symb))
			*errstr = memerr;
		if ((ptr = dlerror())) {
			if (!stralloc_cats(&errbuf, ptr) ||
					!stralloc_catb(&errbuf, ": ", 2))
				*errstr = memerr;
		}
		if (!stralloc_0(&errbuf))
			*errstr = memerr;
		*errstr = errbuf.s;
	}
	return (i);
}

static void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = env_get(envname)))
		*source = defaultValue;
	return;
}

/*
 * parse out user and domain from an email address utility function
 *
 * email  = input email address
 * user   = parsed user
 * domain = parsed domain
 * return   0 success
 *         -1 if either user or domain was truncated due to buff_size being reached
 */
int
parse_email(char *email, stralloc *user, stralloc *domain)
{
	char           *ptr;
	int             i, len;

	for (len = 0, ptr = email; *ptr; ptr++, len++) {
		i = str_chr(ATCHARS, *ptr);
		if (ATCHARS[i])
			break;
	}
	if (len) {
		if (!stralloc_copyb(user, email, len) || !stralloc_0(user))
			return (-1);
		user->len--;
	} else {
		if (!stralloc_0(user))
			return (-1);
		user->len = 0;
	}
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	if (!stralloc_copys(domain, ptr) || !stralloc_0(domain))
		return (-1);
	domain->len--;
	return (0);
}

void
getversion_indimail_stub_c()
{
	static char    *x = "$Id: indimail_stub.c,v 1.17 2023-04-01 19:26:44+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
