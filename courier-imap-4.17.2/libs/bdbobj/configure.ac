dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 1999 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(bdbobj, 0.10, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(bdbobj.h)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_USE_SYSTEM_EXTENSIONS
AC_PROG_CC
AC_PROG_AWK
AC_PROG_CXX
AC_ISC_POSIX
AC_PROG_LIBTOOL

dnl Checks for libraries.

saveLIBS="$LIBS"
AC_CHECK_LIB(db, dbopen, [ LIBDB=-ldb ; LIBS="-ldb $LIBS" ],
	AC_CHECK_LIB(db, db_open, [ LIBDB=-ldb ; LIBS="-ldb $LIBS"],
	AC_CHECK_LIB(db, db_env_create, [ LIBDB=-ldb; LIBS="-ldb $LIBS"])
	))

FOUND_DB=0
AC_CHECK_FUNC(dbopen, FOUND_DB=1)
AC_CHECK_FUNC(db_open, FOUND_DB=1)
AC_CHECK_FUNC(db_env_create, FOUND_DB=1)

LIBS="$saveLIBS"

AC_SUBST(LIBDB)

AM_CONDITIONAL(FOUND_DB, test "$FOUND_DB" != 0)

dnl Checks for header files.

AC_CHECK_HEADERS(limits.h fcntl.h unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_SYS_LARGEFILE

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

if test "$GCC" = "yes"
then
	CFLAGS="-Wall $CFLAGS"
fi

AC_OUTPUT(Makefile)
