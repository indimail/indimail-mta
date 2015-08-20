/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"


/*
** This function is generally called to find the "primary" text/plain
** section in a MIME message which, presumably, contains the main message
** text.
**
** Still the content type is set as a parameter in the event, I guess, we'll
** ever be able to grok text/html.
**
** We'll return a NULL pointer if we can't find it.
*/

struct rfc2045 *rfc2045_searchcontenttype(struct rfc2045 *rfc, const char *ct)
{
	const	char *content_type, *dummy;
	struct rfc2045 *p;

        rfc2045_mimeinfo(rfc, &content_type, &dummy, &dummy);
	if (strcmp(content_type, ct) == 0)
		return (rfc);

	for (p=rfc->firstpart; p; p=p->next)
	{
		if (p->isdummy)	continue;
		rfc2045_mimeinfo(p, &content_type, &dummy, &dummy);
		if (strcmp(content_type, ct) == 0)
			break;
		if (strncmp(content_type, "multipart/", 10) == 0)
			return(rfc2045_searchcontenttype(p, ct));
	}

	return (p);
}
