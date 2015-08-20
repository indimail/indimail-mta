#ifndef	soxwrap_h
#define	soxwrap_h

/*
** Copyright 2000 Double Precision, Inc.
** See COPYING for distribution information.
*/


#ifdef	__cplusplus
extern "C" {
#endif

#include "soxwrap/soxwrap_config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

#define sox_init(s) 0
#define sox_socket socket

#if HAVE_SOCKS

#include <socks.h>

#define sox_getpeername	Rgetpeername
#define sox_getsockname	Rgetsockname
#define sox_accept	Raccept
#define sox_connect	Rconnect
#define sox_bind	Rbind
#define sox_listen	Rlisten
#define sox_recvfrom	Rrecvfrom
#define sox_sendto	Rsendto
#define sox_read	Rread
#define sox_write	Rwrite
#define sox_close	Rclose
#define sox_dup		Rdup
#define sox_dup2	Rdup2
#define sox_select	Rselect
#define sox_poll	Rpoll
#define sox_getsockopt	Rgetsockopt

#else

#define sox_getpeername	getpeername
#define sox_getsockname	getsockname
#define sox_accept	accept
#define sox_connect	connect
#define sox_bind	bind
#define sox_listen	listen
#define sox_recvfrom	recvfrom
#define sox_sendto	sendto
#define sox_read	read
#define sox_write	write
#define sox_close	close
#define sox_dup		dup
#define sox_dup2	dup2
#define sox_select	select
#define sox_poll	poll
#define sox_getsockopt	getsockopt
#endif

#ifdef	__cplusplus
}
#endif

#endif
