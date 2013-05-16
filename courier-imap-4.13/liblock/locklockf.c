/*
** Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
** distribution information.
*/


#if	HAVE_CONFIG_H
#include	"config.h"
#endif
#include	<sys/types.h>
#if	HAVE_FCNTL_H
#include	<fcntl.h>
#endif
#if	HAVE_SYS_FCNTL_H
#include	<sys/fcntl.h>
#endif
#if	HAVE_UNISTD_H
#include	<unistd.h>
#endif
#if	HAVE_ERRNO_H
#include	<errno.h>
#endif
#include	"liblock.h"

int	ll_lockfd(int fd, int ltype, LL_OFFSET_TYPE start, LL_OFFSET_TYPE len)
{
off_t	p;

	if (ltype & ll_whence_curpos)
		p=lseek(fd, start, SEEK_CUR);
	else if (ltype && ll_whence_end)
		p=lseek(fd, start, SEEK_END);
	else	p=lseek(fd, start, SEEK_SET);

	if (p < 0)	return (-1);

	if (lockf(fd, ltype & ll_unlock ? F_ULOCK:
		ltype & ll_wait ? F_LOCK:F_TLOCK, len))
	{
		lseek(fd, p, SEEK_SET);
		return (-1);
	}
	lseek(fd, SEEK_SET, p);
	return (0);
}
