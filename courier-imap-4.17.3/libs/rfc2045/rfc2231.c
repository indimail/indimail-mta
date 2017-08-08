/*
** Copyright 2002-2011 Double Precision, Inc.  See COPYING for
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
#include	"rfc822/rfc822.h"
#include	"unicode/courier-unicode.h"

/*
** Deallocate a link list of rfc2231param structures.
*/

void rfc2231_paramDestroy(struct rfc2231param *p)
{
	while (p)
	{
		struct rfc2231param *q=p->next;

		free(p);
		p=q;
	}
}

int rfc2231_buildAttrList(struct rfc2231param **paramList,
			  const char *name,

			  const char *attrName,
			  const char *attrValue)
{
	int nameLen=strlen(name);

	if (strncmp(attrName, name, nameLen) == 0 &&
	    (attrName[nameLen] == 0 ||
	     attrName[nameLen] == '*'))
	{
		struct rfc2231param *n
			=malloc(sizeof(struct rfc2231param)), **o;

		const char *p=attrName + nameLen;

		if (!n)
		{
			rfc2231_paramDestroy(*paramList);
			return -1;
		}

		/*
		** A non-rfc 2231 parameter has paramnum set to 0, an
		** rfc 2231 parameter has paramnum set to its number, plus 1.
		*/

		if (*p == 0)
		{
			n->paramnum=0;
		}
		else
		{
			p++;

			n->paramnum=atoi(p)+1;

			if (n->paramnum <= 0)
				n->paramnum=1;
		}

		p=strrchr(attrName, '*');

		n->encoded=p && p[1] == 0;
		n->value=attrValue;

		for (o=paramList; *o; o= &(*o)->next)
			if ( (*o)->paramnum > n->paramnum)
				break;

		n->next= *o;
		*o=n;
	}
	return 0;
}


/*
** Create a link list of rfc2231param structures for a specific attribute
**
** Returns: 0 - ok, < 0 - out of memory.
*/

static int rfc2231_paramCreate(struct rfc2045attr *attr,
			       const char *name,
			       struct rfc2231param **paramList)
{
	*paramList=NULL;

	while (attr)
	{
		if (rfc2231_buildAttrList(paramList, name, attr->name,
					  attr->value) < 0)
			return (-1);
		attr=attr->next;
	}

	return (0);
}

static const char rfc2231_xdigit[]="0123456789ABCDEFabcdef";

static int nyb(char c)
{
	const char *p=strchr(rfc2231_xdigit, c);
	int n;

	if (!p)
		return 0;

	n=p-rfc2231_xdigit;

	if (n >= 16)
		n -= 6;

	return n;
}

/*
** Decode an rfc2231param link list.
**
** charset, language, text, are decoded, if the corresponding args below are
** not null.  Their corresponding lengths (including the null bytes) are
** always saved in the corresponding int args.  rfc2231_decode() is called
** twice to get the lengths, then once again after the buffers are allocated.
*/

void rfc2231_paramDecode(struct rfc2231param *paramList,
			 char *charsetPtr,
			 char *langPtr,
			 char *textPtr,
			 int *charsetLen,
			 int *langLen,
			 int *textLen)
{
	int first=1;

	*charsetLen=*langLen=*textLen=1;	/* null byte */

	if (paramList && paramList->paramnum == 0 &&
	    paramList->next)
		paramList=paramList->next;
	/*
	** Both a non-rfc2231 and an rfc2231 parameter was specified, so
	** take the better one.
	*/

	while (paramList)
	{
		const char *p=paramList->value;

		if (first && paramList->encoded)
		{
			const char *q=strchr(p, '\'');

			if (q && strchr(q+1, '\''))
			{
				while (*p != '\'')
				{
					if (charsetPtr)
						*charsetPtr++ = *p;
					p++;
					(*charsetLen)++;
				}
				p++;
				while (*p != '\'')
				{
					if (langPtr)
						*langPtr++ = *p;
					p++;
					(*langLen)++;
				}
				p++;
			}
		}

		first=0;

		while (*p)
		{
			if (*p == '%' && p[1] && p[2] && paramList->encoded)
			{
				if (textPtr)
					*textPtr++ = nyb(p[1]) * 16 +
						nyb(p[2]);
				p += 3;
			}
			else
			{
				if (textPtr)
					*textPtr++ = *p;

				p++;
			}

			(*textLen)++;
		}

		paramList=paramList->next;
	}

	if (charsetPtr)
		*charsetPtr=0;
	if (langPtr)
		*langPtr=0;
	if (textPtr)
		*textPtr=0;
}

