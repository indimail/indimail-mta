dnl Process this file with autoconf to produce a configure script.

AC_INIT([courier-unicode], [2.1], [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(unicodecpp.C)

AC_CONFIG_MACRO_DIR([m4])

LPATH="$PATH:/usr/local/bin"

AC_CONFIG_HEADERS(unicode_config.h)
AM_INIT_AUTOMAKE([foreign no-define dist-bzip2])
dnl Checks for programs.

AC_PROG_MAKE_SET
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CC
AC_PROG_CXX
AX_COURIER_UNICODE_CXXFLAGS
AC_SUBST(COURIER_UNICODE_CXXFLAGS)

AC_PATH_PROGS(PERL, perl5 perl, perl, $LPATH)

if test "$PERL" = "perl"
then
	AC_MSG_ERROR(Perl not found.)
fi

dnl Checks for libraries.

dnl Checks for header files.

AC_CHECK_HEADERS(stddef.h wchar.h)

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

have_langinfo_l=0

AC_CHECK_FUNC(nl_langinfo_l,[
    AC_TRY_COMPILE([
#include <langinfo.h>
],[
    nl_langinfo_l(CODESET, (locale_t)0);
],[
   have_langinfo_l=1
   ])])

if test "$have_langinfo_l" != 0
then
     AC_DEFINE(HAVE_LANGINFO_L,1,[nl_langinfo_l implemented])
     AC_SUBST(LANGINFO_L,1)
else
     AC_SUBST(LANGINFO_L,0)
fi


AC_TRY_COMPILE([
#include <uchar.h>
],[
char32_t c=0;
],
[
HAVE_UCHAR_H=1
],
[
HAVE_UCHAR_H=0
])

AC_SUBST(HAVE_UCHAR_H)

AM_CONDITIONAL(UPDATE_UNICODE, test -f ${srcdir}/UnicodeData.txt)

AC_SYS_LARGEFILE

AC_C_BIGENDIAN

AC_MSG_CHECKING([whether libcharset is used])
AC_ARG_WITH(libcharset,
	[  --with-libcharset     Use the libcharset library],
	unicode_with_libcharset=$withval,
	unicode_with_libcharset=no)
AC_MSG_RESULT($unicode_with_libcharset)
if test "$unicode_with_libcharset" = "yes"; then
  AC_CHECK_LIB(charset,locale_charset)
  if test "$ac_cv_lib_charset_locale_charset" = "yes"; then
    AC_CHECK_HEADERS(localcharset.h libcharset.h)
  else
    AC_MSG_ERROR([libcharset was not found.  Install libcharset package.])
  fi
  AC_DEFINE_UNQUOTED(UNICODE_USE_LIBCHARSET, 1,
	[ Set this to 1 to use libcharset library. ])
fi

save_LIBS="$LIBS"

LIBS="$LIBS -liconv"

AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <iconv.h>
],[

iconv_open("","");
])] ,[ : ], [LIBS="$save_LIBS"])

if test "$GCC" = "yes"
then
	CFLAGS="$CFLAGS -Wall -fexceptions"
fi
CFLAGS="-I.. -I$srcdir/.. $CFLAGS"
CXXFLAGS="-I.. -I$srcdir/.. $CXXFLAGS"

AM_CONDITIONAL(HAVE_DOCS,[test -f $srcdir/docbook/icon.gif])
AC_OUTPUT(Makefile courier-unicode.spec courier-unicode.h)
