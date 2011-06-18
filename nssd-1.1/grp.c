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
 * $Id: grp.c,v 1.1 2011-06-18 11:38:29+05:30 Cprogrammer Exp mbhangui $ 
 */
#include "common.h"
#include <grp.h>
#include <string.h>
#include <stdlib.h>
#include "nsvs.h"

typedef struct {
	gid_t         **groupsp;
	long int        group;
	long int       *start;
	long int       *size;
	long int        limit;
} group_info_t;

static ent_t    grent;

static          NSS_STATUS
_load_group(struct response_data *data, struct group *result, char *buffer, size_t buflen, int *errnop)
{
	if (buflen < data->header.record_size - RDS)
		EXHAUSTED_BUFFER;

	memcpy(buffer, data->strdata, data->header.record_size);

	result->gr_name = buffer + data->header.offsets[ROW_GR_NAME];
	result->gr_passwd = buffer + data->header.offsets[ROW_GR_PASSWD];
	result->gr_gid = atoi(buffer + data->header.offsets[ROW_GR_GID]);

	return NSS_SUCCESS;
}

static          NSS_STATUS
_load_group_members(struct response_data *data, response_header_t response_header, struct group *result, char *buffer,
					size_t buflen, int *errnop)
{
	char          **members;
	size_t          strings_offset;
	char           *cp;
	struct response_data *dp;
	int             i;

	align(buffer, buflen, char *);

	members = (char **) buffer;
	cp = data->strdata;
	dp = data;

	/*- Leave room for pointer list at the front */
	strings_offset = (response_header.count + 1) * sizeof (char *);
	if (buflen < strings_offset)
		EXHAUSTED_BUFFER;
	buflen -= strings_offset;
	/*- set the first pointer */
	members[0] = buffer + strings_offset;
	/*- Loop through each record ...  */
	for (i = 0; i < response_header.count; i++) {
		if (buflen < dp->header.record_size - RDS)
			EXHAUSTED_BUFFER;
		buflen -= dp->header.record_size - RDS;
		/*- copy in the data (username) */
		strncpy(members[i], cp, dp->header.record_size - RDS);
		/*- set the next pointer */
		members[i + 1] = members[i] + dp->header.record_size - RDS;
		/*- load the next record */
		cp += dp->header.record_size;
		dp = (struct response_data *) &(((char *) dp)[dp->header.record_size]);
	}

	/*- Null-terminate the pointer list */
	members[response_header.count] = NULL;
	/*- set gr_mem to our list of pointers */
	result->gr_mem = (char **) (uintptr_t) buffer;
	return NSS_SUCCESS;
}

