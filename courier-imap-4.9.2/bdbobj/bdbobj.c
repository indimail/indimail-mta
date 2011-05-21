/*
** Copyright 1998 - 2003 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif

#include	<fcntl.h>
#include	<string.h>
#include	<stdlib.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

#include	"bdbobj.h"

void bdbobj_init(struct bdbobj *obj)
{
	obj->has_dbf=0;

#if	DB_VERSION_MAJOR >= 2
	obj->has_dbc=0;
#endif
}

void bdbobj_close(struct bdbobj *obj)
{
#if	DB_VERSION_MAJOR >= 2
	if (obj->has_dbc)
	{
		(*obj->dbc->c_close)(obj->dbc);
		obj->has_dbc=0;
	}
#endif
	if ( obj->has_dbf )
	{
#if	DB_VERSION_MAJOR < 2
		(*obj->dbf->close)(obj->dbf);
#else
		(*obj->dbf->close)(obj->dbf, 0);
#endif
		obj->has_dbf=0;
	}
}

int bdbobj_open(struct bdbobj *obj, const char *filename, const char *modestr)
{
#if	DB_VERSION_MAJOR < 2

int	flags=O_RDONLY;

#else

int	flags=DB_RDONLY;

#endif

DBTYPE	dbtype=DB_HASH;

	for ( ; *modestr; modestr++)
		switch (*modestr)	{
		case 'c':
		case 'C':
#if	DB_VERSION_MAJOR < 2
			flags=O_RDWR|O_CREAT;
#else
			flags=DB_CREATE;
#endif
			break;
		case 'w':
		case 'W':
#if	DB_VERSION_MAJOR < 2
			flags=O_RDWR;
#else
			flags=0;
#endif
			break;
		case 'n':
		case 'N':
#if	DB_VERSION_MAJOR < 2
			flags=O_RDWR|O_CREAT|O_TRUNC;
#else
			flags=DB_CREATE|DB_TRUNCATE;
#endif

			break;

		case 'b':
		case 'B':
			dbtype=DB_BTREE;
			break;

		case 'e':
		case 'E':
			dbtype=DB_RECNO;
			break;
		}

	bdbobj_close(obj);

#if DB_VERSION_MAJOR < 3
#if DB_VERSION_MAJOR < 2
	if ( (obj->dbf=dbopen(filename, flags, 0664, dbtype, 0)) != 0)
#else
	if ( db_open(filename, dbtype, flags, 0664, 0, 0, &obj->dbf) == 0)
#endif
#else
	obj->dbf=0;

#define DB_40 0

#if DB_VERSION_MAJOR == 4
#if DB_VERSION_MINOR == 0

#undef DB_40
#define DB_40 1

#endif
#endif

#if DB_VERSION_MAJOR == 3
#undef DB_40
#define DB_40 1
#endif

	if (db_create(&obj->dbf, NULL, 0) == 0)
	{
		if ( (*obj->dbf->open)(obj->dbf,

#if DB_40

#else
				       NULL,
#endif

				       filename, NULL,
				       dbtype, flags, 0664))
		{
			(*obj->dbf->close)(obj->dbf, DB_NOSYNC);
			obj->dbf=0;
		}
	}

	if (obj->dbf)
#endif
	{
#ifdef  FD_CLOEXEC

#if DB_VERSION_MAJOR < 2
        int     fd=(*obj->dbf->fd)(obj->dbf);
#else
	int	fd;

		if ((*obj->dbf->fd)(obj->dbf, &fd))
			fd= -1;
#endif

                if (fd >= 0)    fcntl(fd, F_SETFD, FD_CLOEXEC);
#endif


		obj->has_dbf=1;
		return (0);
	}
	return (-1);
}

int bdbobj_store(struct bdbobj *obj, const char *key, size_t keylen,
		const char *data,
		size_t	datalen,
		const	char *mode)
{
DBT dkey, dval;

	memset(&dkey, 0, sizeof(dkey));
	memset(&dval, 0, sizeof(dval));

	dkey.data=(void *)key;
	dkey.size=keylen;
	dval.data=(void *)data;
	dval.size=datalen;

#if	DB_VERSION_MAJOR < 2
	return (obj->has_dbf ? (*obj->dbf->put)(obj->dbf, &dkey, &dval, (
			*mode == 'i' || *mode == 'I' ?  R_NOOVERWRITE:0)):-1);
#else
	return (obj->has_dbf ? (*obj->dbf->put)(obj->dbf, 0, &dkey, &dval, (
			*mode == 'i' || *mode == 'I' ? DB_NOOVERWRITE:0)):-1);
#endif
}

static char *doquery(struct bdbobj *obj,
	const char *, size_t, size_t *, const char *);

char	*bdbobj_fetch(struct bdbobj *obj, const char *key, size_t keylen,
		size_t *datalen, const char *options)
{
char	*p=doquery(obj, key, keylen, datalen, options);
char	*q;

	if (!p)	return (0);

	q=(char *)malloc(*datalen);

	if (!q)	return (0);

	memcpy(q, p, *datalen);
	return (q);
}

char    *dofetch(struct bdbobj *, const char *, size_t, size_t *);

static char *doquery(struct bdbobj *obj, const char *key, size_t keylen,
	size_t *datalen, const char *options)
{
char	*p;

	for (;;)
	{
		if ((p=dofetch(obj, key, keylen, datalen)) != 0)
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
				if ((p=dofetch(obj, key, i, datalen)) != 0)
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

char	*dofetch(struct bdbobj *obj, const char *key, size_t keylen,
	size_t *datalen)
{
DBT	dkey, val;

	if (!obj->has_dbf)	return (0);

	memset(&dkey, 0, sizeof(dkey));
	memset(&val, 0, sizeof(val));

	dkey.data=(void *)key;
	dkey.size=keylen;

#if	DB_VERSION_MAJOR < 2
	if ( (*obj->dbf->get)(obj->dbf, &dkey, &val, 0))	return (0);
#else
	if ( (*obj->dbf->get)(obj->dbf, 0, &dkey, &val, 0)) return (0);
#endif

	*datalen=val.size;
	return ((char *)val.data);
}

int	bdbobj_exists(struct bdbobj *obj, const char *key, size_t keylen)
{
size_t	datalen;
char	*p=doquery(obj, key, keylen, &datalen, 0);

	return (p ? 1:0);
}