/*
** Retrieve RFC 2231 information from a specific rfc2045attr list
**
** Returns 0 success, -1 for failure
*/

static int rfc2231_decode(struct rfc2045attr *attrList,
			  const char *name,

			  char **chsetPtr,
			  char **langPtr,
			  char **textPtr)
{
	int chsetLen;
	int langLen;
	int textLen;

	struct rfc2231param *paramList;

	if (rfc2231_paramCreate(attrList, name, &paramList) < 0)
		return -1;

	rfc2231_paramDecode(paramList, NULL, NULL, NULL,
			    &chsetLen,
			    &langLen,
			    &textLen);

	if (chsetPtr)
		*chsetPtr=NULL;

	if (langPtr)
		*langPtr=NULL;

	if (textPtr)
		*textPtr=NULL;


	if ((chsetPtr && (*chsetPtr=malloc(chsetLen)) == NULL)
	    || (langPtr && (*langPtr=malloc(langLen)) == NULL)
	    || (textPtr && (*textPtr=malloc(textLen)) == NULL))
	{
		rfc2231_paramDestroy(paramList);

		if (*chsetPtr)
			free(*chsetPtr);

		if (*langPtr)
			free(*langPtr);

		if (*textPtr)
			free(*textPtr);
		return (-1);
	}

	rfc2231_paramDecode(paramList,
			    chsetPtr ? *chsetPtr:NULL,
			    langPtr ? *langPtr:NULL,
			    textPtr ? *textPtr:NULL,
			    &chsetLen,
			    &langLen,
			    &textLen);
	return 0;
}

int rfc2231_decodeType(struct rfc2045 *rfc, const char *name,
		       char **chsetPtr,
		       char **langPtr,
		       char **textPtr)
{
	return rfc2231_decode(rfc->content_type_attr, name,
			      chsetPtr, langPtr, textPtr);
}

int rfc2231_decodeDisposition(struct rfc2045 *rfc, const char *name,
			      char **chsetPtr,
			      char **langPtr,
			      char **textPtr)
{
	return rfc2231_decode(rfc->content_disposition_attr, name,
			      chsetPtr, langPtr, textPtr);
}

static int conv_unicode(char **text, const char *fromChset,
			const char *toChset)
{
	int err;
	char *p;

	if (!toChset)
		toChset=unicode_default_chset();

	if (!fromChset || !*fromChset)
		return 0;

	p=unicode_convert_tobuf(*text, fromChset, toChset, &err);

	if (p && err)
	{
		free(p);
		p=NULL;
	}

	if (!p)
		return (-1);

	free(*text);
	*text=p;
	return (0);
}

int rfc2231_udecodeType(struct rfc2045 *rfc, const char *name,
			const char *myCharset,
			char **textPtr)
{
	char *text, *chset;

	if (rfc2231_decodeType(rfc, name, &chset, NULL, &text) < 0)
		return (-1);

	if (conv_unicode(&text, chset, myCharset) < 0)
	{
		free(text);
		free(chset);
		return (-1);
	}

	*textPtr=text;
	free(chset);
	return (0);
}

int rfc2231_udecodeDisposition(struct rfc2045 *rfc, const char *name,
			       const char *myCharset,
			       char **textPtr)
{
	char *text, *chset;

	if (rfc2231_decodeDisposition(rfc, name, &chset, NULL, &text) < 0)
		return (-1);

	if (conv_unicode(&text, chset, myCharset) < 0)
	{
		free(text);
		free(chset);
		return (-1);
	}

	*textPtr=text;
	free(chset);
	return (0);
}
