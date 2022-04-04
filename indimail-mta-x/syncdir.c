/*
 * $Log: syncdir.c,v $
 * Revision 1.11  2020-09-30 20:39:50+05:30  Cprogrammer
 * Darwin Port
 *
 * Revision 1.10  2020-09-25 11:15:53+05:30  Cprogrammer
 * FreeBSD port
 *
 * Revision 1.9  2020-09-15 21:08:40+05:30  Cprogrammer
 * set default value of use_fsync, use_sycdir as -1
 *
 * Revision 1.8  2020-09-14 21:41:28+05:30  Cprogrammer
 * fixed linkat(), unlinkat(), renameat() calls
 *
 * Revision 1.7  2020-08-21 22:35:07+05:30  Cprogrammer
 * syncdir.c: fix for missing SYS_open, SYS_link, SYS_unlink, SYS_rename (use SYS_openat, SYS_linkat, SYS_renameat syscalls)
 *
 * Revision 1.6  2008-07-25 16:52:25+05:30  Cprogrammer
 * port for darwin
 *
 * Revision 1.5  2004-10-22 20:31:21+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-09-26 00:01:15+05:30  Cprogrammer
 * removed stdio.h
 *
 * Revision 1.3  2004-07-15 23:33:41+05:30  Cprogrammer
 * added unistd.h
 *
 * Revision 1.2  2003-12-31 20:03:50+05:30  Cprogrammer
 * added use_syncdir to turn on/off syncdir
 *
 * Revision 1.1  2003-12-09 23:38:29+05:30  Cprogrammer
 * Initial revision
 *
 *
 * syncdir -- emulate synchronous directories
 * Copyright (C) 1998 Bruce Guenter
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * You can reach me at bruce.guenter@qcc.sk.ca
 */

#ifdef USE_FSYNC
int             use_fsync = -1, use_fdatasync = -1, use_syncdir = -1;
#include <sys/types.h>
#include <sys/stat.h>
#define open XXX_open
#include <fcntl.h>
#undef open
#include <unistd.h>
#include <string.h>
#if defined(linux)
#include <syscall.h>
#elif defined(__FreeBSD__)
#include <sys/syscall.h>
#endif
#include <errno.h>
#include <unistd.h>
#include "env.h"

#ifdef DARWIN
#define SYS_OPEN(FILE,FLAG,MODE) open(FILE,FLAG,MODE)
#define SYS_CLOSE(FD)            close(FD)
#define SYS_LINK(OLD,NEW)        link(OLD,NEW)
#define SYS_UNLINK(PATH)         unlink(PATH)
#define SYS_RENAME(OLD,NEW)      rename(OLD,NEW)
#define SYS_FSYNC(FD)            fsync(FD)
#define SYS_FDATASYNC(FD)        fsync(FD)
int             open(char *, int, ...);
int             rename(char *, char *); 
#else
#if defined(SYS_openat) && defined(AT_FDCWD)
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_openat,AT_FDCWD,FILE,FLAG,MODE)
#else
#define SYS_OPEN(FILE,FLAG,MODE) syscall(SYS_open,FILE,FLAG,MODE)
#endif
#define SYS_CLOSE(FD) syscall(SYS_close, FD)
#if defined(SYS_linkat) && defined(AT_FDCWD)
#define SYS_LINK(OLD,NEW)        syscall(SYS_linkat,AT_FDCWD,OLD,AT_FDCWD,NEW,0)
#else
#define SYS_LINK(OLD,NEW)        syscall(SYS_link,OLD,NEW)
#endif
#if defined(SYS_unlinkat) && defined(AT_FDCWD)
#define SYS_UNLINK(PATH)         syscall(SYS_unlinkat,AT_FDCWD,PATH,0)
#else
#define SYS_UNLINK(PATH)         syscall(SYS_unlink,PATH)
#endif
#if defined(SYS_renameat) && defined(AT_FDCWD)
#define SYS_RENAME(OLD,NEW)      syscall(SYS_renameat,AT_FDCWD,OLD,AT_FDCWD,NEW,0)
#else
#define SYS_RENAME(OLD,NEW)      syscall(SYS_rename,OLD,NEW)
#endif
#define SYS_FSYNC(FD)            syscall(SYS_fsync, FD)
#define SYS_FDATASYNC(FD)        syscall(SYS_fsync, FD)
#endif

