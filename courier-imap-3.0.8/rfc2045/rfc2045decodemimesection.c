/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2045.h"
#include	<stdio.h>
#include	<unistd.h>

static const char rcsid[]="$Id: rfc2045decodemimesection.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $";

/*
** This function is used to decode MIME section content, and pass it to
** a handler function.  It's basically a wrapper around rfc2045_cdecode
** functions.
*/

int rfc2045_decodemimesection(int fd, struct rfc2045 *rfc,
			      int (*handler)(const char *, size_t, void *),
			      void *voidarg)
{
	off_t	start_pos, end_pos, start_body;
	char	buf[BUFSIZ];
	int	cnt;
	off_t	dummy;
	int	rc;

	rfc2045_mimepos(rfc, &start_pos, &end_pos, &start_body,
		&dummy, &dummy);
	if (lseek(fd, start_body, SEEK_SET) == -1)	return (-1);

        rfc2045_cdecode_start(rfc, handler, voidarg);

        while (start_body < end_pos)
        {
                cnt=sizeof(buf);
                if (cnt > end_pos-start_body)
                        cnt=end_pos-start_body;
                cnt=read(fd, buf, cnt);
                if (cnt <= 0)   break;
                if ((rc=rfc2045_cdecode(rfc, buf, cnt)) != 0)
			return (rc);
		start_body += cnt;
        }
        return (rfc2045_cdecode_end(rfc));
}
