AC_PREREQ([2.52])
AC_INIT([pam-multi],1.1,[Manvendra Bhangui manvendra@indimail.org],pam-multi,[http://www.indimail.org])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE([silent-rules])
LT_INIT
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-support@lists.sourceforge.net")

AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(PACKAGE_TARNAME)

AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC
dnl Checks for programs.
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET

AC_ISC_POSIX
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h netinet/in.h netdb.h string.h strings.h sys/socket.h sys/types.h sys/stat.h sys/param.h fcntl.h syslog.h unistd.h stdarg.h errno.h crypt.h shadow.h dlfcn.h])
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_C_VOLATILE
AC_TYPE_PID_T

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	MODFLAGS="-g -O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	MODFLAGS="-g -module -O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -g -O4 -Wall -pie"
	MODFLAGS="-g -O4 -bundle -pie -Wall -undefined dynamic_lookup"
	STRIP_OPT=-S
	;;
*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -g -O4 -Wall -fPIC"
	MODFLAGS="-g -O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
esac

AC_CHECK_HEADERS([pam/pam_appl.h security/pam_appl.h pam_appl.h pam/pam_modules.h security/pam_modules.h pam_modules.h])

defaultprefix="/var/indimail"
LIBVER=0
AC_SUBST(LIBVER)
AC_SUBST(STRIP_OPT)
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args --prefix=$prefix"
fi

AC_CHECK_LIB(dl, dlopen, [AC_SUBST([LIB_DL], ["-ldl"]) AC_DEFINE([HAVE_DL], [1],[Dynamic Link Library])],,-ldl)
AC_CHECK_LIB(pam, pam_start, [AC_SUBST([LIB_PAM], ["-lpam"]) AC_DEFINE([HAVE_PAM], [1],[PAM Library])],,-lpam)
AC_CHECK_LIB(ssl, EVP_MD_CTX_init, [AC_SUBST([LIB_SSL], ["-lssl"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl)
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[Crypt Library])],,)

defaultdomain=""
AC_ARG_ENABLE(default-domain, [  --enable-default-domain=name  Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

moduledir=""
for f in /lib64/security /lib/security /usr/lib/pam
do
	if test -d $f
	then
		moduledir=$f
		break
	fi
done
AC_ARG_ENABLE(moduledir, [  --enable-moduledir=dir specify PAM module installation path],
  moduledir="$enableval",
  [
	  if test "$moduledir" = ""
	  then
		  AC_MSG_ERROR([Unable to find your pam module dir, specify --enable-moduledir.])
	  fi
  ])
AC_DEFINE_UNQUOTED(PAM_MOD_DIR,"$moduledir",full path to PAM Module Directory)
AC_DEFINE_UNQUOTED(USE_MYSQL,$USE_MYSQL,Use MySQL for primary authentication)
pam_confdir=/etc/pam.d
for f in /etc/pam.d /usr/etc/pam.d
do
  if test -d $f
  then
    pam_confdir=$f
    break
  fi
done
AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])

if test "$mysql_prefix" = ""
then
mysql_incdir=""
for f in /usr/local/mysql/include /usr/local/mysql/include/mysql /usr/include/mysql
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
])

mysqlbindir=""
for f in /usr/local/mysql/bin/mysql /usr/bin/mysql
do
if test -f $f
then
	mysqlbindir="`dirname $f`"
	break
	fi
done

AC_ARG_ENABLE(mysqlbindir, [  --enable-mysqlbindir=""       directory where mysql binary are.],
mysqlbindir="$enableval",
[
	if test "$mysqlbindir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql bin dir, specify --enable-mysqlbindir.])
	fi
])
mysql_prefix=`dirname $mysqlbindir`
else
	if test -d $mysql_prefix/include
	then
		mysql_incdir=$mysql_prefix/include
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
AC_DEFINE_UNQUOTED(MYSQLBINDIR,"$mysqlbindir",full path to MySQL binaries)

auth_inc="-I$mysql_incdir -I$mysql_incdir/mysql"
case "$host" in
	*-*-darwin*)
	auth_libs="-L. -L$mysql_libdir -lmysqlclient $LIB_SSL"
	;;
	*)
	auth_libs="-L. -Xlinker -R -Xlinker $mysql_libdir -L$mysql_libdir -lmysqlclient $LIB_SSL"
	;;
esac

AC_CHECK_LIB(m,floor)
AC_CHECK_LIB(z,compress)

HOST=$host
AC_SUBST(MODFLAGS)
AC_SUBST(auth_inc)
AC_SUBST(auth_libs)
AC_SUBST(moduledir)
AC_SUBST(pam_confdir)
AC_SUBST(HOST)

(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Host               = $host"
echo "CPPFLAGS           = $CPPFLAGS"
echo "CXXFLAGS           = $CXXFLAGS"
echo "CFLAGS             = $CFLAGS"
echo "LDFLAGS            = $LDFLAGS"
echo "MODFLAGS           = $MODFLAGS"
echo "            Current settings"
echo "---------------------------------------"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
echo "MySQL bin directory= $mysqlbindir --enable-mysqlbindir=$mysqlbindir"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
echo "         libraries = $libs"
echo "  module directory = $moduledir"
echo "          auth inc = $auth_inc"
echo "          auth lib = $auth_libs"
) | tee pam-multi.settings
AC_OUTPUT(Makefile)
AC_OUTPUT([pam-multi.spec])

dnl vim600: sts=2 sw=2 ts=2 et

