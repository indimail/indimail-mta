/*
** Copyright 2000-2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	"rfc2045src.h"
#include	<stdio.h>
#include	<unistd.h>


/*
** This function is used to decode MIME section content, and pass it to
** a handler function.  It's basically a wrapper around rfc2045_cdecode
** functions.
*/

int rfc2045_decodemimesection(struct rfc2045src *src, struct rfc2045 *rfc,
			      int (*handler)(const char *, size_t, void *),
			      void *voidarg)
{
	off_t	start_pos, end_pos, start_body;
	char	buf[BUFSIZ];
	ssize_t	cnt;
	off_t	dummy;
	int	rc;

	rfc2045_mimepos(rfc, &start_pos, &end_pos, &start_body,
		&dummy, &dummy);
	if (SRC_SEEK(src, start_body) == (off_t)-1) return (-1);

        rfc2045_cdecode_start(rfc, handler, voidarg);

        while (start_body < end_pos)
        {
                cnt=sizeof(buf);
                if (cnt > end_pos-start_body)
                        cnt=end_pos-start_body;
                cnt=SRC_READ(src, buf, cnt);
                if (cnt <= 0)   break;
                if ((rc=rfc2045_cdecode(rfc, buf, cnt)) != 0)
			return (rc);
		start_body += cnt;
        }
        return (rfc2045_cdecode_end(rfc));
}
