/*-
 * Copyright (C) 2004 Ben Goodwin
 * This file is part of the nsvs package
 *
 * The nsvs package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The nsvs package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with the nsvs package; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * $Id: misc.c,v 1.4 2004/12/15 00:06:03 cinergi Exp $ 
 */
#include "common.h"
#include <stdarg.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include "nsvs.h"

#define MAX_MSG_SIZE 1024
void
nsvs_log(int prio, const char *fmt, ...)
{
	va_list         ap;
	char            msg[MAX_MSG_SIZE];

	/*- higher prio = more debugging output */
	if (prio > LOG_DEBUG)
		return;
	va_start(ap, fmt);
	vsnprintf(msg, sizeof (msg), fmt, ap);
	syslog(prio, "(%s) %d: %s", PACKAGE, getpid(), msg);
	va_end(ap);
}

#if defined (sun)
NSS_STATUS
_nss_nssd_default_destr(nss_backend_t * be, void *args)
{
	if (be) {
		free(be);
		be = NULL;
	}
	return NSS_SUCCESS;

}
#endif

/*
 * Thanks to Clement Laforet for most of this 
 */
#if defined(__FreeBSD__)
NSS_METHOD_PROTOTYPE(__nss_compat_getpwnam_r);
NSS_METHOD_PROTOTYPE(__nss_compat_getpwuid_r);
NSS_METHOD_PROTOTYPE(__nss_compat_getpwent_r);
NSS_METHOD_PROTOTYPE(__nss_compat_setpwent);
NSS_METHOD_PROTOTYPE(__nss_compat_endpwent);
NSS_METHOD_PROTOTYPE(__nss_compat_getgrnam_r);
NSS_METHOD_PROTOTYPE(__nss_compat_getgrgid_r);
NSS_METHOD_PROTOTYPE(__nss_compat_getgrent_r);
NSS_METHOD_PROTOTYPE(__nss_compat_setgrent);
NSS_METHOD_PROTOTYPE(__nss_compat_endgrent);

NSS_STATUS      _nss_nssd_getpwnam_r(const char *, struct passwd *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_getpwuid_r(uid_t, struct passwd *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_getpwent_r(struct passwd *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_setpwent(void);
NSS_STATUS      _nss_nssd_endpwent(void);

NSS_STATUS      _nss_nssd_getgrnam_r(const char *, struct group *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_getgrgid_r(gid_t, struct group *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_getgrent_r(struct group *, char *, size_t, int *);
NSS_STATUS      _nss_nssd_setgrent(void);
NSS_STATUS      _nss_nssd_endgrent(void);

static ns_mtab  methods[] = {
	{NSDB_PASSWD, "getpwnam_r", __nss_compat_getpwnam_r, _nss_nssd_getpwnam_r},
	{NSDB_PASSWD, "getpwuid_r", __nss_compat_getpwuid_r, _nss_nssd_getpwuid_r},
	{NSDB_PASSWD, "getpwent_r", __nss_compat_getpwent_r, _nss_nssd_getpwent_r},
	{NSDB_PASSWD, "setpwent", __nss_compat_setpwent, _nss_nssd_setpwent},
	{NSDB_PASSWD, "endpwent", __nss_compat_endpwent, _nss_nssd_endpwent},
	{NSDB_GROUP, "getgrnam_r", __nss_compat_getgrnam_r, _nss_nssd_getgrnam_r},
	{NSDB_GROUP, "getgrgid_r", __nss_compat_getgrgid_r, _nss_nssd_getgrgid_r},
	{NSDB_GROUP, "getgrent_r", __nss_compat_getgrent_r, _nss_nssd_getgrent_r},
	{NSDB_GROUP, "setgrent", __nss_compat_setgrent, _nss_nssd_setgrent},
	{NSDB_GROUP, "endgrent", __nss_compat_endgrent, _nss_nssd_endgrent},
};

ns_mtab        *
nss_module_register(const char *name, unsigned int *size, nss_module_unregister_fn * unregister)
{
	*size = sizeof (methods) / sizeof (methods[0]);
	*unregister = NULL;
	return (methods);
}
#endif /* defined(__FreeBSD__) */
