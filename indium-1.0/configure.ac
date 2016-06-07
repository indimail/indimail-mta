#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([indium],1.0,[Manvendra Bhangui manvendra@indimail.org],indium, [http://www.indimail.org])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-support@lists.sourceforge.net")
AM_INIT_AUTOMAKE

AC_LIB_PREPARE_MULTILIB
AC_CONFIG_MACRO_DIR([m4])
dnl Does not work on RHEL 5, CentOS 5 64 bit systems
RPATH_LIB="$acl_libdirstem"

dnl Checks for libraries.
AC_CHECK_LIB(ssl, SSL_CTX_new, [AC_SUBST([LIB_SSL], ["-lssl -lcrypto"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl -lcrypto)
#AC_CHECK_LIB(ssl, EVP_MD_CTX_init, [AC_SUBST([LIB_SSL], ["-lssl"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl)
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	STRIP_OPT=-s
	;;
*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
esac

defaultprefix="/usr"
LIBVER=0:0:0
AC_SUBST(LIBVER)
AC_SUBST(STRIP_OPT)
AC_SUBST(RPATH_LIB)
AC_SUBST(defaultprefix)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_LIBTOOL

# Checks for libraries.
# FIXME: Replace `main' with a function in `-ltcl':
AC_CHECK_LIB([tcl], [Tcl_Init])
# FIXME: Replace `main' with a function in `-ltk':
AC_CHECK_LIB([tk], [Tk_Init])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset putenv strcasecmp strchr strerror strncasecmp strrchr])
AC_CHECK_LIB(ssl, EVP_MD_CTX_init, [AC_SUBST([LIB_SSL], ["-lssl"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl)

if test -f /usr/lib/tclConfig.sh
then
	. /usr/lib/tclConfig.sh
fi

shareddir=$prefix
AC_ARG_ENABLE(shareddir, [  --enable-shareddir=dir       directory for shared files.],
	shareddir="$enableval",
	[
	 	if test "$shareddir" != ""
		then
			AC_DEFINE_UNQUOTED(SHAREDDIR,"$shareddir", Directory for shared files.)
		fi
	])
AC_DEFINE_UNQUOTED(SHAREDDIR, "$shareddir", Directory for shared files)
AC_SUBST(shareddir)

DATE=`date`
AC_SUBST(DATE)

AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])

if test "$mysql_prefix" = ""
then
mysql_incdir=""
for f in /usr/include/mysql /usr/local/mysql/include/mysql /usr/local/mysql/include
do
	if test -d $f
	then
		mysql_incdir=$f
		break
	fi
done

AC_ARG_ENABLE(mysqlincdir, [  --enable-mysqlincdir=""       directory where auth include files are.],
mysql_incdir="$enableval",
[
	if test "$mysql_incdir" = ""
	then
		AC_MSG_ERROR([Unable to find your inc dir, specify --enable-mysqlincdir.])
	fi
])

mysql_libdir=""
for f in /usr/lib64/mysql /usr/local/mysql/lib64/mysql /usr/local/mysql/lib64 /usr/lib/mysql /usr/local/mysql/lib/mysql /usr/local/mysql/lib
do
	if test -d $f
	then
		mysql_libdir=$f
		break
	fi
done
AC_ARG_ENABLE(mysqllibdir, [  --enable-mysqllibdir=path-to-auth-libs directory where auth libs are.],
mysql_libdir="$enableval",
[
	if test "$mysql_libdir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql libraries dir, specify --enable-mysqllibdir.])
	fi
])
mysqlPrefix="`dirname $mysql_libdir`"
mysqlPrefix="`dirname $mysqlPrefix`"
else
	if test -d $mysql_prefix/include/mysql
	then
		mysql_incdir=$mysql_prefix/include/mysql
	elif test -d $mysql_prefix/include
	then
		mysql_incdir=$mysql_prefix/include
	else
		mysql_incdir=$mysql_prefix/include/mysql
	fi

	if test -d $mysql_prefix/lib64/mysql
	then
		mysql_libdir=$mysql_prefix/lib64/mysql
	elif test -d $mysql_prefix/lib64
	then
		mysql_libdir=$mysql_prefix/lib64
	elif test -d $mysql_prefix/lib/mysql
	then
		mysql_libdir=$mysql_prefix/lib/mysql
	elif test -d $mysql_prefix/lib
	then
		mysql_libdir=$mysql_prefix/lib
	else
		mysql_libdir=$mysql_prefix/lib/mysql
	fi
fi

AC_CHECK_LIB(mysqlclient -L${mysql_libdir}, mysql_real_connect,libs="$libs -lmysqlclient",
	AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)

auth_inc="$TCL_INCLUDE_SPEC -I$prefix/include/indimail -I$mysql_incdir"
auth_libs="-L. -L$libdir -L$mysql_libdir -lmysqlclient"

AC_SUBST(mysql_prefix)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(auth_inc)
AC_SUBST(auth_libs)
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
