#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(src/action.c)
#AM_INIT_AUTOMAKE
AM_INIT_AUTOMAKE(logalert, 0.1)
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_SRCDIR([src/action.c])
#AC_CONFIG_HEADER([config.h])

AC_PREFIX_DEFAULT(/usr/local/bin)

# Checks for programs.
AC_PROG_CC
AC_PROG_LEX
AC_PROG_YACC

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/time.h unistd.h getopt.h])

have_pcre=no
LIBPCREDIR=libpcre

AC_ARG_WITH(libpcre,
    [  --with-libpcre=DIR    Use an existing (compiled) pcre lib from DIR/include and DIR/lib. ],
    [  case "$with_libpcre" in
       yes)
       ;;
       *)
    CXXFLAGS="-I$with_libpcre/include $CXXFLAGS"
    LDFLAGS="-L$with_libpcre/lib $LDFLAGS"
    have_pcre=yes
        ;;
  esac]
)

if test $have_pcre != yes ; then
	AC_CHECK_HEADER(pcre.h,
	  AC_CHECK_LIB(pcre, pcre_version, [have_pcre=yes ]),
	    [AC_CHECK_HEADERS(pcre.h,
            [AC_CHECK_LIB(pcre, pcre_version, [have_pcre=yes])]
           )]
        )

fi

if test $have_pcre != yes ; then

	echo "I could not find pcre library/headere, therefore I'm using"
	echo "regex.h header. Note that pcre is necessary to some funcionalities"
	echo "in logalert. See documentation for further details."
	echo "If you wish, visit http://www.pcre.org/"
	AC_CHECK_HEADERS([regex.h])
	AC_CHECK_FUNCS([regcomp])
else

	AC_DEFINE(HAVE_PCRE_H)
	LIBPCRE_LIBS="-lpcre"
	AC_SUBST(LIBPCRE_LIBS)
	AC_SUBST(LIBPCREDIR)
	AC_SUBST(PCRE_DEPENDS)
	AC_SUBST(PCRE_CLEAN)
	AC_SUBST(PCRE_DIST_CLEAN)

fi


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

AC_CANONICAL_HOST


case "$host" in

    *-openbsd*)
    AC_DEFINE(OPENBSD,,[Define if SOLARIS])
    ;;

    *-solaris*)
    AC_DEFINE(SOLARIS,,[Define if SOLARIS])
    ;;

    *-sunos*)
    AC_DEFINE(SUNOS,,[Define if SUNOS])
    ;;

    *-linux*)
    AC_DEFINE(LINUX,,[Define if LINUX])
    ;;

    *-freebsd*)
    AC_DEFINE(FREEBSD,,[Define if FREEBSD])
    ;;

    *-netbsd*)
    AC_DEFINE(NETBSD,,[Define if SOLARIS])
    ;;

    *)
    AC_DEFINE(LINUX,,[Define if LINUX])
    ;;

esac

test "$prefix" = "NONE" && prefix=/usr/local
test "$exec_prefix" = "NONE" && exec_prefix='${prefix}'
test "$program_prefix" = "NONE" && program_prefix=
test "$program_suffix" = "NONE" && program_suffix=


AC_DEFINE_UNQUOTED([BINDIR], ["$prefix/bin"],
		   [DO NOT CHANGE THIS - this will be used by logalert in PARENT mode. If you would like to change it, please do it via configure script.])

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([alarm getcwd strchr strrchr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT


