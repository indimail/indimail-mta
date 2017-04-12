#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ripmime],1.4.0.10,[Manvendra Bhangui manvendra@indimail.org],indimail, [http://www.indimail.org])
AC_CONFIG_SRCDIR([pldstr.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lc':
AC_CHECK_LIB([c], [main])
# FIXME: Replace `main' with a function in `-lsocket':
AC_CHECK_LIB([socket], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stddef.h stdlib.h string.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([alarm memmove memset mkdir strcasecmp strchr strdup strerror strncasecmp strpbrk strrchr strstr])

AC_OUTPUT(Makefile)
