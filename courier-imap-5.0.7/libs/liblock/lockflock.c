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
#if	HAVE_SYS_FILE_H
#include	<sys/file.h>
#endif
#include	<errno.h>
#include	"liblock.h"

int	ll_lockfd(int fd, int ltype, LL_OFFSET_TYPE start, LL_OFFSET_TYPE len)
{
	if (start || len
		|| (ltype & ll_whence_curpos)
		|| (ltype & ll_whence_end))
	{
		errno=EINVAL;
		return (-1);
	}

	return (flock(fd, ltype & ll_unlock ? LOCK_UN:
		(ltype & ll_writelock ? LOCK_EX:LOCK_SH) |
		(ltype & ll_wait ? 0:LOCK_NB)));
}
