/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/


#include	"config.h"
#include	"spipe.h"

#if HAVE_SPIPE_SOCKETPAIR

#include <sys/types.h>
#include <sys/socket.h>

int libmail_streampipe(int fd[2])
{
	return (socketpair(PF_UNIX, SOCK_STREAM, 0, fd));
}
#endif

#if HAVE_SPIPE_SVR3

/* This is, basically, Stevens */

#include	<sys/types.h>
#include	<sys/stream.h>		/* defines queue_t */
#include	<stropts.h>		/* defines struct strfdinsert */
#include	<fcntl.h>

#define	SPX_DEVICE	"/dev/spx"

int					/* return 0 if OK, -1 on error */
libmail_streampipe(int fd[2])
		/* two file descriptors returned through here */
{
	struct strfdinsert	ins;
	queue_t			*pointer;

	/*
	 * First open the stream clone device "/dev/spx" twice,
	 * obtaining the two file descriptors.
	 */

	if ( (fd[0] = open(SPX_DEVICE, O_RDWR)) < 0)
		return(-1);

	if ( (fd[1] = open(SPX_DEVICE, O_RDWR)) < 0) {
		close(fd[0]);
		return(-1);
	}

	/*
	 * Now link these two streams together with an I_FDINSERT ioctl.
	 */

	ins.ctlbuf.buf     = (char *) &pointer;	/* no ctl info, just the ptr */
	ins.ctlbuf.maxlen  = sizeof(queue_t *);
	ins.ctlbuf.len     = sizeof(queue_t *);

	ins.databuf.buf    = (char *) 0;	/* no data to send */
	ins.databuf.len    = -1; /* magic: must be -1, not 0, for stream pipe */
	ins.databuf.maxlen = 0;

	ins.fildes = fd[1];	/* the fd to connect with fd[0] */
	ins.flags  = 0;		/* nonpriority message */
	ins.offset = 0;		/* offset of pointer in control buffer */

	if (ioctl(fd[0], I_FDINSERT, (char * ) &ins) < 0) {
		close(fd[0]);
		close(fd[1]);
		return(-1);
	}

	return(0);		/* all OK */
}

#endif

#if HAVE_SPIPE_SVR4

#include	<stdio.h>
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#include	<stdlib.h>

int	libmail_streampipe(int fd[2])
{
	return (pipe(fd));
}

#endif
