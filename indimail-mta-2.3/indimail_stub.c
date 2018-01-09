/*
 * $Log: indimail_stub.c,v $
 * Revision 1.1  2018-01-09 10:41:16+05:30  Cprogrammer
 * Initial revision
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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "auto_qmail.h"
#include "auto_sysconfdir.h"
#include "auto_uids.h"
#include "auto_control.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "stralloc.h"
#include "env.h"
#include "fmt.h"
#include "scan.h"
#include "str.h"
#include "variables.h"

/*- Fifo Server Definitions */
#define USER_QUERY   1
#define RELAY_QUERY  2
#define PWD_QUERY    3
#define HOST_QUERY   4
#define ALIAS_QUERY  5
#define LIMIT_QUERY  6
#define DOMAIN_QUERY 7

/* gid flags */
#define NO_PASSWD_CHNG          0x01
#define NO_POP                  0x02
#define NO_WEBMAIL              0x04
#define NO_IMAP                 0x08
#define BOUNCE_MAIL             0x10
#define NO_RELAY                0x20
#define NO_DIALUP               0x40
#define QA_ADMIN                0x80
#define V_OVERRIDE              0x100
#define NO_SMTP                 0x200
#define V_USER0                 0x400
#define V_USER1                 0x800
#define V_USER2                 0x1000
#define V_USER3                 0x2000

#define ATCHARS                 "@%:"
#define INDIMAIL_QMAIL_MODE     0644

#define INFIFO                  "infifo"
#define INDIMAILDIR             "/var/indimail"
#define DEFAULT_DOMAIN          "indimail.org"

stralloc        querybuf = { 0 };
stralloc        myfifo = { 0 };
stralloc        InFifo = { 0 };
stralloc        tmp = { 0 };
stralloc        dirbuf = { 0 };
stralloc        _pwstruct = { 0 };
stralloc        __PWstruct = { 0 };

int             userNotFound = 0;
int             is_inactive;
int             is_overquota;
int             verbose = 0;
int             use_etrn;
static char     strnum[FMT_ULONG];
void           *handle;

void *
loadLibrary(int *errflag, char **errstr)
{
	char           *ptr;
	static stralloc errbuf = { 0 };

	if (handle) {
		if (errflag)
			*errflag = 0;
		if (errstr)
			*errstr = (char *) 0;
		return (handle);
	}
	if (!(ptr = env_get("VIRTUAL_PACKAGE")))
		ptr = "libindimail.so.2";
	if (errflag)
		*errflag = -1;
	if (errstr)
		*errstr = (char *) 0;
#ifdef RTLD_DEEPBIND
	if (!(handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_DEEPBIND|RTLD_NODELETE))) {
#else
	if (!(handle = dlopen(ptr, RTLD_NOW|RTLD_LOCAL|RTLD_NODELETE))) {
#endif
		if (errno == 2 && errflag) {
			*errflag = 0;
			if (errstr)
				*errstr = (char *) 0;
			return ((void *) 0);
		}
		if (!stralloc_copys(&errbuf, dlerror())) {
			if (errstr)
				*errstr = (char *) 0;
		} else
		if (!stralloc_0(&errbuf)) {
			if (errstr)
				*errstr = (char *) 0;
		} else
		if (errstr)
			*errstr = errbuf.s;
		return ((void *) 0);
	}
	dlerror();
	if (errflag)
		*errflag = 0;
	return (handle);
}

void
closeLibrary()
{
	if (handle) {
		dlclose(handle);
		handle = (void *) 0;
	}
	return;
}

void *
getFunction(char *plugin_symb, char **errstr)
{
	if (!handle)
		handle = loadLibrary(0, errstr);
	if (!handle)
		return ((void *) 0);
	return (dlsym(handle, plugin_symb));
}

int
r_mkdir(dir, mode, uid, gid)
	char           *dir;
	mode_t          mode;
	uid_t           uid;
	gid_t           gid;
{
	char           *ptr;
	int             i;

	if (!stralloc_copys(&dirbuf, dir))
		return (-1);
	if (!stralloc_0(&dirbuf))
		return (-1);
	for (ptr = dirbuf.s + 1;*ptr;ptr++) {
		if (*ptr == '/') {
			*ptr = 0;
			if (access(dirbuf.s, F_OK) && (i = mkdir(dirbuf.s, mode)) == -1)
				return (i);
			else
			if ((i = chown(dirbuf.s, uid, gid)) == -1)
				return (i);
			*ptr = '/';
		}
	}
	return (mkdir(dirbuf.s, mode));
}

