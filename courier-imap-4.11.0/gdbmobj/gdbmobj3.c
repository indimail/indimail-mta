/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"gdbmobj.h"

int	gdbmobj_delete(struct gdbmobj *obj, const char *key, size_t keylen)
{
datum	dkey;

	if (!obj->has_dbf)	return (0);

	dkey.dptr=(char *)key;
	dkey.dsize=keylen;

	if (gdbm_delete(obj->dbf, dkey))	return (-1);
	return (0);
}
