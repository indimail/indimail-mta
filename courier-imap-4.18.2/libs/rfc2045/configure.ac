dnl
dnl Copyright 1998 - 2011 Double Precision, Inc.  See COPYING for
dnl distribution information.
AC_INIT(rfc2045lib, 0.20, [courier-maildrop@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(rfc2045.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
AC_CONFIG_HEADERS(rfc2045_config.h)

dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_CXX
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

if test "$GCC" = yes ; then
        CXXFLAGS="$CXXFLAGS -Wall"
        CFLAGS="$CFLAGS -Wall"
fi

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h sys/wait.h strings.h locale.h)

AC_CACHE_CHECK([for missing gethostname prototype],rfc2045_cv_SYS_GETHOSTNAME,

AC_TRY_COMPILE([
#if HAVE_UNISTD_H
#include	<unistd.h>
#endif

extern int gethostname(int,int);
],[
],rfc2045_cv_SYS_GETHOSTNAME=yes,rfc2045_cv_SYS_GETHOSTNAME=no
)

)

if test $rfc2045_cv_SYS_GETHOSTNAME = "no"
then
	AC_DEFINE_UNQUOTED(HAS_GETHOSTNAME,1,
	[ Whether unistd.h includes a prototype for gethostname() ])
fi

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AC_TYPE_PID_T
AC_SYS_LARGEFILE
AX_COURIER_UNICODE_CXXFLAGS
AC_SUBST(COURIER_UNICODE_CXXFLAGS)

dnl Checks for library functions.
AC_CHECK_FUNCS(setlocale)
AC_CHECK_FUNC(strncasecmp,,CFLAGS="$CFLAGS -Dstrncasecmp=strnicmp")
AC_CHECK_FUNC(strcasecmp,,CFLAGS="$CFLAGS -Dstrcasecmp=stricmp")

AC_ARG_WITH(package, [], package="$withval",
	[
		package="$PACKAGE"
		ac_configure_args="$ac_configure_args --with-package=$PACKAGE"
	])

AC_ARG_WITH(version, [], version="$withval",
	[
		version="$VERSION"
		ac_configure_args="$ac_configure_args --with-version=$VERSION"
	])

AC_DEFINE_UNQUOTED(RFC2045PKG,"$package", [ Package that uses librfc2045.a ])
AC_DEFINE_UNQUOTED(RFC2045VER,"$version", [ Package that uses librfc2045.a ])

AC_ARG_ENABLE(mimecharset,
	[  --enable-mimecharset=charset   Default MIME charset to set on new messages],
	RFC2045CHARSET="$enableval", RFC2045CHARSET="utf-8")

CFLAGS="$CFLAGS -I.. -I${srcdir}/.."
CXXFLAGS="$CXXFLAGS -I.. -I${srcdir}/.."
AC_SUBST(rfc822includedir)

AC_SUBST(RFC2045CHARSET)

AM_CONDITIONAL(HAVE_SGML, test -d ${srcdir}/../docbook)

. ../rfc822/rfc822.config

AM_CONDITIONAL(HAVE_LIBIDN, test "$libidn" = "yes")
AC_OUTPUT(Makefile rfc2045charset.h)
