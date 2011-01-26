/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"config.h"
#include	<string.h>
#include	<stdlib.h>
#include	"bdbobj.h"

char	*bdbobj_firstkey(struct bdbobj *obj, size_t *keylen, char **val,
	size_t *vallen)
{
DBT	key, value;

	if (!obj->has_dbf)	return (0);

	memset(&key, 0, sizeof(key));
	memset(&value, 0, sizeof(value));

#if	DB_VERSION_MAJOR < 2
	if ((*obj->dbf->seq)(obj->dbf, &key, &value, R_FIRST))	return (0);
#else
	if (obj->has_dbc)
	{
		(*obj->dbc->c_close)(obj->dbc);
		obj->has_dbc=0;
	}

#if DB_VERSION_MAJOR > 2
	if ((*obj->dbf->cursor)(obj->dbf, 0, &obj->dbc, 0))	return (0);
#else
#if DB_VERSION_MINOR >= 5
	if ((*obj->dbf->cursor)(obj->dbf, 0, &obj->dbc, 0))	return (0);
#else
	if ((*obj->dbf->cursor)(obj->dbf, 0, &obj->dbc))	return (0);
#endif
#endif
	obj->has_dbc=1;

	if ((*obj->dbc->c_get)(obj->dbc, &key, &value, DB_FIRST)) return (0);
#endif
	*keylen=key.size;
	*vallen=value.size;
	if ((*val=(char *)malloc(*vallen)) == 0)	return (0);

	memcpy(*val, value.data, *vallen);
	return ((char *)key.data);
}

char	*bdbobj_nextkey(struct bdbobj *obj, size_t *keylen, char **val,
	size_t *vallen)
{
DBT	key, value;

	if (!obj->has_dbf)	return (0);

	memset(&key, 0, sizeof(key));
	memset(&value, 0, sizeof(value));

#if	DB_VERSION_MAJOR < 2
	if ((*obj->dbf->seq)(obj->dbf, &key, &value, R_NEXT))	return (0);
#else
	if (!obj->has_dbc)	return (0);

	if ((*obj->dbc->c_get)(obj->dbc, &key, &value, DB_NEXT))
	{
		(*obj->dbc->c_close)(obj->dbc);
		obj->has_dbc=0;
	}
#endif

	*keylen=key.size;
	*vallen=value.size;
	if ((*val=(char *)malloc(*vallen + 1)) == 0)	return (0);

	memcpy(*val, value.data, *vallen);
	(*val)[*vallen]=0;

	return ((char *)key.data);
}
