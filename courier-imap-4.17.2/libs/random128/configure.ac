dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2002 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(random128, 0.10, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(random128.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
LPATH="$PATH:/usr/local/bin"


dnl Checks for programs.
AC_CONFIG_HEADERS(config.h)
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC
AC_PATH_PROGS(PS, ps, ps, $LPATH)
AC_PATH_PROGS(W, w, w, $LPATH)
AC_PROG_LIBTOOL

if test "$PS" = "ps"
then
	AC_MSG_ERROR(Cannot find pathname to ps)
fi

if test x$GXX = xyes
then
	CFLAGS="-Wall $CFLAGS"
fi

CFLAGS="-I.. -I$srcdir/.. $CFLAGS"

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS(unistd.h fcntl.h)

AC_TYPE_PID_T
AC_SYS_LARGEFILE

AC_ARG_WITH(random, [ --with-random=/dev/urandom - location of the system random file generator
--without-random            - there is no system random file generator ],
	random="$withval",
	random="y")

case "$random" in
/*)
	;;
y*|1*)
	AC_CACHE_CHECK([for random source],random_cv_RANDOM,

		if test -c /dev/urandom
		then
			random_cv_RANDOM=/dev/urandom
		else
			if test -c /dev/random
			then
				random_cv_RANDOM=/dev/random
			else
				random_cv_RANDOM="none"
			fi
		fi
	)
	random="$random_cv_RANDOM"
	;;
*)
	random="none"
	;;
esac

if test "$random" != "none"
then
	AC_DEFINE_UNQUOTED(RANDOM, "$random", [ Entropy source ])
fi

AC_CACHE_CHECK([for some good options for ps],random_cv_PS_OPTIONS,

	for opts in -Afl -Afw -Af -Al -afl -afw -af -al Afl Afw Af Al afl afw af al
	do
		ps $opts >/dev/null 2>/dev/null || continue
		break
	done
	random_cv_PS_OPTIONS="$opts"
)

AC_DEFINE_UNQUOTED(PS_OPTIONS,"$random_cv_PS_OPTIONS",
	[ How to make ps(1) spit out lots of crap ])
AC_DEFINE_UNQUOTED(PS,"$PS", [ The PS program ])

if test "$W" != "w"
then
	AC_DEFINE_UNQUOTED(W, "$w", [ The W program ])
fi

AC_OUTPUT(Makefile)
