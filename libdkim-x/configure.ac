#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([libdkim],1.5, [Manvendra Bhangui indimail-mta@indimail.org], libdkim)
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_CONFIG_SRCDIR([dkimverify.h])
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-mta@indimail.org")
AC_CANONICAL_HOST
LT_INIT
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

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

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	CXXFLAGS="$CXXFLAGS -DDARWIN -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	STRIP_OPT=-S
	;;
*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	CXXFLAGS="$CXXFLAGS -O4 -Wno-delete-non-virtual-dtor -Wno-reorder -Wall -fPIC"
	STRIP_OPT=-s
	;;
esac

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

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strerror strncasecmp])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
