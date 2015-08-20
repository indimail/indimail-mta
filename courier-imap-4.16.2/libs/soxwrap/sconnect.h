#ifndef	sconnect_h
#define	sconnect_h

#if	HAVE_CONFIG_H
#include	"soxwrap/soxwrap_config.h"
#endif

#if TIME_WITH_SYS_TIME
#include	<sys/time.h>
#include	<time.h>
#else
#if HAVE_SYS_TIME_H
#include	<sys/time.h>
#else
#include	<time.h>
#endif
#endif
#include	<sys/types.h>
#include	<sys/socket.h>

/*
** Copyright 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/


#ifdef  __cplusplus
extern "C"
#endif
int s_connect(int, const struct sockaddr *, size_t, time_t);

#endif
