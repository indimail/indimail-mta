dnl Process this file with autoconf to produce a configure script.

AC_INIT([libsrs2],m4_normalize(m4_include([conf-version])), m4_normalize(m4_include([conf-email])), libsrs2)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])

dnl Weird Unix variants
AC_USE_SYSTEM_EXTENSIONS

dnl Checks for programs.
AC_PROG_CC
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR
AC_PROG_INSTALL
AC_PROG_MAKE_SET
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

dnl More weird Unix variants
AC_SEARCH_LIBS([strerror],[cposix])

AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_C_VOLATILE
AC_C_BIGENDIAN
AC_C_INLINE
AC_C_CHAR_UNSIGNED
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(unsigned long)

dnl Checks for header files.
AC_CHECK_HEADERS(strings.h time.h sys/time.h sys/types.h unistd.h)
AC_CHECK_HEADERS(errno.h sys/select.h sys/socket.h stdarg.h getopt.h)
AC_CHECK_HEADERS(alloca.h)

AC_CHECK_LIB(socket, socket)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	LDFLAGS="$LDFLAGS -pie"
	;;
*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD"
	CFLAGS="$CFLAGS -I/usr/local/include -Wall"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -I/opt/local/include -Wall"
	LDFLAGS="$LDFLAGS -L/opt/local/lib"
	;;
*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	;;
esac

dnl Checks for library functions.
AC_CHECK_FUNCS(strdup strstr)
dnl Unix versions
AC_CHECK_FUNCS(alloca strcasecmp strncasecmp)
dnl Windows versions
AC_CHECK_FUNCS(_alloca _stricmp _strnicmp)

AC_CHECK_FUNCS(getopt_long)

LIBVER=1:0:0
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER)
AC_SUBST(VERSION)
AC_CONFIG_FILES([libsrs2.pc:libsrs2.pc.in],[], [])
AC_CONFIG_FILES([libsrs2/Makefile tools/Makefile Makefile])
AC_OUTPUT
