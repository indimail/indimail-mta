/*
 * $Log: sq_vacation.c,v $
 * Revision 2.11  2017-03-13 14:09:39+05:30  Cprogrammer
 * replaced INDIMAILDIR with PREFIX
 *
 * Revision 2.10  2011-10-28 14:16:29+05:30  Cprogrammer
 * added auth_method argument to pw_comp
 *
 * Revision 2.9  2011-10-25 20:49:38+05:30  Cprogrammer
 * plain text password to be passed with response argument of pw_comp()
 *
 * Revision 2.8  2011-07-29 09:26:24+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 2.7  2010-05-01 14:14:03+05:30  Cprogrammer
 * added connect_all argument to vauthOpen_user
 *
 * Revision 2.6  2010-04-11 22:22:43+05:30  Cprogrammer
 * LPWD_QUERY changed to LIMIT_QUERY
 *
 * Revision 2.5  2010-02-18 22:44:34+05:30  Cprogrammer
 * removed unused definitions
 *
 * Revision 2.4  2009-11-09 10:43:23+05:30  Cprogrammer
 * removed BUFF_SIZE definition
 *
 * Revision 2.3  2009-11-09 08:35:00+05:30  Cprogrammer
 * added definitions for default MySQL connections
 *
 * Revision 2.2  2009-11-08 02:34:38+05:30  Cprogrammer
 * removed stray debug statement
 *
 * Revision 2.1  2009-11-07 20:16:54+05:30  Cprogrammer
 * vacation plugin squirrel mail
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#include "indimail.h"
#endif
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SERVER 1
#define USER   2
#define ACTION 3
#define SRC    4
#define DEST   5

#define BUFSIZE 512

#ifndef INDIMAILH_H
#define HAVE_STDARG_H
#define QUERY_CACHE 1
#define CLUSTERED_SITE 1
#ifndef PREFIX
#define PREFIX "/usr"
#endif
#define INDIMAIL_QMAIL_MODE     0644
#define DEFAULT_DOMAIN "indimail.org"
#define AUTH_SIZE               300

#define USER_QUERY   1
#define RELAY_QUERY  2
#define PWD_QUERY    3
#ifdef CLUSTERED_SITE
#define HOST_QUERY   4
#endif
#define ALIAS_QUERY  5
#define LIMIT_QUERY  6
#define DOMAIN_QUERY 7

#endif /* INDIMAILH_H */

/*- Exit status */
#define ERR_OK          0       /*- no error */
#define ERR_NOTFOUND    1       /*- file not found */
#define ERR_BADPASS     32      /*- bad password */
#define ERR_USAGE       33      /*- usage error */
#define ERR_RESTRICTED  34      /*- not allowed to use this program */
#define ERR_REMOTEFILE  35      /*- illegal remote filename */
#define ERR_LOCALFILE   36      /*- illegal local filename */
#define ERR_CONFIG      37      /*- global configuration problem */
#define ERR_USER        38      /*- problem with this user */
#define ERR_HOME        39      /*- problem accessing home directory */
#define ERR_SOURCEFILE  40      /*- problem opening/stat()ing source file */
#define ERR_DESTFILE    41      /*- problem opening/deleting dest file */
#define ERR_COPYFILE    42      /*- problem copying file contents */
#define ERR_UNLINK      43      /*- problem unlinking file */
#define ERR_FILETYPE    44      /*- not a regular file */
#define ERR_EXEC        45      /*- exec() of vacation program failed */
#define ERR_NOTSUPPORTED 46     /*- feature not enabled */
#define ERR_AUTO_MSG    47      /*- error create .vacation.sq */
#define ERR_PRIVILEGE   125     /*- unexpected privileges problem */
#define ERR_UNEXPECTED  126     /*- other unexpected error */

#ifndef lint
static char     sccsid[] = "$Id: sq_vacation.c,v 2.11 2017-03-13 14:09:39+05:30 Cprogrammer Stab mbhangui $";
#endif
#ifndef INDIMAILH_H
int             vauthOpen_user(char *);
char           *vget_assign(char *, char *, int, uid_t *, gid_t *);
int             pw_comp(unsigned char *, unsigned char *, unsigned char *, unsigned char *, int);
void            vclose();
void            getEnvConfigStr(char **, char *, char *);
struct passwd  *vauth_getpw(char *, char *);
int             fappend(char *, char *, char *, mode_t, uid_t, gid_t);
char           *vget_real_domain(char *);
void           *inquery(char, char *, char *);
#endif

#define REMOTEFILE_OKCHARS \
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.+-_"
static uid_t orig_uid = 0;
static gid_t orig_gid = 0;
#ifndef INDIMAILH_H
extern int   userNotFound;
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
void
die(int status, const char *fmt, ...)
#else
#include <varargs.h>
void
die(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char            buf[2048];
#ifndef HAVE_STDARG_H
	char           *fmt;
	int             status;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	status = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s\n", buf);
	exit(status);
}

