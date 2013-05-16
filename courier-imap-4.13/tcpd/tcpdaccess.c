/*
** Copyright 1998 - 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/

#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<stdlib.h>
#include	<ctype.h>
#include	"dbobj.h"


static struct dbobj db;
static int db_isopen=0, db_isinit=0;

int openaccess(const char *filename)
{
	if (!db_isinit)
	{
		dbobj_init(&db);
		db_isinit=1;
	}
	if (db_isopen)
	{
		dbobj_close(&db);
		db_isopen=0;
	}

	if (dbobj_open(&db, filename, "R"))
		return (-1);
	db_isopen=1;
	return (0);
}

void closeaccess()
{
	if (!db_isopen)	return;
	dbobj_close(&db);
	db_isopen=0;
}

char *chkaccess(const char *ip)
{
size_t	l;
char	*p, *q;

	if (!db_isopen)	return (0);


	p=dbobj_fetch(&db, ip, strlen(ip), &l, "");

	if (!p)	return (0);
	q=(char *)malloc(l+1);
	if (!q)
	{
		perror("malloc");
		free(p);
		return (0);
	}
	memcpy(q, p, l);
	q[l]=0;
	free(p);
	return (q);
}
