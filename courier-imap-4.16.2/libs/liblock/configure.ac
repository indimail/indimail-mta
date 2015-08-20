dnl
dnl Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_PREREQ(2.59)
AC_INIT(liblock, 0.60, courier-users@lists.sourceforge.net)

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR([liblock.h])
AC_CONFIG_AUX_DIR(../..)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign no-define])

CFLAGS="-I.. -I${srcdir}/.. $CFLAGS"

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
AC_CHECK_HEADERS(errno.h fcntl.h sys/fcntl.h sys/ioctl.h sys/file.h unistd.h sysexits.h)
AC_HEADER_STDC
AC_HEADER_SYS_WAIT

dnl Checks for typedefs, structures, and compiler characteristics.

AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIGNAL
AC_SYS_LARGEFILE

AC_CACHE_CHECK( [for off64_t], liblock_cv_hasoff64t,

liblock_cv_hasoff64t="no"
AC_TRY_COMPILE([
#include <sys/types.h>
],[
off64_t	n;

	n=0;
], liblock_cv_hasoff64t="yes")

)

if test "$liblock_cv_hasoff64t" = "yes"
then
	LL_OFFSET_TYPE=off64_t
else
	LL_OFFSET_TYPE=off_t
fi
AC_DEFINE_UNQUOTED(LL_OFFSET_TYPE,$LL_OFFSET_TYPE,
	[ Either off64_t or off_t ])

AC_CACHE_CHECK( [for flock_t], liblock_cv_hasflock_t,
liblock_cv_hasflock_t="no"
AC_TRY_COMPILE([
#if     HAVE_CONFIG_H
#include        "config.h"
#endif
#include        <sys/types.h>
#if     HAVE_FCNTL_H
#include        <fcntl.h>
#endif
#if     HAVE_SYS_FCNTL_H
#include        <sys/fcntl.h>
#endif
],[
flock_t t;
], liblock_cv_hasflock_t="yes"))

if test "$liblock_cv_hasflock_t" = "yes"
then
	AC_DEFINE_UNQUOTED(HAS_FLOCK_T,1,[ Whether flock_t is defined ])
fi

dnl Checks for library functions.

AC_CHECK_FUNCS(getdtablesize setpgrp sysconf)
AC_CHECK_FUNC(setpgrp,
	[
	AC_FUNC_SETPGRP
	]
)

AC_CACHE_CHECK( [for fcntl], liblock_cv_hasfcntl,
liblock_cv_hasfcntl="no"
AC_TRY_LINK([
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/lockfcntl.c"
],[
int n;
], liblock_cv_hasfcntl="yes"))
if test "$liblock_cv_hasfcntl" = "yes"
then
	AC_DEFINE_UNQUOTED(HAS_FCNTL,1,
		[ Whether fcntl() available ])
fi

AC_CACHE_CHECK( [for flock], liblock_cv_hasflock,
liblock_cv_hasflock="no"
AC_TRY_LINK([
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/lockflock.c"
],[
int n;
], liblock_cv_hasflock="yes"))
if test "$liblock_cv_hasflock" = "yes"
then
	AC_DEFINE_UNQUOTED(HAS_FLOCK,1,
		[ Whether flock() is available ])
fi

AC_CACHE_CHECK( [for lockf], liblock_cv_haslockf,
liblock_cv_haslockf="no"
AC_TRY_LINK([
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locklockf.c"
],[
int n;
], liblock_cv_haslockf="yes"))
if test "$liblock_cv_haslockf" = "yes"
then
	AC_DEFINE_UNQUOTED(HAS_LOCKF,1,
		[ Whether lockf() is available ])
fi

AC_MSG_CHECKING(for locking method)
AC_ARG_WITH(locking-method, [], lockmethod="$withval", lockmethod="")

case x$lockmethod in
xfcntl)
	AC_TRY_RUN([
#define USE_FCNTL 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], , AC_MSG_ERROR(fcntl test failed.),
	:
	)
	;;
x)
	AC_TRY_RUN([
#define USE_FCNTL 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], lockmethod=fcntl, ,AC_MSG_ERROR(--with-locking-method option required))
	;;
esac

case x$lockmethod in
xlockf)
	AC_TRY_RUN([
#define USE_LOCKF 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], , AC_MSG_ERROR(lockf test failed.),
	:
	)
	;;
x)
	AC_TRY_RUN([
#define USE_LOCKF 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], lockmethod=lockf, , AC_MSG_ERROR(--with-locking-method option required))
	;;
esac


case x$lockmethod in
xflock)
	AC_TRY_RUN([
#define USE_FLOCK 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], , AC_MSG_ERROR(flock test failed.),
	:
	)
	;;
x)
	AC_TRY_RUN([
#define USE_FLOCK 1
#define	LL_OFFSET_TYPE	$LL_OFFSET_TYPE
#include "${srcdir}/locktest.c"
], lockmethod=flock, , AC_MSG_ERROR(--with-locking-method option required))
	;;
esac

case x$lockmethod in
xlockf)
	;;
xflock)
	;;
xfcntl)
	;;
*)
	AC_MSG_ERROR(must specify --with-locking-method option)
	;;
esac

AC_MSG_RESULT($lockmethod)

AM_CONDITIONAL(USE_FCNTL, test "$lockmethod" = "fcntl")
AM_CONDITIONAL(USE_FLOCK, test "$lockmethod" = "flock")
AM_CONDITIONAL(USE_LOCKF, test "$lockmethod" = "lockf")

AM_CONDITIONAL(HAVE_SGML, test -d ${srcdir}/../docbook)

AC_DEFINE_UNQUOTED(liblock_config_h,1, [liblock/config.h has been read])

AC_OUTPUT(Makefile)
