/*
** Copyright 2002-2004 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/

#if    HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	"rfc2045.h"
#include	<errno.h>

static const char xdigit[]="0123456789ABCDEFabcdef";

#define DOENCODE(c) \
	(strchr("()'\"\\%:;=", (c)) || (c) <= ' ' || (c) >= 127)

static int docreate(const char *name,
		    char *attrvalue,
		    int (*cb_func)(const char *param,
				   const char *value,
				   void *void_arg),
		    void *cb_arg);

int rfc2231_attrCreate(const char *name, const char *value,
		       const char *charset,
		       const char *language,
		       int (*cb_func)(const char *param,
				      const char *value,
				      void *void_arg),
		       void *cb_arg)
{
	size_t l;
	const char *cp;
	char *p, *q;
	int rc;

	if (strlen(name)>60)
	{
		errno=EINVAL;
		return -1; /* You kidding me? */
	}

	for (l=0; value[l]; l++)
		if (DOENCODE(value[l]))
			break;

	if (value[l] == 0 && strlen(name)+strlen(value)<75) /* No need to encode */
	{
		char *p=malloc(strlen(value)+3);

		if (!p)
			return -1;

		strcat(strcat(strcpy(p, "\""), value), "\"");

		rc=(*cb_func)(name, p, cb_arg);
		free(p);
		return rc;
	}

	if (!charset) charset="";
	if (!language) language="";

	l=strlen(charset)+strlen(language)+strlen(value)+3;

	for (cp=value; *cp; cp++)
		if (DOENCODE(*cp))
			l += 2;

	p=malloc(l);
	if (!p)
		return -1;

	strcat(strcat(strcat(strcpy(p, charset), "'"),language), "'");
	q=p+strlen(p);
	for (cp=value; *cp; cp++)
	{
		if (DOENCODE(*cp))
		{
			*q++='%';
			*q++ = xdigit[ ((unsigned char)*cp / 16) & 15];
			*q++ = xdigit[ *cp & 15];
		}
		else
			*q++= *cp;
	}
	*q=0;

	rc=docreate(name, p, cb_func, cb_arg);
	free(p);
	return rc;
}

static int docreate(const char *name,
		    char *q,
		    int (*cb_func)(const char *param,
				   const char *value,
				   void *void_arg),
		    void *cb_arg)
{
	char c;
	char *r;
	int rc;
	size_t l;
	int n;

	r=malloc(strlen(name)+20);
	if (!r)
		return -1;

	rc=0;
	n=0;

	while (*q)
	{
		sprintf(r, "%s*%d*", name, n++);

		l=strlen(q);
		if (l > 70-strlen(r))
			l=70-strlen(r);

		if (q[l] == '%')
			l += 3;
		else if (l && q[l-1] == '%')
			l += 2;
		else if (l > 1 && q[l-2] == '%')
			l += 1;
			
		c=q[l];
		q[l]=0;

		rc=(*cb_func)(r, q, cb_arg);
		if (rc)
			break;
		q[l]=c;
		q += l;
	}
	free(r);
	return rc;
}
