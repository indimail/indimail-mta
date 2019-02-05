#ifndef	mksocket_h
#define	mksocket_h

#if	HAVE_CONFIG_H
#include	"soxwrap/soxwrap_config.h"
#endif


#ifdef  __cplusplus
extern "C" {
#endif

extern int mksocket(const char *address,
		    const char *service,
		    int socktype,
		    int *fd1,
		    int *fd2,
		    int recycle_fd_func( int(*)(int, void *), void *, void *),
		    void *voidarg);

#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#else

#define POLLIN      1
#define POLLPRI     2
#define POLLOUT     4
#define POLLERR     8
#define POLLHUP     16
#define POLLNVAL    32

struct pollfd {
	int fd;
	short events, revents;
};

extern int poll(struct pollfd *pfd, unsigned int n, int timeout);

#endif

#ifdef  __cplusplus
};
#endif

#endif
