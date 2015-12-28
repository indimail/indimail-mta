dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.52])
AC_INIT([ripmime],1.4.0.9,[Manvendra Bhangui manvendra@indimail.org],ripmime, [http://www.indimail.org])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC

defaultprefix="/var/indimail"
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_ISC_POSIX

AC_TRY_RUN( [

#include <stdio.h>

int main()
{
	printf("hello word\n");
	return(0);
}
],
    AC_MSG_RESULT(yes),
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.),
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.))

dnl AC_CHECK_HEADERS(crypt.h)
dnl Checks for libraries.
dnl AC_CHECK_LIB(dl, main)
dnl AC_CHECK_FUNCS(setenv unsetenv)

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_PROG_INSTALL

AC_OUTPUT(Makefile)