stralloc        IUser = { 0 };
stralloc        IPass = { 0 };
stralloc        IGecos = { 0 };
stralloc        IDir = { 0 };
stralloc        IShell = { 0 };

struct passwd  *
strToPw(char *pwbuf, int len)
{
	char           *ptr1, *ptr2;
	int             i, row_count, pwstruct_len;
	static struct passwd pwent;

	if (!pwbuf || !*pwbuf)
		return((struct passwd *) 0);
	if (!str_diffn(pwbuf, "PWSTRUCT=", 9)) {
		pwstruct_len = len - 9;
		if (!stralloc_copyb(&_pwstruct, pwbuf + 9, len - 9))
			return((struct passwd *) 0);
	} else {
		pwstruct_len = len;
		if (!stralloc_copyb(&_pwstruct, pwbuf, len))
			return((struct passwd *) 0);
	}
	if (__PWstruct.len && !str_diffn(__PWstruct.s, _pwstruct.s, pwstruct_len))
		return((struct passwd *) &pwent);
	if (!pwent.pw_name) /*- first time */
	{
		pwent.pw_name = IUser.s;
		pwent.pw_passwd = IPass.s;
		pwent.pw_gecos = IGecos.s;
		pwent.pw_dir = IDir.s;
		pwent.pw_shell = IShell.s;
	}
	is_overquota = is_inactive = userNotFound = 0;
	if (!stralloc_copy(&__PWstruct, &_pwstruct))
		return((struct passwd *) &pwent);
	if (!str_diffn(_pwstruct.s, "No such user", 12)) {
		userNotFound = 1;
		return ((struct passwd *) 0);
	}
	for (ptr1 = ptr2 = _pwstruct.s, row_count = 0;*ptr2;ptr2++) {
		if (*ptr2 != ':')
			continue;
		*ptr2 = 0;
		switch (row_count)
		{
		case 0:
			i = str_chr(ptr1, '@');
			if (!stralloc_copyb(&IUser, ptr1, i))
				return ((struct passwd *) 0);
			else
			if (ptr1[i] && !stralloc_0(&IUser))
				return ((struct passwd *) 0);
			break;
		case 1:
			if (!stralloc_copys(&IPass, ptr1))
				return ((struct passwd *) 0);
			else
			if (!stralloc_0(&IPass))
				return ((struct passwd *) 0);
			break;
		case 2:
			scan_uint(ptr1, &pwent.pw_uid);
			break;
		case 3:
			scan_uint(ptr1, &pwent.pw_gid);
			if (pwent.pw_gid & BOUNCE_MAIL)
				is_overquota = 1;
			break;
		case 4:
			if (!stralloc_copys(&IGecos, ptr1))
				return ((struct passwd *) 0);
			else
			if (!stralloc_0(&IGecos))
				return ((struct passwd *) 0);
			break;
		case 5:
			if (!stralloc_copys(&IDir, ptr1))
				return ((struct passwd *) 0);
			else
			if (!stralloc_0(&IDir))
				return ((struct passwd *) 0);
			break;
		case 6:
			if (!stralloc_copys(&IShell, ptr1))
				return ((struct passwd *) 0);
			else
			if (!stralloc_0(&IShell))
				return ((struct passwd *) 0);
			break;
		case 7:
			if (*ptr1)
				scan_uint(ptr1, (unsigned int *) &is_inactive);
		}
		ptr1 = ptr2 + 1;
		row_count++;
	} /*- for (ptr1 = ptr2 = _pwstruct.s, row_count = 0;*ptr2;ptr2++) { */
	return(row_count == 8 ? &pwent : (struct passwd *) 0);
}

static void
getEnvConfigStr(char **source, char *envname, char *defaultValue)
{
	if (!(*source = env_get(envname)))
		*source = defaultValue;
	return;
}

