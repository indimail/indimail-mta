dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([fortune],1.1,[Manvendra Bhangui manvendra@indimail.org],indimail, [http://www.indimail.org])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AC_CANONICAL_HOST

dnl Checks for programs.
AC_PROG_CC
AC_PROG_RANLIB

defaultprefix="/var/indimail"
AC_SUBST(defaultprefix)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_ISC_POSIX
AC_HEADER_STDC
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_CHECK_HEADERS(regex.h)
AC_CHECK_HEADERS(rx.h)
AC_CHECK_HEADERS(netinet/in.h)
dnl Checks for libraries.
AC_CHECK_FUNC(regexec)
dnl Checks for header files.
AC_PROG_INSTALL
INSTALL_DATA="sh ./copy_fortune"

cookiedir=$prefix/share/fortunes
AC_ARG_ENABLE(cookiedir, [  --enable-cookiedir=dir   directory where fortune cookies is installed.],
	cookiedir="$enableval",
	[
		if test "$cookiedir" = ""
		then
			AC_MSG_ERROR([Unable to find your cookiedir directory, specify --enable-cookiedir.])
		fi
	])
AC_DEFINE_UNQUOTED(COOKIEDIR,"$cookiedir",  fortune cookie directory)
AC_DEFINE_UNQUOTED(OFFDIR,   "$cookiedir/off",  fortune cookie directory)
AC_DEFINE_UNQUOTED(FORTDIR,  "$cookiedir", fortune bin directory)

AC_ARG_ENABLE(offensive, [  --enable-offensive=y|n    Turn on (y, default) or off (n) to install offensive file],
	ENABLE_OFFENSIVE=$enableval,
	[
		ENABLE_OFFENSIVE=y
	])
case $ENABLE_OFFENSIVE in
1*|y*|Y*)
	ENABLE_OFFENSIVE=1
	AC_DEFINE_UNQUOTED(ENABLE_OFFENSIVE,$ENABLE_OFFENSIVE,Enable Offensive cookie files)
	AC_SUBST(ENABLE_OFFENSIVE)
	;;
*)
	;;
esac

AC_OUTPUT(Makefile)
echo "           $PACKAGE $VERSION $host     "
echo "            Current settings"
echo "---------------------------------------"
echo "            prefix = $prefix --prefix=$prefix"
echo "  cookie directory = $cookiedir --enable-cookiedir=$cookiedir"
case $ENABLE_OFFENSIVE in
1*|y*|Y*)
	echo "  Offensive Quotes = ON  --enable-offensive=y default"
	;;
*)
	echo "  Offensive Quotes = OFF --enable-offensive=n"
	;;
esac
