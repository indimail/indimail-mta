dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(gdbmobj, 0.10, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(gdbmobj.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_ISC_POSIX
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_LIBTOOL

dnl Checks for libraries.

saveLIBS="$LIBS"
AC_CHECK_LIB(gdbm, gdbm_open, [ LIBGDBM=-lgdbm ; LIBS="-lgdbm $LIBS" ])

AC_CHECK_FUNC(gdbm_open, FOUND_GDBM=1, FOUND_GDBM=0)

if test "$FOUND_GDBM" = 1
then
	AC_CHECK_FUNCS(gdbm_fdesc)
fi
AC_CHECK_HEADERS(unistd.h fcntl.h)

LIBS="$saveLIBS"

AC_SUBST(LIBGDBM)

AM_CONDITIONAL(FOUND_GDBM, test "$FOUND_GDBM" = 1)

dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_SYS_LARGEFILE

dnl Checks for library functions.

AC_LANG_CPLUSPLUS
AC_CACHE_CHECK([if the C++ compiler needs -fhandle-exceptions],
	ac_cv_need_handlexceptions,

AC_TRY_COMPILE([],
[
throw;
], ac_cv_need_handlexceptions=no,
ac_cv_need_handlexceptions=yes)
)

if test "$ac_cv_need_handlexceptions" = "yes"
then
	case "$CXXFLAGS" in
	*handle-exceptions*)
		;;
	*)
		CXXFLAGS="-fhandle-exceptions $CXXFLAGS"
		CXXFLAGS=`echo "$CXXFLAGS" | sed 's/-O2//'`
		;;
	esac
fi

AC_OUTPUT(Makefile)
