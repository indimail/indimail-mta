dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(makedat, 0.60, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(makedatprog.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
LPATH="$PATH:/usr/local/bin"
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PATH_PROGS(PERL, perl5 perl, perl, $LPATH)
AC_SUBST(PERL)
AC_PATH_PROGS(CAT, cat, cat)
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL
AC_SYS_LARGEFILE

if test x$GCC = xyes
then
	CFLAGS="-Wall $CFLAGS"
fi
if test x$GXX = xyes
then
	CXXFLAGS="-Wall $CXXFLAGS"
fi

CFLAGS="-I$srcdir/.. -I.. -I$srcdir/../.. -I../.. $CFLAGS"
CXXFLAGS="-I$srcdir/.. -I.. -I$srcdir/../.. -I../.. $CXXFLAGS"

dnl Check whether we should use gdbm, or db.

. ../../dbobj.config
if test "$dblibrary" != ""
then
	dblibrary="../../$dblibrary"
fi
AC_SUBST(dblibrary)

makedatprog_target=""
if test "$dblibrary" != ""
then
	makedatprog_target=makedatprog
fi
AC_SUBST(makedatprog_target)
AC_SUBST(dblibrary)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

AC_ARG_WITH(makedatprog,[ --with-makedatprog=filename      Installed location and name of makedatprog], makedatprog="$withval", makedatprog="$bindir/makedatprog")

makedatprogpath="$makedatprog"
AC_SUBST(makedatprogpath)

AC_OUTPUT(Makefile makedat)
