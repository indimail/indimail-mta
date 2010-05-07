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
 * $Id: spwd.c,v 1.2 2004/11/24 21:25:48 cinergi Exp $ 
 */
#include "common.h"
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#include <stdlib.h>
#include <string.h>
#include "nsvs.h"

static ent_t    spent;

static          NSS_STATUS
_load_shadow(struct response_data *data, struct spwd *result, char *buffer, size_t buflen, int *errnop)
{
	if (buflen < data->header.record_size - RDS)
		EXHAUSTED_BUFFER;

	memcpy(buffer, data->strdata, data->header.record_size);

	result->sp_namp = buffer + data->header.offsets[ROW_SP_NAMP];
	result->sp_pwdp = buffer + data->header.offsets[ROW_SP_PWDP];
	result->sp_lstchg = atol(buffer + data->header.offsets[ROW_SP_LSTCHG]);
	result->sp_min = atol(buffer + data->header.offsets[ROW_SP_MIN]);
	result->sp_max = atol(buffer + data->header.offsets[ROW_SP_MAX]);
	result->sp_warn = atol(buffer + data->header.offsets[ROW_SP_WARN]);
	result->sp_inact = atol(buffer + data->header.offsets[ROW_SP_INACT]);
	result->sp_expire = atol(buffer + data->header.offsets[ROW_SP_EXPIRE]);
	result->sp_flag = (unsigned long) atol(buffer + data->header.offsets[ROW_SP_FLAG]);

	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_getspnam_r(nss_backend_t * be, void *args)
#else
_nss_nssd_getspnam_r(const char *key, struct spwd * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	response_header_t response_header;
	struct response_data *data;
	int             status;
#if defined (sun)
	const char     *key = NSS_ARGS(args)->key.name;
	struct spwd    *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	status = _get_response_data(GETSPBYNAME, key, &response_header, &data, SINGLE_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	memset(buf_out, 0, buflen);
	status = _load_shadow(data, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif

	return (status);
}

NSS_STATUS
#if defined (sun)
_nss_nssd_endspent(nss_backend_t * be, void *args)
#else
_nss_nssd_endspent(void)
#endif
{
	XFREE(spent.data);
	memset(&spent, 0, sizeof (spent));
	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_setspent(nss_backend_t * be, void *args)
#else
_nss_nssd_setspent(void)
#endif
{
	int             status;

	XFREE(spent.data);
	memset(&spent, 0, sizeof (spent));

	status = _get_response_data(GETSP, "", &spent.response_header, &spent.data, MULTI_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	spent.dp = spent.data;

	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nssd_getspent_r(nss_backend_t * be, void *args)
#else
_nss_nssd_getspent_r(struct spwd * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	int             status;
#if defined (sun)
	struct spwd    *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	if (++spent.cur_rec > spent.response_header.count)
		return NSS_NOTFOUND;

	memset(buf_out, 0, buflen);
	status = _load_shadow(spent.dp, result, buf_out, buflen, errnop);
	spent.dp = (struct response_data *) &(((char *) spent.dp)[spent.dp->header.record_size]);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif
	return status;
}

#if defined (sun)
static nss_backend_op_t shadow_ops[] = {
	_nss_nssd_default_destr,	/* NSS_DBOP_DESTRUCTOR */
	_nss_nssd_endspent,			/* NSS_DBOP_ENDENT */
	_nss_nssd_setspent,			/* NSS_DBOP_SETENT */
	_nss_nssd_getspent_r,		/* NSS_DBOP_GETENT */
	_nss_nssd_getspnam_r		/* NSS_DBOP_SHADOW_BYNAME */
};

nss_backend_t  *
_nss_nssd_shadow_constr(const char *db_name, const char *src_name, const char *cfg_args)
{
	nss_backend_t  *be;
	be = (nss_backend_t *) malloc(sizeof (*be));
	if (!be)
		return NULL;
	be->ops = shadow_ops;
	be->n_ops = sizeof (shadow_ops) / sizeof (nss_backend_op_t);
	return be;
}
#endif

#endif	/* HAVE_SHADOW_H */
