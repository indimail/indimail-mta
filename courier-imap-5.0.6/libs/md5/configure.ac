dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2004 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_PREREQ(2.59)
AC_INIT(libmd5, 1.21, courier-users@lists.sourceforge.net)

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR([hmac.c])
AC_CONFIG_AUX_DIR(../..)
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign no-define])

AM_CONDITIONAL(HMACC, test -d ${srcdir}/../libhmac)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_LIBTOOL

if test "$GCC" = yes
then
	CFLAGS="-Wall $CFLAGS"
fi

CFLAGS="$CFLAGS -I$srcdir/.. -I.."

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC

AC_CHECK_HEADERS(sys/types.h)


AC_ARG_WITH(int32,
[  --with-int32='type'     use 'type' for an unsigned 32 bit integer type
                          ( default is 'unsigned')],
	int32="$withval", [

		AC_MSG_CHECKING(for uint32_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
		],[
			uint32_t	i=0;
		], [ AC_MSG_RESULT(yes) ; int32="uint32_t"], [

		AC_MSG_RESULT(no)
		AC_MSG_CHECKING(for u_int_32_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
			],[
				u_int32_t	i=0;
			], [AC_MSG_RESULT(yes); int32="u_int32_t"],[

			AC_MSG_RESULT(no)

			AC_CHECK_SIZEOF(unsigned, 0)
			if test "$ac_cv_sizeof_unsigned" != 4
			then
				AC_CHECK_SIZEOF(unsigned long, 0)
				if test "$ac_cv_sizeof_unsigned_long" != 4
				then
					AC_CHECK_SIZEOF(unsigned short, 0)
					if test "$ac_cv_sizeof_unsigned_short" != 4
					then
						AC_ERROR(--with-int32 option is required)
					fi
					int32="unsigned short"
				fi
				int32="unsigned long"
			else
				int32="unsigned"
			fi
			])
		])
	]
)
UINT32="$int32"

AC_DEFINE_UNQUOTED(MD5_WORD, $UINT32, [ 32 bit data type ])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_SYS_LARGEFILE

dnl Checks for library functions.

AC_OUTPUT(Makefile)
