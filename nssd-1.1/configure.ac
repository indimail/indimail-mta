dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([nssd], 1.1,[Manvendra Bhangui manvendra@indimail.org],nssd, [http://www.indimail.org])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT([disable-static])
AM_INIT_AUTOMAKE([silent-rules])
AM_CONFIG_HEADER(config.h)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-support@lists.sourceforge.net")

dnl Checks for programs.
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET

dnl Checks for header files.
AC_ISC_POSIX
AC_HEADER_STDC
AC_C_CONST
AC_TYPE_UID_T
AC_TYPE_SIZE_T

AC_LIB_PREPARE_MULTILIB
RPATH_LIB="$acl_libdirstem"

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	OS=sunos
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
    LIBVER=1
	OS=solaris
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	#STRIP_OPT=-s
    LIBVER=2
	OS=linux
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	STRIP_OPT=-s
	OS=darwin
	;;
freebsd*)
	INSTALL="/usr/bin/install -c"
    CPPFLAGS="$CPPFLAGS -DFREEBSD"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	STRIP_OPT=-s
    LIBVER=1
	OS=freebsd
    ;;
*)
	INSTALL="/usr/bin/install -c"
	CFLAGS="$CFLAGS -s -O4 -Wall -fPIC"
	STRIP_OPT=-s
	;;
esac

defaultprefix="/var/indimail"

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_CHECK_HEADERS(syslog.h stdint.h shadow.h strings.h)
AC_CHECK_TYPES([socklen_t], , , [
#include <unistd.h>
#include <sys/socket.h>
])
AC_MSG_CHECKING("struct ucred\n")
AC_EGREP_HEADER([struct ucred], [sys/socket.h], have_ucred=yes)
dnl AC_EGREP_CPP([struct ucred], [
dnl				    #define _GNU_SOURCE
dnl				    #include <sys/socket.h>
dnl				], have_ucred=yes)
if test "$have_ucred" = "yes"; then
AC_DEFINE_UNQUOTED([HAVE_STRUCT_UCRED], 1, [Define if you have the struct ucred])
AC_MSG_RESULT(yes)
fi

AC_CHECK_FUNCS(strerror_r getgrouplist)
#FIND_MYSQL()
ACX_PTHREAD()

AC_CHECK_LIB(socket, getsockname)
AC_CHECK_LIB(nsl, gethostbyname)
AC_CHECK_LIB(m, floor)
AC_CHECK_LIB(dl, dlsym)
AC_CHECK_LIB(z, compress)

#mysql=""
#AC_ARG_ENABLE(mysql,     [--enable-mysql=DIR                        Location of your MySQL installation],
#	mysql="$enableval",
#	[
#		if test "$mysql" = ""
#		then
#			AC_MSG_ERROR([Unable to find your MySQL directory, specify --enable-mysql.])
#		fi
#	])
#MYSQL_LIB_DIR=$mysql;export MYSQL_LIB_DIR
#AC_DEFINE_UNQUOTED(MYSQL_LIB_DIR,"$mysql", MySQL Library Path)
#---------------------------------------------------------------------
AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])

if test "$mysql_prefix" = ""
then
mysql_incdir=""
for f in /usr/local/mysql/include /usr/include/mysql
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
] )

mysql_libdir=""
for f in /usr/local/mysql/lib64/mysql /usr/local/mysql/lib64 /usr/local/mysql/lib/mysql /usr/local/mysql/lib /usr/lib/mysql
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
] )
else
	if test -d $mysql_prefix/include
	then
		mysql_incdir=$mysql_prefix/include
	else
		mysql_incdir=/usr/include/mysql
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
	mysqlbindir=$mysql_prefix/bin
fi

AC_CHECK_LIB(mysqlclient -L${mysql_libdir}, mysql_real_connect,libs="$libs -lmysqlclient",
	AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
auth_inc="-I$indimaildir/include -I$mysql_incdir -I$mysql_incdir/mysql"
case "$host" in
	*-*-darwin*)
	auth_libs="-L$mysql_libdir -lmysqlclient"
	;;
	*)
	auth_libs="-L$mysql_libdir -lmysqlclient"
	;;
esac
#---------------------------------------------------------------------

AC_ARG_ENABLE(debug,          [--enable-debug                           Enable debug (see DEBUGGING)], [AC_DEFINE([DEBUG], 1, [Enable debug])])
defaultdomain=""
AC_ARG_ENABLE(default-domain, [--enable-default-domain=name             Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

socket_path=""
AC_ARG_ENABLE(socket_path,    [--enable-nssd-socket=/path/to/socket      socket for nssd daemon.],
socket_path="$enableval",
[
	if test "$socket_path" = ""
	then
		socket_path="/tmp/nssd.sock"
	fi
])
AC_DEFINE_UNQUOTED(_PATH_NSVSD_SOCKET,"$socket_path",full path of the NSVS Socket File)

config_path="$prefix/etc/nssd.conf"
AC_ARG_ENABLE(config_path, [--enable-nssd-config-path=$prefix/nssd.conf     configuration file for nssd daemon.],
config_path="$enableval",
[
	if test "$config_path" = ""
	then
		config_path="$prefix/etc/nssd.conf"
	fi
])
AC_DEFINE_UNQUOTED(_PATH_NSVSD_CONF,"$config_path",full path of the NSVS configuration File)

AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER)
AC_SUBST(OS)
AC_SUBST(defaultdomain)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(auth_inc)
AC_SUBST(auth_libs)
(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Host               = $host"
echo "CPPFLAGS           = $CPPFLAGS"
echo "CXXFLAGS           = $CXXFLAGS"
echo "CFLAGS             = $CFLAGS"
echo "LDFLAGS            = $LDFLAGS"
echo "            Current settings"
echo "---------------------------------------"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
#echo "       mysql  path = $mysql --enable-mysql=$mysql"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
echo "       socket Path = $socket_path --enable-nssd-socket-path=$socket_path"
echo "       config Path = $config_path --enable-nssd-config-path=$config_path"
) | tee nssd.settings
AC_OUTPUT(Makefile)