int
FifoCreate(char *fifoname)
{
	struct stat     statbuf;
	int             i, len;

	errno = 0;
	if (!mkfifo(fifoname, INDIMAIL_QMAIL_MODE)) {
		if ((!getuid() || !geteuid()) && chown(fifoname, auto_uidv, auto_gidv))
			return (-1);
		return (0);
	} else
	if (errno == EEXIST) {
		if (stat(fifoname, &statbuf))
			return(-1);
		if (S_ISFIFO(statbuf.st_mode)) {
			if ((!getuid() || !geteuid()) && chown(fifoname, auto_uidv, auto_gidv))
				return (-1);
			return (0);
		}
		errno = EEXIST;
		return(-1);
	} else
	if (errno == ENOENT) {
		len = str_len(fifoname);
		i = str_rchr(fifoname, '/');
		if (i && fifoname[i])
			fifoname[i] = 0;
		if (len > i + 1) {
			if (access(fifoname, F_OK)) {
				if (r_mkdir(fifoname, 0755, auto_uidv, auto_gidv)) {
					if (i)
						fifoname[i] = '/';
					return (-1);
				}
				if (i)
					fifoname[i] = '/';
				if (mkfifo(fifoname, INDIMAIL_QMAIL_MODE))
					return (-1);
			} else {
				if (i)
					fifoname[i] = '/';
				if (mkfifo(fifoname, INDIMAIL_QMAIL_MODE))
					return (-1);
			}
		} else {
			if (mkfifo(fifoname, INDIMAIL_QMAIL_MODE)) {
				if (i)
					fifoname[i] = '/';
				return (-1);
			}
			if (i)
				fifoname[i] = '/';
		}
		if ((!getuid() || !geteuid()) && chown(fifoname, auto_uidv, auto_gidv))
			return (-1);
	}
	return (-1);
}

/*- 
 *  Format of Query Buffer
 *  |len of string - int|QueryType - 1|NULL - 1|EmailId - len1|NULL - 1|Fifo - len2|NULL - 1|Ip - len3|NULL - 1|
 */
