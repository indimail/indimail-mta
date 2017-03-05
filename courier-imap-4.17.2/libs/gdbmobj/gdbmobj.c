/*
** Copyright 1998 - 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	"gdbmobj.h"
#include	<stdlib.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif


void gdbmobj_init(struct gdbmobj *obj)
{
	obj->has_dbf=0;
	obj->prev_key=0;
	obj->prev_key_len=0;
}

void gdbmobj_close(struct gdbmobj *obj)
{
	if (obj->has_dbf)
	{
		obj->has_dbf=0;
		gdbm_close(obj->dbf);
	}
	if (obj->prev_key)
	{
		free(obj->prev_key);
		obj->prev_key=0;
	}
}

int gdbmobj_open(struct gdbmobj *obj, const char *filename, const char *modestr)
{
int	mode=GDBM_READER;

	for ( ; *modestr; modestr++)
		switch (*modestr)	{
		case 'c':
		case 'C':
			mode=GDBM_WRCREAT;
			break;
		case 'w':
		case 'W':
			mode=GDBM_WRITER;
			break;
		case 'n':
		case 'N':
			mode=GDBM_NEWDB;
			break;
	}

	gdbmobj_close(obj);
	if ((obj->dbf=gdbm_open((char *)filename, 0, mode, 0664, 0)) != 0)
	{
		/* Where possible, set the close-on-exec bit */

#if	HAVE_GDBM_FDESC
#ifdef  FD_CLOEXEC

	int	fd=gdbm_fdesc(obj->dbf);

		if (fd >= 0)	fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif
#endif

		obj->has_dbf=1;
		return (0);
	}
	return (-1);
}

int gdbmobj_store(struct gdbmobj *obj, const char *key, size_t keylen,
		const char *data,
		size_t	datalen,
		const	char *mode)
{
datum dkey;
datum dval;

	dkey.dptr=(char *)key;
	dkey.dsize=keylen;

	dval.dptr=(char *)data;
	dval.dsize=datalen;

	return (obj->has_dbf ? gdbm_store(obj->dbf, dkey, dval, (
			*mode == 'i' || *mode == 'I' ?
				GDBM_INSERT:GDBM_REPLACE)):-1);
}

int	gdbmobj_exists(struct gdbmobj *obj, const char *key, size_t keylen)
{
datum	dkey;

	if (!obj->has_dbf)	return (0);

	dkey.dptr=(char *)key;
	dkey.dsize=keylen;

	if (gdbm_exists(obj->dbf, dkey))	return (1);
	return (0);
}

char *gdbm_dofetch(struct gdbmobj *, const char *, size_t, size_t *);

char	*gdbmobj_fetch(struct gdbmobj *obj, const char *key, size_t keylen,
		size_t *datalen, const char *options)
{
char	*p;

	for (;;)
	{
		if ((p=gdbm_dofetch(obj, key, keylen, datalen)) != 0)
			return (p);
		if (!options)	break;
		if (*options == 'I')
		{
			while (keylen && key[--keylen] != '.')
				;
			if (!keylen)	break;
			continue;
		}
		if (*options == 'D')
		{
		size_t	i;

			for (i=0; i<keylen; i++)
				if (key[i] == '@') { ++i; break; }
			if (i < keylen)
			{
				if ((p=gdbm_dofetch(obj, key, i, datalen)) != 0)
					return (p);
				key += i;
				keylen -= i;
				continue;
			}

			for (i=0; i<keylen; i++)
				if (key[i] == '.') { ++i; break; }
			if (i < keylen)
			{
				key += i;
				keylen -= i;
				continue;
			}
			break;
		}
		break;
	}
	return (0);
}

char *gdbm_dofetch(struct gdbmobj *obj,
	const char *key, size_t keylen, size_t *datalen)
{
datum	dkey, val;

	if (!obj->has_dbf)	return (0);

	dkey.dptr=(char *)key;
	dkey.dsize=keylen;

	val=gdbm_fetch(obj->dbf, dkey);

	if (!val.dptr)	return (0);
	*datalen=val.dsize;
	return (val.dptr);
}
