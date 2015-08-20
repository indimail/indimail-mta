/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	<string.h>
#include	<stdlib.h>
#include	"bdbobj.h"

int	bdbobj_delete(struct bdbobj *obj, const char *key, size_t keylen)
{
DBT	dkey, val;

	if (!obj->has_dbf)	return (0);

	memset(&dkey, 0, sizeof(dkey));
	memset(&val, 0, sizeof(val));
	dkey.data=(void *)key;
	dkey.size=keylen;

#if	DB_VERSION_MAJOR < 2
	if ( (*obj->dbf->del)(obj->dbf, &dkey, 0))	return (-1);
#else
	if ( (*obj->dbf->del)(obj->dbf, 0, &dkey, 0))	return (-1);
#endif
	return (0);
}
