dnl Process this file with autoconf to produce a configure script.

dnl Copyright 2000-2009 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(soxwrap, 0.50, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(soxwrap.h)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])

AC_CONFIG_HEADERS(soxwrap_config.h)

>confdefs.h  # Nuke PACKAGE_ macros.

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LN_S
AC_PROG_CC

dnl Check for options

AC_CACHE_CHECK([for the Courier Socks library],
		    ac_cv_courier_socks,

AC_COMPILE_IFELSE([
	AC_LANG_SOURCE([
#include <socks.h>

#ifndef courier_socks_h
#error Not a Courier socks header file
#endif
],[ int x=0; ])], [ac_cv_courier_socks=yes],[ac_cv_courier_socks=no])

)

AC_ARG_WITH(socks,
[--without-socks                    - Do not use the Courier Socks library
--with-socks                       - Use Courier Socks library],
[
	if test "$withval" = "yes"
	then
		if test "$ac_cv_courier_socks" = "no"
		then
			AC_MSG_ERROR([Courier Socks header files not found])
		fi
	else
		ac_cv_courier_socks=no
	fi
])

dnl Checks for libraries.

saveLIBS="$LIBS"
NETLIBS=""
USENSL=no

AC_CHECK_LIB(socket,socket,result=yes,result=no)
if test $result = yes; then
        NETLIBS="-lsocket"
else
        AC_CHECK_LIB(socket,socket,result=yes,result=no,-lnsl)
        if test $result = yes; then
                NETLIBS = "-lsocket -lnsl"
                USENSL=yes
        else
                AC_CHECK_LIB(socket,connect,result=yes,result=no)
                if test $result = yes; then
                        NETLIBS="-lsocket"
                else
                        AC_CHECK_LIB(socket,connect,result=yes,result=no,-lnsl)
                        if test $result = yes; then
                                NETLIBS="-lsocket -lnsl"
                                USENSL=yes
                        fi
                fi
        fi
fi

if test $USENSL != yes; then
	LIBS="$LIBS $NETLIBS"
	AC_TRY_LINK_FUNC(inet_addr, [ : ],
	[
	        AC_CHECK_LIB(nsl,inet_addr,result=yes,result=no)
	        if test $result = yes; then
	                NETLIBS="$NETLIBS -lnsl"
	        fi
	])
fi

LIBS="$saveLIBS"

if test "$ac_cv_courier_socks" = "yes"
then
	SOCKLIBS="-lsocks"
	DOSOCKS=1
else
	SOCKLIBS=$NETLIBS
	DOSOCKS=0
fi

AC_DEFINE_UNQUOTED(HAVE_SOCKS, $DOSOCKS, [ Whether to use the Courier Socks library ])

AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/select.h sys/poll.h pthread.h sys/stat.h fcntl.h sys/select.h sys/poll.h])
AC_HEADER_TIME
AC_SYS_LARGEFILE
AC_SUBST(SOCKLIBS)


echo $SOCKLIBS >soxlibs.dep
CPPFLAGS="-I.. -I$srcdir/.. $CPPFLAGS"
dnl Checks for typedefs, structures, and compiler characteristics.

AC_CACHE_CHECK([for structs in6_addr, sockaddr_in6, and sockaddr_storage],
	soxwrap_cv_hasipv6structs,

AC_TRY_COMPILE( [
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

        ], [
struct in6_addr in6a;
struct sockaddr_in6 sain6;
struct sockaddr_storage soas;
int x=PF_INET6;

	], soxwrap_cv_hasipv6structs=yes,
                soxwrap_cv_hasipv6structs=no )
)

AC_ARG_WITH(ipv6, [ --without-ipv6               Disable IPv6 support],
[
case $withval in
y*|Y*)
	if test "$soxwrap_cv_hasipv6structs" = no
	then
		AC_MSG_ERROR(IPv6 support not available)
	fi
	;;
*)
	soxwrap_cv_hasipv6structs=no
	;;
esac
]
)

storage="struct sockaddr"
storage_family="sa_family"

if test "$soxwrap_cv_hasipv6structs" = "yes"
then
	AC_DEFINE_UNQUOTED(HAVE_SOXWRAP_IPV6,1,[Whether IPv6 is available])
	storage="struct sockaddr_storage"
	storage_family="ss_family"
fi
AC_DEFINE_UNQUOTED(SOCKADDR_STORAGE, $storage, [ Whether sockaddr_storage is available ])
AC_DEFINE_UNQUOTED(SS_family, $storage_family, [ The address family field in SOCKADDR_STORAGE])

dnl Check for socklen_t

AC_CACHE_CHECK([for socklen_t],
        sox_cv_hassocklen_t,

AC_COMPILE_IFELSE([
AC_LANG_SOURCE( [
#include <sys/types.h>
#include <sys/socket.h>

socklen_t sl_t;
],[
        accept(0, 0, &sl_t);
])],
        sox_cv_hassocklen_t=yes,
        sox_cv_hassocklen_t=no)
)

socklen_t="int"

if test $sox_cv_hassocklen_t = yes
then
        :
else
        AC_DEFINE_UNQUOTED(socklen_t, int, [ Default definition for socklen_t ])
fi



AC_CHECK_FUNCS(inet_pton poll)
AC_OUTPUT(Makefile)
