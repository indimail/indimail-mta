dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(waitlib, 0.50, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(waitlib.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])

AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_CC

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(sys/wait.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

dnl Checks for library functions.
AC_TYPE_SIGNAL
AC_SYS_LARGEFILE

AC_CHECK_FUNCS(wait wait3 sigblock sighold sigprocmask)

AC_ARG_WITH(waitfunc, [--with-waitfunc=wait3     Use the wait3 system call
--with-waitfunc-wait      Use the wait system call],

	waitfunc="$withval", waitfunc="")

case $waitfunc in
wait)
	;;
wait3)
	AC_DEFINE_UNQUOTED(USE_WAIT3, 1, [ Whether to use wait3() ])
	;;
"")
	AC_CACHE_CHECK([if wait function is broken],waitlib_cv_SYS_WAITBROKEN,

		AC_TRY_RUN([
#include	"confdefs.h"
#include	"$srcdir/confwait.c"
				], waitlib_cv_SYS_WAITBROKEN=no,
				waitlib_cv_SYS_WAITBROKEN=yes,
			AC_MSG_ERROR(Must specify --with-waitfunc when cross-compiling)))

	has_xsig=no;
	if test "$ac_cv_func_sigblock" = "yes"
	then
		has_xsig=yes
	fi
	if test "$has_sighold" = "yes"
	then
		has_xsig=yes
	fi

	if test "$waitlib_cv_SYS_WAITBROKEN$has_xsig$ac_cv_func_wait3" = "yesyesyes"
	then
		AC_CACHE_CHECK([if wait3 function is broken],waitlib_cv_SYS_WAIT3BROKEN,
			AC_TRY_RUN([
#define	USE_WAIT3	1
#include	"confdefs.h"
#include	"$srcdir/confwait.c"
				], waitlib_cv_SYS_WAIT3BROKEN=no,
				waitlib_cv_SYS_WAIT3BROKEN=yes,
			AC_MSG_ERROR(Must specify --with-waitfunc when cross-compiling)))
		use_wait3=yes
		if test $waitlib_cv_SYS_WAIT3BROKEN = yes
		then
			use_wait3=no
		fi
	else
		use_wait3=no
	fi
	
	if test "$waitlib_cv_SYS_WAITBROKEN$use_wait3" = "yesno"
	then
		AC_MSG_ERROR([I give up -- neither wait nor wait3 works properly])
	fi

	if test "$use_wait3" = "yes"
	then
		AC_DEFINE_UNQUOTED(USE_WAIT3)
	fi
	;;
*)
	AC_MSG_ERROR(Invalid --with-wait option.)
	;;
esac

if test x$GCC = xyes
then
	CFLAGS="-Wall $CFLAGS"
fi

AC_OUTPUT(Makefile)
