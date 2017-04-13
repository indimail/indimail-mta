#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ircube],1.0,[Manvendra Bhangui manvendra@indimail.org],ircube, [http://www.indimail.org])
AM_INIT_AUTOMAKE

# Checks for programs.

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_ARG_ENABLE(plugindir, [  --enable-plugindir=dir         directory where roundcube plugins is installed.],
	plugindir="$enableval",
	[
		if test "$plugindir" = ""
		then
			AC_MSG_ERROR([Unable to find your plugindir directory, specify --enable-plugindir.])
		fi
	])
AC_DEFINE_UNQUOTED(QMAILDIR,"$plugindir",roundcube's plugin directory)
AC_SUBST(plugindir)

AC_OUTPUT(Makefile)
