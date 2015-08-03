dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.63])
AC_INIT([cdb],1.0,[Manvendra Bhangui manvendra@indimail.org],indimail, [http://www.indimail.org])
AC_CANONICAL_HOST
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([silent-rules])
AM_CONFIG_HEADER(config.h)
LT_INIT

dnl Checks for programs.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CC

case "$host" in
*-*-sunos4.1.1*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-linux*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
*-*-darwin*)
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	;;
*)
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	;;
esac

defaultprefix="/var/indimail"
LIBVER=0:0:0
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(defaultprefix)
AC_SUBST(LIBVER)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_ISC_POSIX

dnl Checks for libraries.
dnl Checks for header files.
AC_HEADER_STDC
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_PROG_INSTALL

AC_OUTPUT(Makefile)

