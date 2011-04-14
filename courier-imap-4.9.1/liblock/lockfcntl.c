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
#include	"liblock.h"

int	ll_lockfd(int fd, int ltype, LL_OFFSET_TYPE start, LL_OFFSET_TYPE len)
{
#if HAS_FLOCK_T
flock_t	ft;
#else
struct flock ft;
#endif

	ft.l_type=ltype & ll_unlock ? F_UNLCK:
		ltype & ll_writelock ? F_WRLCK:F_RDLCK;
	ft.l_whence=ltype & ll_whence_curpos ? 1:
			ltype & ll_whence_end ? 2:0;
	ft.l_start=start;
	ft.l_len=len;

	return (fcntl(fd, (ltype & ll_unlock) == 0 && (ltype & ll_wait)
			? F_SETLKW:F_SETLK, &ft));
}