static int
fdirsync(const char *filename, unsigned length)
{
	char            dirname[length + 1];
	/*
	 * This could also be:
	 * char *dirname = alloca(length+1); 
	 */
	int             dirfd;
	int             retval;

	memcpy(dirname, filename, length);
	dirname[length] = 0;
	if ((dirfd = SYS_OPEN(dirname, O_RDONLY, 0)) == -1)
		return -1;
	retval = (SYS_FSYNC(dirfd) == -1 && errno == EIO) ? -1 : 0;
	SYS_CLOSE(dirfd);
	return retval;
}

static int
fdirsyncfn(const char *filename)
{
	const char     *slash = filename + strlen(filename) - 1;

	/*
	 * Skip over trailing slashes, which would be ignored by some
	 * operations 
	 */
	while (slash > filename && *slash == '/')
		--slash;

	/*
	 * Skip back to the next slash 
	 */
	while (slash > filename && *slash != '/')
		--slash;

	/*
	 * slash now either points to a '/' character, or no slash was found 
	 */
	if (*slash == '/')
		return fdirsync(filename, (slash == filename) ? 1 : slash - filename);
	else
		return fdirsync(".", 1);
}

int
#ifdef DARWIN
qopen(const char *file, int oflag, mode_t mode)
#else
open(const char *file, int oflag, mode_t mode)
#endif
{
	int             fd = SYS_OPEN(file, oflag, mode);

	if (use_syncdir == -1)
		use_syncdir = (env_get("USE_SYNCDIR") ? 1 : 0);
	if (use_syncdir == -1 || !use_syncdir)
		return (fd);
	if (fd == -1)
		return fd;
	if (oflag & O_CREAT && fdirsyncfn(file) == -1) {
		SYS_CLOSE(fd);
		return -1;
	}
	return fd;
}

int
#ifdef DARWIN
qlink(const char *oldpath, const char *newpath)
#else
link(const char *oldpath, const char *newpath)
#endif
{
	if (SYS_LINK(oldpath, newpath) == -1)
		return -1;
	if (use_syncdir == -1)
		use_syncdir = (env_get("USE_SYNCDIR") ? 1 : 0);
	if (use_syncdir == -1 || !use_syncdir)
		return (0);
	return fdirsyncfn(newpath);
}

int
#ifdef DARWIN
qunlink(const char *path)
#else
unlink(const char *path)
#endif
{
	if (SYS_UNLINK(path) == -1)
		return -1;
	if (use_syncdir == -1)
		use_syncdir = (env_get("USE_SYNCDIR") ? 1 : 0);
	if (use_syncdir == -1 || !use_syncdir)
		return (0);
	return (fdirsyncfn(path));
}

int
#ifdef DARWIN
qrename(const char *oldpath, const char *newpath)
#else
rename(const char *oldpath, const char *newpath)
#endif
{
	if (SYS_RENAME(oldpath, newpath) == -1)
		return -1;
	if (use_syncdir == -1)
		use_syncdir = (env_get("USE_SYNCDIR") ? 1 : 0);
	if (use_syncdir == -1 || !use_syncdir)
		return (0);
	if (fdirsyncfn(newpath) == -1)
		return -1;
	return (fdirsyncfn(oldpath));
}
#endif /*- #ifdef USE_FSYNC */

void
getversion_syncdir_c()
{
	static char    *x = "$Id: syncdir.c,v 1.11 2020-09-30 20:39:50+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
