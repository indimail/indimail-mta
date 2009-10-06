/*
 * Copyright (C) 2004 Ben Goodwin
 * * This file is part of the nsvs package
 * *
 * * The nsvs package is free software; you can redistribute it and/or
 * * modify it under the terms of the GNU General Public License as published
 * * by the Free Software Foundation; either version 2 of the License, or
 * * (at your option) any later version.
 * *
 * * The nsvs package is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * * GNU General Public License for more details.
 * * You should have received a copy of the GNU General Public License
 * * along with the nsvs package; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * $Id: pwd.c,v 1.2 2004/11/24 21:25:47 cinergi Exp $ 
 */
#include "common.h"
#include <pwd.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include "nsvs.h"

static ent_t    pwent;

static          NSS_STATUS
_load_passwd(struct response_data *data, struct passwd *result, char *buffer, size_t buflen, int *errnop)
{
	if (buflen < data->header.record_size - RDS)
		EXHAUSTED_BUFFER;

	memcpy(buffer, data->strdata, data->header.record_size);

	result->pw_name = buffer + data->header.offsets[ROW_PW_NAME];
	result->pw_passwd = buffer + data->header.offsets[ROW_PW_PASSWD];
	result->pw_uid = atoi(buffer + data->header.offsets[ROW_PW_UID]);
	result->pw_gid = atoi(buffer + data->header.offsets[ROW_PW_GID]);
	result->pw_gecos = buffer + data->header.offsets[ROW_PW_GECOS];
	result->pw_dir = buffer + data->header.offsets[ROW_PW_DIR];
	result->pw_shell = buffer + data->header.offsets[ROW_PW_SHELL];
#if defined(__FreeBSD__)
	result->pw_change = atoi(buffer + data->header.offsets[ROW_PW_CHANGE]);
	result->pw_expire = atoi(buffer + data->header.offsets[ROW_PW_EXPIRE]);
	result->pw_class = buffer + data->header.offsets[ROW_PW_CLASS];
#endif
#if defined (sun)
	result->pw_age = buffer + data->header.offsets[ROW_PW_AGE];
	result->pw_comment = buffer + data->header.offsets[ROW_PW_COMMENT];
#endif

	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_getpwnam_r(nss_backend_t * be, void *args)
#else
_nss_nssd_getpwnam_r(const char *key, struct passwd * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	response_header_t response_header;
	struct response_data *data;
	int             status;
#if defined (sun)
	const char     *key = NSS_ARGS(args)->key.name;
	struct passwd  *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	status = _get_response_data(GETPWBYNAME, key, &response_header, &data, SINGLE_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	memset(buf_out, 0, buflen);
	status = _load_passwd(data, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif

	return (status);
}

NSS_STATUS
#if defined (sun)
_nss_nssd_getpwuid_r(nss_backend_t * be, void *args)
#else
_nss_nssd_getpwuid_r(uid_t uid, struct passwd * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	response_header_t response_header;
	struct response_data *data;
	int             status;
	char            key[12];
#if defined (sun)
	uid_t           uid = NSS_ARGS(args)->key.uid;
	struct passwd  *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	snprintf(key, sizeof (key), "%u", uid);
	status = _get_response_data(GETPWBYUID, key, &response_header, &data, SINGLE_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	memset(buf_out, 0, buflen);
	status = _load_passwd(data, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif

	return (status);
}

NSS_STATUS
#if defined (sun)
_nss_nssd_endpwent(nss_backend_t * be, void *args)
#else
_nss_nssd_endpwent(void)
#endif
{
	XFREE(pwent.data);
	memset(&pwent, 0, sizeof (pwent));
	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_setpwent(nss_backend_t * be, void *args)
#else
_nss_nssd_setpwent(void)
#endif
{
	int             status;

	XFREE(pwent.data);
	memset(&pwent, 0, sizeof (pwent));

	status = _get_response_data(GETPW, "", &pwent.response_header, &pwent.data, MULTI_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	pwent.dp = pwent.data;

	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_getpwent_r(nss_backend_t * be, void *args)
#else
_nss_nssd_getpwent_r(struct passwd * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	int             status;
#if defined (sun)
	struct passwd  *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	if (++pwent.cur_rec > pwent.response_header.count)
		return NSS_NOTFOUND;

	memset(buf_out, 0, buflen);
	status = _load_passwd(pwent.dp, result, buf_out, buflen, errnop);
	pwent.dp = (struct response_data *) &(((char *) pwent.dp)[pwent.dp->header.record_size]);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif
	return status;
}

#if defined (sun)
static nss_backend_op_t passwd_ops[] = {
	_nss_nssd_default_destr,	/* NSS_DBOP_DESTRUCTOR */
	_nss_nssd_endpwent,			/* NSS_DBOP_ENDENT */
	_nss_nssd_setpwent,			/* NSS_DBOP_SETENT */
	_nss_nssd_getpwent_r,		/* NSS_DBOP_GETENT */
	_nss_nssd_getpwnam_r,		/* NSS_DBOP_PASSWD_BYNAME */
	_nss_nssd_getpwuid_r,		/* NSS_DBOP_PASSWD_BYUID */
};

nss_backend_t  *
_nss_nssd_passwd_constr(const char *db_name, const char *src_name, const char *cfg_args)
{
	nss_backend_t  *be;
	be = (nss_backend_t *) malloc(sizeof (*be));
	if (!be)
		return NULL;
	be->ops = passwd_ops;
	be->n_ops = sizeof (passwd_ops) / sizeof (nss_backend_op_t);
	return be;
}
#endif