#ifdef HAVE_STDARG_H
#include <stdarg.h>
void
die_sys(int status, const char *fmt, ...)
#else
#include <varargs.h>
void
die(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char            buf[2048];
#ifndef HAVE_STDARG_H
	char           *fmt;
	int             status;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, fmt);
#else
	va_start(ap);
	status = va_arg(ap, int);
	fmt = va_arg(ap, char *);
#endif
	(void) vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s: %s\n", buf, strerror(errno));
	exit(status);
}

void
remoteok(char *rmtfile, char *desc)
{
	if (!rmtfile || !*rmtfile)
		die(ERR_REMOTEFILE, "%s: Remote filename cannot be empty", desc);
	if (strspn(rmtfile, REMOTEFILE_OKCHARS) != strlen(rmtfile))
		die(ERR_REMOTEFILE, "%s: %s: Remote filename contains illegal character(s)", desc, rmtfile);
	if (strstr(rmtfile, ".."))
		die(ERR_REMOTEFILE, "%s: %s: Remote filename cannot have ..", desc);
}

void
localok(char *localfile, char *desc)
{
	if (*localfile != '/')
		die(ERR_LOCALFILE, "%s: %s: must be absolute pathname", desc, localfile);
}

int
do_list(char *src, struct passwd *pw)
{
	struct stat     statbuf;

	if (!src || !*src)
		return ERR_OK;
	remoteok(src, "do_list"); /*- in home directory of user */
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (!stat(src, &statbuf))
	{
		if (!S_ISREG(statbuf.st_mode))
			die(ERR_FILETYPE, "%s: target is not a regular file", src);
	} else
	if (errno == ENOENT)
		return ERR_NOTFOUND;
	else
		die_sys(ERR_SOURCEFILE, "stat: %s", src);
	return ERR_OK;
}