static          NSS_STATUS
_load_gidsbymem(struct response_data *data, response_header_t response_header, group_info_t * gi, int *errnop)
{
	gid_t          *groups;
	int             i;
	gid_t           gid;
	struct response_data *dp;
	char           *cp;
	int             count;

	/*- Nothing to load = success */
	if (response_header.count == 0)
		return NSS_SUCCESS;

	/*- If we need more room and we're allowed to alloc it, alloc it */
	if (response_header.count + *gi->start > *gi->size) {
		long int        newsize = *gi->size;

		if (gi->limit <= 0)		/* Allocate as much as we need */
			newsize = response_header.count + *gi->start;
		else if (*gi->size != gi->limit)	/* Allocate to limit */
			newsize = gi->limit;

		if (newsize != *gi->size) {	/* If we've got more room, do it */
			gid_t          *groups = *gi->groupsp;
			gid_t          *newgroups;

			newgroups = (gid_t *) realloc(groups, newsize * sizeof (*groups));
			if (newgroups != NULL) {
				*gi->groupsp = groups = newgroups;
				*gi->size = newsize;
			}
		}
	}
	groups = *gi->groupsp;
	dp = data;
	cp = data->strdata;
	for (i = *gi->start, count = 0; i < *gi->size; i++, count++) {
		if (count >= response_header.count)
			break;
		gid = atoi(cp);
		if ((long int) gid != gi->group && (long int) gid != groups[0])
			groups[(*gi->start)++] = gid;
		cp += dp->header.record_size;
		dp = (struct response_data *) &(((char *) dp)[dp->header.record_size]);
	}
	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_getgrnam_r(nss_backend_t * be, void *args)
#else
_nss_nsvs_getgrnam_r(const char *key, struct group * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	response_header_t response_header;
	struct response_data *data;
	int             status;
	char            gid[12];
#if defined (sun)
	const char     *key = NSS_ARGS(args)->key.name;
	struct group   *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	status = _get_response_data(GETGRBYNAME, key, &response_header, &data, SINGLE_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	if (buflen < grent.response_header.response_size)
		EXHAUSTED_BUFFER;

	memset(buf_out, 0, buflen);
	status = _load_group(data, result, buf_out, buflen, errnop);
	XFREE(data);
	if (status != NSS_SUCCESS)
		return status;
	/*- Skip ahead to where we'll put member list */
	buf_out += response_header.response_size;
	buflen -= response_header.response_size;

	/*- Get members using GID */
	snprintf(gid, 12, "%u", result->gr_gid);
	status = _get_response_data(GETGRMEMSBYGID, gid, &response_header, &data, MULTI_READ_TIMEOUT);
	if (status == NSS_NOTFOUND) {
		result->gr_mem = (char **) (uintptr_t) buf_out;
		return NSS_SUCCESS;
	}
	if (status != NSS_SUCCESS)
		return status;

	status = _load_group_members(data, response_header, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif
	return (status);
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_getgrgid_r(nss_backend_t * be, void *args)
#else
_nss_nsvs_getgrgid_r(gid_t gid, struct group * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	response_header_t response_header;
	struct response_data *data;
	int             status;
	char            key[12];
#if defined (sun)
	gid_t           gid = NSS_ARGS(args)->key.gid;
	struct group   *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif

	snprintf(key, sizeof (key), "%u", gid);
	status = _get_response_data(GETGRBYGID, key, &response_header, &data, SINGLE_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	if (buflen < grent.response_header.response_size)
		EXHAUSTED_BUFFER;

	memset(buf_out, 0, buflen);
	status = _load_group(data, result, buf_out, buflen, errnop);
	XFREE(data);
	if (status != NSS_SUCCESS)
		return status;

	/*- Skip ahead to where we'll put member list */
	buf_out += response_header.response_size;
	buflen -= response_header.response_size;

	/*- Get members using GID */
	status = _get_response_data(GETGRMEMSBYGID, key, &response_header, &data, MULTI_READ_TIMEOUT);
	if (status == NSS_NOTFOUND) {
		result->gr_mem = (char **) (uintptr_t) buf_out;
		return NSS_SUCCESS;
	}
	if (status != NSS_SUCCESS)
		return status;

	status = _load_group_members(data, response_header, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif
	return (status);
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_endgrent(nss_backend_t * be, void *args)
#else
_nss_nsvs_endgrent(void)
#endif
{
	XFREE(grent.data);
	memset(&grent, 0, sizeof (grent));
	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_setgrent(nss_backend_t * be, void *args)
#else
_nss_nsvs_setgrent(void)
#endif
{
	int             status;

	XFREE(grent.data);
	memset(&grent, 0, sizeof (grent));

	status = _get_response_data(GETGR, "", &grent.response_header, &grent.data, MULTI_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;

	grent.dp = grent.data;

	return NSS_SUCCESS;
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_getgrent_r(nss_backend_t * be, void *args)
#else
_nss_nsvs_getgrent_r(struct group * result, char *buf_out, size_t buflen, int *errnop)
#endif
{
	int             status;
#if defined (sun)
	struct group   *result = NSS_ARGS(args)->buf.result;
	char           *buf_out = NSS_ARGS(args)->buf.buffer;
	size_t          buflen = NSS_ARGS(args)->buf.buflen;
	int            *errnop = &NSS_ARGS(args)->erange;
#endif
	/*- All these locals are needed for fetching member list */
	response_header_t response_header;
	struct response_data *data;
	char            key[12];
	/*- end locals */

	if (++grent.cur_rec > grent.response_header.count)
		return NSS_NOTFOUND;

	if (buflen < grent.response_header.response_size)
		EXHAUSTED_BUFFER;
	/*- Load main group data */
	memset(buf_out, 0, buflen);
	status = _load_group(grent.dp, result, buf_out, buflen, errnop);
	if (status != NSS_SUCCESS)
		return status;

	/*- Move to next record NOW, as we may not have a member list below */
	grent.dp = (struct response_data *) &(((char *) grent.dp)[grent.dp->header.record_size]);

	/*- Skip ahead to where we'll put member list */
	buf_out += grent.response_header.response_size;
	buflen -= grent.response_header.response_size;

	/*- Get member list */
	snprintf(key, 12, "%u", result->gr_gid);
	status = _get_response_data(GETGRMEMSBYGID, key, &response_header, &data, MULTI_READ_TIMEOUT);
	if (status == NSS_NOTFOUND) {
		result->gr_mem = (char **) (uintptr_t) buf_out;
		return NSS_SUCCESS;
	}
	if (status != NSS_SUCCESS)
		return status;

	/*- Place member list into NSS structure */
	status = _load_group_members(data, response_header, result, buf_out, buflen, errnop);
	XFREE(data);
#if defined (sun)
	if (status == NSS_SUCCESS)
		NSS_ARGS(args)->returnval = NSS_ARGS(args)->buf.result;
#endif
	return status;
}

NSS_STATUS
#if defined (sun)
_nss_nsvs_getgrmem(nss_backend_t * be, void *args)
#else
_nss_nsvs_initgroups_dyn(const char *key, gid_t group, long int *start, long int *size, gid_t ** groupsp, long int limit,
						 int *errnop)
#endif
{
	int             status;
	response_header_t response_header;
	struct response_data *data;
	group_info_t    gi;

#if defined (sun)
	gi.start = (long int *) &((struct nss_groupsbymem *) args)->numgids;
	gi.limit = ((struct nss_groupsbymem *) args)->maxgids;
	gi.size = &gi.limit;
	gi.groupsp = &(((struct nss_groupsbymem *) args)->gid_array);
	gi.group = -1;
	const char     *key = ((struct nss_groupsbymem *) args)->username;
	int            *errnop = NULL;
#else
	gi.start = start;
	gi.size = size;
	gi.limit = limit;
	gi.groupsp = groupsp;
	gi.group = (long int) group;
#endif

	status = _get_response_data(GETGRGIDSBYMEM, key, &response_header, &data, MULTI_READ_TIMEOUT);
	if (status != NSS_SUCCESS)
		return status;
	status = _load_gidsbymem(data, response_header, &gi, errnop);
	XFREE(data);
	if (status != NSS_SUCCESS)
		return status;

#if defined (sun)
	return NSS_NOTFOUND;
#else
	return NSS_SUCCESS;
#endif
}

#if defined (sun)
static nss_backend_op_t group_ops[] = {
	_nss_nsvs_default_destr,	/* NSS_DBOP_DESTRUCTOR */
	_nss_nsvs_endgrent,			/* NSS_DBOP_ENDENT */
	_nss_nsvs_setgrent,			/* NSS_DBOP_SETENT */
	_nss_nsvs_getgrent_r,		/* NSS_DBOP_GETENT */
	_nss_nsvs_getgrnam_r,		/* NSS_DBOP_GROUP_BYNAME */
	_nss_nsvs_getgrgid_r,		/* NSS_DBOP_GROUP_BYGID */
	_nss_nsvs_getgrmem			/* NSS_DBOP_GROUP_BYMEMBER */
};

nss_backend_t  *
_nss_nsvs_group_constr(const char *db_name, const char *src_name, const char *cfg_args)
{
	nss_backend_t  *be;
	be = (nss_backend_t *) malloc(sizeof (*be));
	if (!be)
		return NULL;
	be->ops = group_ops;
	be->n_ops = sizeof (group_ops) / sizeof (nss_backend_op_t);
	return be;
}
#endif
