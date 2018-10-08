dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2009 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(rfc822lib, 0.13, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(rfc822.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_LIBTOOL

if test "$GCC" = yes ; then
        CXXFLAGS="$CXXFLAGS -Wall"
        CFLAGS="$CFLAGS -Wall"
fi

CFLAGS="$CFLAGS -I.. -I$srcdir/.."

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(locale.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_SYS_LARGEFILE

dnl Checks for library functions.

AC_ARG_WITH(libidn, AC_HELP_STRING([--with-libidn=[DIR]],
		    [Support IDN (needs GNU Libidn)]),
		    libidn=$withval, libidn=yes)

if test "$libidn" != "no"
then
	PKG_CHECK_MODULES(LIBIDN, libidn >= 0.0.0, [libidn=yes], [libidn=no])
        if test "$libidn" != "yes"
	then
		libidn=no
         	AC_MSG_WARN([Libidn not found])
	else
		libidn=yes
		AC_DEFINE(LIBIDN, 1, [Define to 1 if you want Libidn.])
       	fi
fi
AC_MSG_CHECKING([if Libidn should be used])
AC_MSG_RESULT($libidn)

AC_CHECK_FUNCS(strcasecmp strncasecmp setlocale)

AC_CACHE_CHECK([how to calculate alternate timezone],librfc822_cv_SYS_TIMEZONE,

AC_TRY_COMPILE([
#include	<time.h>
],[
time_t	t=altzone;
], librfc822_cv_SYS_TIMEZONE=altzone,

	AC_TRY_COMPILE([
#include	<time.h>
],[
int	n=daylight;
	], librfc822_cv_SYS_TIMEZONE=daylight,

	AC_TRY_COMPILE([
#include	<time.h>

extern struct tm dummy;
],[
long	n=dummy.tm_gmtoff;
	] ,librfc822_cv_SYS_TIMEZONE=tm_gmtoff,
		librfc822_cv_SYS_TIMEZONE=unknown
			)
		)
	)
)

case $librfc822_cv_SYS_TIMEZONE in
tm_gmtoff)
	AC_DEFINE_UNQUOTED(USE_TIME_GMTOFF,1,
		[ The time offset is specified in the tm_gmtoff member ])
	;;
altzone)
	AC_DEFINE_UNQUOTED(USE_TIME_ALTZONE,1,
		[ The daylight savings time offset is in the altzone member ])
	;;
daylight)
	AC_DEFINE_UNQUOTED(USE_TIME_DAYLIGHT,1,
		[ The daylight savings time offset is in the tm_isdst member ])
	;;
*)
	AC_MSG_WARN([Cannot figure out how to calculate the alternate timezone, will use GMT])
	;;
esac

AM_CONDITIONAL(HAVE_SGML, test -d ${srcdir}/../docbook)

echo "libidn=$libidn" >rfc822.config
AC_OUTPUT(Makefile)