int
do_get(char *src, char *dest, struct passwd *pw)
{
	remoteok(src, "do_get"); /*- in home directory */
	localok(dest, "do_get");
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
#if 0
	if (set_eugid(orig_uid, orig_gid) != 0) {
		die_sys(ERR_PRIVILEGE, "setuid(ruid)\n");
#endif
	if (fappend(src, dest, "w", INDIMAIL_QMAIL_MODE, orig_uid, orig_gid))
	{
		if (errno == ENOENT)
			return ERR_NOTFOUND;
		if (access(src, R_OK))
			die_sys(ERR_SOURCEFILE, "access %s", src);
		die_sys(ERR_DESTFILE, "fappend %s", dest);
	}
	return ERR_OK;
}

int
do_put(char *src, char *dest, struct passwd *pw, uid_t uid, gid_t gid)
{
	localok(src, "do_put");
	remoteok(dest, "do_put"); /*- in home directory of user */
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (fappend(src, dest, "w", INDIMAIL_QMAIL_MODE, uid, gid))
		die_sys(ERR_DESTFILE, "fappend %s", dest);
	return ERR_OK;
}

int
do_delete(char *src, struct passwd *pw)
{
	struct stat     statbuf;

	remoteok(src, "do_delete");
	if (chdir(pw->pw_dir))
		die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
	if (!lstat(src, &statbuf)) {
		if (S_ISREG(statbuf.st_mode)
#ifdef S_ISLNK
			|| S_ISLNK(statbuf.st_mode)
#endif /* S_ISLNK */
	) {
			if (unlink(src))
				die_sys(ERR_UNLINK, "unlink %s", src);
		} else
			die(ERR_FILETYPE, "%s: delete: not a regular file", src);
	} else
	if (errno == ENOENT)
		return ERR_NOTFOUND;
	else
		die_sys(ERR_DESTFILE, "lstat %s", src);
	return ERR_OK;
}

void
close_connection()
{
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
		vclose();
#else /*- Not QUERY_CACHE */
	vclose();
#endif
}

int
set_eugid(uid_t uid, gid_t gid)
{
	/* need euid=root to set ids */
	if (setreuid(0, 0))
		return -1;
	if (setegid(gid))
		return -1;
	if (setreuid(0, uid))
		return -1;
	return 0;
}

int
main(int argc, char **argv)
{
	char           *email = 0, *action, *ptr, *cptr, *real_domain, *crypt_pass;
	char            User[AUTH_SIZE], Domain[AUTH_SIZE], passbuf[BUFSIZE];
	char           *(vacargs[5]);
	int             fd, status = -1;
	uid_t           uid;
	gid_t           gid;
	struct passwd  *pw;

	if (argc < ACTION + 1)
		die(ERR_USAGE, "Incorrect usage");
	if (!fgets(passbuf, BUFSIZE, stdin))
		die(ERR_BADPASS, "Could not read password");
	if ((ptr = strchr(passbuf, '\n')))
		*ptr = 0;
	email = argv[USER];
	action = argv[ACTION];
	orig_uid = getuid();
	orig_gid = getgid();
	for (ptr = email, cptr = User;*ptr && *ptr != '@';*cptr++ = *ptr++);
	*cptr = 0;
	if (*ptr)
		ptr++;
	else
		getEnvConfigStr(&ptr, "DEFAULT_DOMAIN", DEFAULT_DOMAIN);
	for (cptr = Domain;*ptr;*cptr++ = *ptr++);
	*cptr = 0;
#ifdef QUERY_CACHE
	if (!getenv("QUERY_CACHE"))
	{
#ifdef CLUSTERED_SITE
		if (vauthOpen_user(email, 0))
#else
		if (vauth_open((char *) 0))
#endif
			die(ERR_UNEXPECTED, "vauth_open: unable to connect to IndiMail database");
	}
#else
#ifdef CLUSTERED_SITE
	if (vauthOpen_user(email, 0))
#else
	if (vauth_open((char *) 0))
#endif
		die(ERR_UNEXPECTED, "vauth_open: unable to connect to IndiMail database");
#endif
#ifdef QUERY_CACHE
	if (getenv("QUERY_CACHE"))
	{
		pw = inquery(PWD_QUERY, email, 0);
		if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
			die(ERR_USER, "%s: domain does not exist", Domain);
	} else
	{
		if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
			die(ERR_USER, "%s: domain does not exist", Domain);
		if (!(real_domain = vget_real_domain(Domain)))
			real_domain = Domain;
		pw = vauth_getpw(User, real_domain);
	}
#else
	if (!vget_assign(Domain, 0, 0, &uid, &gid)) 
		die(ERR_USER, "%s: domain does not exist", Domain);
	if (!(real_domain = vget_real_domain(Domain)))
		real_domain = Domain;
	pw = vauth_getpw(User, real_domain);
#endif
	if (!pw)
	{
		if(userNotFound)
			die(ERR_USER, "%s@%s: user not found", User, Domain);
		else
			die(ERR_UNEXPECTED, "inquery failed");
		close_connection();
		die(ERR_USER, "failed to get pw info for %s@%s", User, Domain);
	}
	crypt_pass = pw->pw_passwd;
	if (pw_comp((unsigned char *) email, (unsigned char *) crypt_pass,
		0, (unsigned char *) passbuf, 0))
		die(ERR_BADPASS, "Password does not match");
	if (!strncmp(action, "list", 4))
	{
		if (argc != SRC + 1)
			die(ERR_USAGE, "Incorrect usage for list");
		status = do_list(argv[SRC], pw);
	} else
	if (!strncmp(action, "get", 3))
	{
		if (argc != DEST + 1)
			die(ERR_USAGE, "Incorrect usage for get");
		status = do_get(argv[SRC], argv[DEST], pw);
	} else
	if (!strncmp(action, "put", 3))
	{
		if (argc != DEST + 1)
			die(ERR_USAGE, "Incorrect usage for put");
		status = do_put(argv[SRC], argv[DEST], pw, uid, gid);
	} else
	if (!strncmp(action, "delete", 6))
	{
		if (argc != SRC + 1)
			die(ERR_USAGE, "Incorrect usage for delete");
		status = do_delete(argv[SRC], pw);
		if (setgid(gid))
			die_sys(ERR_PRIVILEGE, "setgid %d", gid);
		else
		if (setuid(uid))
			die_sys(ERR_PRIVILEGE, "setuid %d", uid);
		vacargs[0] = PREFIX"/bin/vmoduser";
		vacargs[1] = "-l";
		vacargs[2] = "-"; /*- remove autoresponder */
		vacargs[3] = email;
		vacargs[4] = 0;
		execv(*vacargs, vacargs);
		die_sys(ERR_EXEC, "execv: %s/bin/vmoduser -l- %s", PREFIX, email);
		exit(ERR_EXEC);
	} else
	if (!strncmp(action, "init", 4))
	{
		if (argc != ACTION + 1 || !email)
			die(ERR_USAGE, "Incorrect usage for init");
		if (chdir(pw->pw_dir))
			die_sys(ERR_HOME, "chdir %s", pw->pw_dir);
		if (setgid(gid))
			die_sys(ERR_PRIVILEGE, "setgid %d", gid);
		else
		if (setuid(uid))
			die_sys(ERR_PRIVILEGE, "setuid %d", uid);
		if ((fd = open(".vacation.msg", O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR)) == -1)
			die_sys(ERR_AUTO_MSG, ".vacation.msg");
		close(fd);
		vacargs[0] = PREFIX"/bin/vmoduser";
		vacargs[1] = "-l";
		vacargs[2] = ".vacation.msg";
		vacargs[3] = email;
		vacargs[4] = 0;
		execv(*vacargs, vacargs);
		die_sys(ERR_EXEC, "execv: %s/bin/vmoduser -l .vacation.msg %s", PREFIX, email);
	}
	exit(status);
}

void
getversion_sq_vacation_c()
{
	printf("%s\n", sccsid);
#ifdef INDIMAILH_H
	printf("%s\n", sccsidh);
#endif
}
