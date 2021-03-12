#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([libdkim],m4_normalize(m4_include([conf-version])), [Manvendra Bhangui indimail-mta@indimail.org], libdkim)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_CONFIG_SRCDIR([dkimverify.h])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_HOST
LT_INIT
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

case "$host" in
*-*-sunos4.1.1*)
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wall -fPIC"
	;;
*-*-solaris*)
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wall -fPIC"
	;;
*-*-linux*)
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -DLINUX -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	LDFLAGS="$LDFLAGS -pie"
	;;
*-*-freebsd*)
	CPPFLAGS="$CPPFLAGS -DFREEBSD -I/usr/local/include"
	CFLAGS="$CFLAGS -I/usr/local/include -Wall"
	CXXFLAGS="$CXXFLAGS -DFREEBSD -I/usr/local/include -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -Wno-delete-non-virtual-dtor -Wno-deprecated-register -Wno-reorder"
	CXXFLAGS="$CXXFLAGS -Wno-delete-non-abstract-non-virtual-dtor"
	LDFLAGS="$LDFLAGS -L/usr/local/lib"
	;;
*-*-darwin*)
	CPPFLAGS="$CPPFLAGS -DDARWIN -I/opt/local/include"
	CFLAGS="$CFLAGS -I/opt/local/include -I/usr/local/include -Wall"
	CXXFLAGS="$CXXFLAGS -DDARWIN -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -DBIND_8_COMPAT -I/opt/local/include -Wno-c++11-extensions"
	CXXFLAGS="$CXXFLAGS -Wno-delete-non-virtual-dtor -Wno-reorder-ctor -Wno-reorder"
	LDFLAGS="$LDFLAGS -L/opt/local/lib -L/usr/local/lib"
	;;
*)
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wall -fPIC"
	;;
esac

defaultprefix="/var/indimail"
LIBVER=1:0:0
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(defaultprefix)
AC_SUBST(LIBVER)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_CHECK_SIZEOF(time_t)

# Checks for libraries.
AC_CHECK_LIB([crypto],[EVP_sha256],[AC_SUBST([LIB_CRYPTO],["-lcrypto"]) AC_DEFINE([HAVE_EVP_SHA256],[1],[HAVE_EVP_SHA256])],,-lcrypto)
if test " $LIB_CRYPTO" = " "
then
	AC_CHECK_LIB([crypto], [EVP_sha1],[AC_SUBST([LIB_CRYPTO], ["-lcrypto"])],,-lcrypto)
	if test " $LIB_CRYPTO" = " "
	then
    	AC_MSG_ERROR(Could not find crypto library.)
	fi
fi
AC_CHECK_LIB([resolv], [res_query],[AC_SUBST([LIB_RESOLV],["-lresolv"])],,-lresolv)
AC_CHECK_LIB([resolv], [__res_query],[AC_SUBST([LIB_RESOLV],["-lresolv"])],,-lresolv)

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([fcntl.h netdb.h netinet/in.h stdlib.h string.h unistd.h openssl/evp.h])
AC_CHECK_HEADERS([arpa/nameser.h nameser8_compat.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strerror strncasecmp])

AC_CONFIG_FILES([libdkim.pc:libdkim.pc.in],[], [])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