void           *
inquery(char query_type, char *email, char *ip)
{
	int             rfd, wfd, len, bytes, idx, readTimeout, writeTimeout, pipe_size, tmperrno, relative, fd;
	static int      intBuf;
	char           *sysconfdir, *infifo_dir, *infifo, *ptr;
	static char    *pwbuf;
	void            (*sig_pipe_save) ();

	userNotFound = 0;
	switch(query_type)
	{
		case RELAY_QUERY:
			if (!ip || !*ip)
			{
				errno = EINVAL;
				return ((void *) 0);
			}
		case USER_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			break;
		default:
			errno = EINVAL;
			return ((void *) 0);
	}
	/* - Prepare the Query Buffer for the Daemon */
	if (!stralloc_ready(&querybuf, 2 + sizeof(int)))
		return ((void *) 0);
	ptr = querybuf.s;
	ptr[sizeof(int)] = query_type; /*- query type */
	ptr[1 + sizeof(int)] = 0;
	querybuf.len = sizeof(int) + 2;
	if (!stralloc_cats(&querybuf, email)) /*- email */
		return ((void *) 0);
	else
	if (!stralloc_0(&querybuf))
		return ((void *) 0);

	if (!stralloc_copyb(&myfifo, "/tmp/", 5))
		return ((void *) 0);
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (!stralloc_cats(&myfifo, strnum))
		return ((void *) 0);
	strnum[fmt_ulong(strnum, time(0))] = 0;
	if (!stralloc_cats(&myfifo, strnum))
		return ((void *) 0);
	else
	if (!stralloc_0(&myfifo))
		return ((void *) 0);

	if (!stralloc_cat(&querybuf, &myfifo)) /*- fifo */
		return ((void *) 0);
	if (ip && *ip) {
		if (!stralloc_cats(&querybuf, ip)) /*- ip */
			return ((void *) 0);
	}
	if (!stralloc_0(&querybuf))
		return ((void *) 0);
	ptr = querybuf.s;
	*((int *) ptr) = querybuf.len - sizeof(int);
	bytes = querybuf.len;

	getEnvConfigStr(&sysconfdir, "SYSCONFDIR", auto_sysconfdir);
	if (!controldir) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
	}
	getEnvConfigStr(&infifo_dir, "FIFODIR", INDIMAILDIR"/inquery");
	relative = *infifo_dir == '/' ? 0 : 1;
	if (!(infifo = env_get("INFIFO")))
		infifo = INFIFO;
	/*- Open the Fifos */
	if (*infifo == '/' || *infifo == '.') {
		if (!stralloc_copys(&InFifo, infifo))
			return ((void *) 0);
		else
		if (!stralloc_0(&InFifo))
			return ((void *) 0);
	} else {
		if (relative) {
			if (!stralloc_copys(&InFifo, auto_qmail))
				return ((void *) 0);
			else
			if (!stralloc_cats(&InFifo, infifo_dir))
				return ((void *) 0);
			else
			if (!stralloc_catb(&InFifo, "/", 1))
				return ((void *) 0);
			else
			if (!stralloc_cats(&InFifo, infifo))
				return ((void *) 0);
		} else {
			r_mkdir(infifo_dir, 0775, auto_uidv, auto_uidv);
			if (!stralloc_copys(&InFifo, infifo_dir))
				return ((void *) 0);
			else
			if (!stralloc_catb(&InFifo, "/", 1))
				return ((void *) 0);
			else
			if (!stralloc_cats(&InFifo, infifo))
				return ((void *) 0);
		}
		for (idx = 1, len = InFifo.len;;idx++) {
			InFifo.len = len;
			strnum[fmt_ulong(strnum, (unsigned long) idx)] = 0;
			if (!stralloc_catb(&InFifo, ".", 1))
				return ((void *) 0);
			else
			if (!stralloc_cats(&InFifo, strnum))
				return ((void *) 0);
			else
			if (!stralloc_0(&InFifo))
				return ((void *) 0);
			if (access(InFifo.s, F_OK))
				break;
		}
#ifdef RANDOM_BALANCING
		srand(getpid() + time(0));
#endif
		InFifo.len = len;
#ifdef RANDOM_BALANCING
		strnum[fmt_ulong(strnum, 1 + (int) ((float) (idx - 1) * rand()/(RAND_MAX + 1.0)))] = 0;
#else
		strnum[fmt_ulong(strnum, 1 + (time(0) % (idx - 1)))] = 0;
#endif
	}
	if (!stralloc_catb(&InFifo, ".", 1))
		return ((void *) 0);
	else
	if (!stralloc_cats(&InFifo, strnum))
		return ((void *) 0);
	else
	if (!stralloc_0(&InFifo))
		return ((void *) 0);
	if ((wfd = open(InFifo.s, O_WRONLY | O_NDELAY, 0)) == -1)
		return ((void *) 0);
	else 
	if (bytes > (pipe_size = fpathconf(wfd, _PC_PIPE_BUF)))
	{
		errno = EMSGSIZE;
		return ((void *) 0);
	} else
	if (FifoCreate(myfifo.s) == -1)
	{
		tmperrno = errno;
		close(wfd);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((rfd = open(myfifo.s, O_RDONLY | O_NDELAY, 0)) == -1)
	{
		tmperrno = errno;
		close(wfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	} else
	if ((sig_pipe_save = signal(SIGPIPE, SIG_IGN)) == SIG_ERR)
	{
		tmperrno = errno;
		close(rfd);
		close(wfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	} 
	if (relative) {
		if (!stralloc_copys(&tmp, sysconfdir))
			return ((void *) 0);
		else
		if (!stralloc_catb(&tmp, "/", 1))
			return ((void *) 0);
		else
		if (!stralloc_cats(&tmp, controldir))
			return ((void *) 0);
		else
		if (!stralloc_catb(&tmp, "/timeoutwrite", 13))
			return ((void *) 0);
	} else {
		if (!stralloc_copys(&tmp, controldir))
			return ((void *) 0);
		else
		if (!stralloc_catb(&tmp, "/timeoutwrite", 13))
			return ((void *) 0);
	}
	if (!stralloc_0(&tmp))
		return ((void *) 0);
	if ((fd = open(tmp.s, O_RDONLY)) == -1)
		writeTimeout = 4;
	else {
		if (read(fd, strnum, sizeof(strnum) - 1) == -1)
			writeTimeout = 4;
		close(fd);
		scan_ulong(strnum, (unsigned long *) &writeTimeout);
	}
	ptr = querybuf.s;
	if (timeoutwrite(writeTimeout, wfd, ptr, bytes) != bytes) {
		tmperrno = errno;
		signal(SIGPIPE, sig_pipe_save);
		close(wfd);
		close(rfd);
		unlink(myfifo.s);
		errno = tmperrno;
		return ((void *) 0);
	}
	signal(SIGPIPE, sig_pipe_save);
	close(wfd);
	switch(query_type)
	{
		case USER_QUERY:
		case RELAY_QUERY:
		case PWD_QUERY:
#ifdef CLUSTERED_SITE
		case HOST_QUERY:
#endif
		case ALIAS_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
		case LIMIT_QUERY:
#endif
		case DOMAIN_QUERY:
			if (relative) {
				if (!stralloc_copys(&tmp, sysconfdir))
					return ((void *) 0);
				else
				if (!stralloc_catb(&tmp, "/", 1))
					return ((void *) 0);
				else
				if (!stralloc_cats(&tmp, controldir))
					return ((void *) 0);
				else
				if (!stralloc_catb(&tmp, "/timeoutread", 12))
					return ((void *) 0);
			} else {
				if (!stralloc_copys(&tmp, controldir))
					return ((void *) 0);
				else
				if (!stralloc_catb(&tmp, "/timeoutread", 12))
					return ((void *) 0);
			}
			if (!stralloc_0(&tmp))
				return ((void *) 0);
			if ((fd = open(tmp.s, O_RDONLY)) == -1)
				readTimeout = 4;
			else {
				if (read(fd, strnum, sizeof(strnum) - 1) == -1)
					readTimeout = 4;
				close(fd);
				scan_ulong(strnum, (unsigned long *) &readTimeout);
			}
			if ((idx = timeoutread(readTimeout, rfd, (char *) &intBuf, sizeof(int))) == -1 || !idx)
			{
				tmperrno = errno;
				close(rfd);
				unlink(myfifo.s);
				errno = tmperrno;
				return((void *) 0);
			} else
			if (intBuf == -1)
			{
				close(rfd);
				unlink(myfifo.s);
				errno = 0;
				return((void *) 0);
			} else
			if (intBuf > pipe_size)
			{
				close(rfd);
				unlink(myfifo.s);
				errno = EMSGSIZE;
				return((void *) 0);
			}
			switch(query_type)
			{
				case USER_QUERY:
				case RELAY_QUERY:
					close(rfd);
					unlink(myfifo.s);
					return(&intBuf);
				case PWD_QUERY:
#ifdef ENABLE_DOMAIN_LIMITS
				case LIMIT_QUERY:
#endif
					if (!intBuf)
					{
						close(rfd);
						unlink(myfifo.s);
						errno = 0;
						return((void *) 0);
					} else
					if (!(pwbuf = (char *) realloc(pwbuf, intBuf + 1)))
					{
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return((void *) 0);
					} 
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx)
					{
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return((void *) 0);
					}
					close(rfd);
					unlink(myfifo.s);
#ifdef ENABLE_DOMAIN_LIMITS
					if (query_type == PWD_QUERY)
						return(strToPw(pwbuf, intBuf));
					else
						return (pwbuf);
#else
					return(strToPw(pwbuf, intBuf));
#endif
				case ALIAS_QUERY:
#ifdef CLUSTERED_SITE
				case HOST_QUERY:
#endif
				case DOMAIN_QUERY:
					if (!intBuf)
					{
						close(rfd);
						unlink(myfifo.s);
						userNotFound = 1;
						errno = 0;
						return((void *) 0);
					} else
					if (!(pwbuf = (char *) realloc(pwbuf, intBuf + 1)))
					{
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return((void *) 0);
					} 
					if ((idx = timeoutread(readTimeout, rfd, pwbuf, intBuf)) == -1 || !idx)
					{
						tmperrno = errno;
						close(rfd);
						unlink(myfifo.s);
						errno = tmperrno;
						return((void *) 0);
					}
					close(rfd);
					unlink(myfifo.s);
					return(pwbuf);
					break;
				default:
					break;
			}
		default:
			break;
	} /*- switch(query_type) */
	close(rfd);
	unlink(myfifo.s);
	return((void *) 0);
}

void
getversion_inquery_c()
{
	static char    *x = "$Id: indimail_stub.c,v 1.1 2018-01-09 10:41:16+05:30 Cprogrammer Exp mbhangui $";
	if (x)
		x++;
}
