/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include	"gdbmobj.h"

static char *fetchkeyval(struct gdbmobj *obj, size_t *, char **, size_t *);
extern char *gdbm_dofetch(struct gdbmobj *, const char *, size_t, size_t *);

char	*gdbmobj_firstkey(struct gdbmobj *obj, size_t *keylen, char **val,
	size_t *vallen)
{
datum	key;

	if (!obj->has_dbf)	return (0);

	if (obj->prev_key)	free(obj->prev_key);
	obj->prev_key=0;

	key=gdbm_firstkey(obj->dbf);

	if (!key.dptr)	return (0);

	obj->prev_key=key.dptr;
	obj->prev_key_len=key.dsize;
	return (fetchkeyval(obj, keylen, val, vallen));
}


static char *fetchkeyval(struct gdbmobj *obj, size_t *keylen, char **val,
	size_t *vallen)
{
	if (!obj->prev_key)	return (0);
	*val=gdbm_dofetch(obj, obj->prev_key, obj->prev_key_len, vallen);
	*keylen=obj->prev_key_len;
	return (obj->prev_key);
}

char	*gdbmobj_nextkey(struct gdbmobj *obj, size_t *keylen,
	char **val, size_t *vallen)
{
datum	dkey, key;

	if (!obj->has_dbf || !obj->prev_key)	return (0);

	dkey.dptr=(char *)obj->prev_key;
	dkey.dsize=obj->prev_key_len;

	key=gdbm_nextkey(obj->dbf, dkey);

	free(obj->prev_key);
	obj->prev_key=key.dptr;
	obj->prev_key_len=key.dsize;
	return (fetchkeyval(obj, keylen, val, vallen));
}
