/*
** Copyright 1998 - 2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#ifndef	INCLUDED_FROM_CONFIGURE
#include	"waitlib.h"
#endif
#include	<signal.h>
#include	<stdio.h>


#if	HAVE_SIGPROCMASK

#define HOLD_CHILDREN {\
	sigset_t ss; sigemptyset(&ss); sigaddset(&ss, SIGCHLD); \
	sigprocmask(SIG_BLOCK, &ss, NULL);\
	}

#define RELEASE_CHILDREN {\
	sigset_t ss; sigemptyset(&ss); sigaddset(&ss, SIGCHLD); \
	sigprocmask(SIG_UNBLOCK, &ss, NULL);\
	}

#else
#if	HAVE_SIGHOLD

#define	HOLD_CHILDREN	sighold(SIGCHLD)
#define	RELEASE_CHILDREN	sigrelse(SIGCHLD)

#else

#define	HOLD_CHILDREN	sigblock(sigmask(SIGCHLD))
#define	RELEASE_CHILDREN	sigsetmask(0)

#endif
#endif

#if	USE_WAIT3

void wait_block()
{
	HOLD_CHILDREN;
}

void wait_clear(RETSIGTYPE (*func)(int))
{
	RELEASE_CHILDREN;
}

void wait_restore()
{
	signal(SIGCHLD, SIG_DFL);
	RELEASE_CHILDREN;
}

#else

void wait_block()
{
	signal(SIGCHLD, SIG_DFL);
}

void wait_clear(RETSIGTYPE (*func)(int))
{
	signal(SIGCHLD, func);
}

void wait_restore()
{
	signal(SIGCHLD, SIG_DFL);
}

#endif

void wait_reap( void (*func)(pid_t, int), RETSIGTYPE (*handler)(int))
{
int	dummy;
pid_t	p;

#if	USE_WAIT3

	HOLD_CHILDREN;

	while ((p=wait3(&dummy, WNOHANG, 0)) > 0)
#else
	if ((p=wait(&dummy)) > 0)
#endif
	{
		(*func)(p, dummy);
	}

	signal(SIGCHLD, handler);

#if	USE_WAIT3
	RELEASE_CHILDREN;
#endif

}

void wait_forchild( void (*reap)(pid_t, int), RETSIGTYPE (*func)(int))
{
pid_t	p;
int	wait_stat;

	signal(SIGCHLD, SIG_DFL);
	p=wait(&wait_stat);
	signal(SIGCHLD, func);
	(*reap)(p, wait_stat);
}
