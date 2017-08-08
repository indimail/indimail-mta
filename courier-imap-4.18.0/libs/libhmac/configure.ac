dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2001 Double Precision, Inc.  See COPYING for
dnl distribution information.


AC_INIT(libhmac, 1.00, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(hmac.h)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])


AC_CONFIG_HEADERS(config.h)

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

CPPFLAGS="$CPPFLAGS -I$srcdir/.. -I.."

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.

dnl Other checks

TESTPROGS=""

hmac_list=""

hashlibs=""
if test -d ${srcdir}/../md5
then
	TESTPROGS="$TESTPROGS md5hmactest${EXEEXT}"
	hmac_list="$hmac_list &hmac_md5,"
	hashlibs="$hashlibs ../md5/libmd5.la"
fi

if test -d ${srcdir}/../sha1
then
	TESTPROGS="$TESTPROGS sha1hmactest${EXEEXT}"
	hmac_list="$hmac_list &hmac_sha1, &hmac_sha256, "
	hashlibs="$hashlibs ../sha1/libsha1.la"
fi

AC_SUBST(hashlibs)
hmac_list="$hmac_list 0"

AC_SUBST(TESTPROGS)
AC_DEFINE_UNQUOTED(HMAC_LIST,$hmac_list,
	[ Dynamically-generated list of installed HMAC hash functions ])

AC_OUTPUT(Makefile)
