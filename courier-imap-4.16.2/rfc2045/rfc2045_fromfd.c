/*
** Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
** distribution information.
*/

/*
*/
#if	HAVE_CONFIG_H
#include "rfc2045_config.h"
#endif

#include	<sys/types.h>

#include	"rfc2045.h"
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif

/* Convert a message to the RFC2045 structure */

struct rfc2045 *rfc2045_fromfd(int fd)
{
struct	rfc2045	*rfc;
char	buf[BUFSIZ];
int	n;
off_t	orig_pos;

	if ((orig_pos=lseek(fd, 0L, SEEK_CUR)) == (off_t)-1) return (NULL);
	if (lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1)	return (NULL);
	if ((rfc=rfc2045_alloc()) == 0)	return (NULL);

	while ((n=read(fd, buf, sizeof(buf))) > 0)
		rfc2045_parse(rfc, buf, n);
	rfc2045_parse_partial(rfc);

	if (lseek(fd, orig_pos, SEEK_SET) == (off_t)-1)
	{
		rfc2045_free(rfc);
		rfc=0;
	}
	return (rfc);
}

/* Convert a message to the RFC2045 structure, halting after header */

struct rfc2045 *rfc2045header_fromfd(int fd)
{
struct        rfc2045        *rfc;
char        buf[BUFSIZ];
int        n;
off_t        orig_pos;

        if ((orig_pos=lseek(fd, 0L, SEEK_CUR)) == (off_t)-1) return (NULL);
        if (lseek(fd, (off_t)0, SEEK_SET) == (off_t)-1)        return (NULL);
        if ((rfc=rfc2045_alloc()) == 0)        return (NULL);

        while (rfc->workinheader && (n=read(fd, buf, sizeof(buf))) > 0)
                rfc2045_parse(rfc, buf, n);
        if (lseek(fd, orig_pos, SEEK_SET) == (off_t)-1)
        {
                rfc2045_free(rfc);
                rfc=0;
        }
        return (rfc);
}
