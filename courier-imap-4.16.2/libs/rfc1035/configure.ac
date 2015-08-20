dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2003 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(librfc1035, 0.10, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(rfc1035.h)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])

AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

dnl Checks for libraries.

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(sys/types.h sys/time.h unistd.h arpa/inet.h netinet/in.h)

USENSL=no
saveLIBS="$LIBS"
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

LIBS="$saveLIBS $NETLIBS"
AC_CHECK_LIB(socket,socket)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME
AC_SYS_LARGEFILE

AC_ARG_WITH(int32,
[  --with-int32='type'     use 'type' for an unsigned 32 bit integer type
                          ( default is 'unsigned')],
	int32="$withval", [

		AC_MSG_CHECKING(for uint32_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
		],[
			uint32_t	i=0;
		], [ AC_MSG_RESULT(yes) ; int32="uint32_t"], [

		AC_MSG_RESULT(no)
		AC_MSG_CHECKING(for u_int_32_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
			],[
				u_int32_t	i=0;
			], [AC_MSG_RESULT(yes); int32="u_int32_t"],[

			AC_MSG_RESULT(no)

			AC_CHECK_SIZEOF(unsigned, 0)
			if test "$ac_cv_sizeof_unsigned" != 4
			then
				AC_CHECK_SIZEOF(unsigned long, 0)
				if test "$ac_cv_sizeof_unsigned_long" != 4
				then
					AC_CHECK_SIZEOF(unsigned short, 0)
					if test "$ac_cv_sizeof_unsigned_short" != 4
					then
						AC_ERROR(--with-int32 option is required)
					fi
					int32="unsigned short"
				fi
				int32="unsigned long"
			else
				int32="unsigned"
			fi
			])
		])
	]
)
UINT32="$int32"

AC_ARG_WITH(int16,
[  --with-int16='type'     use 'type' for an unsigned 16 bit integer type
                          ( default is 'unsigned')],
	int16="$withval", [

		AC_MSG_CHECKING(for uint16_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
		],[
			uint16_t	i=0;
		], [ AC_MSG_RESULT(yes) ; int16="uint16_t"], [

		AC_MSG_RESULT(no)
		AC_MSG_CHECKING(for u_int_16_t)

		AC_TRY_COMPILE([
#if HAVE_SYS_TYPES_H
#include	<sys/types.h>
#endif
			],[
				u_int16_t	i=0;
			], [AC_MSG_RESULT(yes); int16="u_int16_t"],[

			AC_MSG_RESULT(no)

			AC_CHECK_SIZEOF(unsigned, 0)
			if test "$ac_cv_sizeof_unsigned" != 4
			then
				AC_CHECK_SIZEOF(unsigned long, 0)
				if test "$ac_cv_sizeof_unsigned_long" != 4
				then
					AC_CHECK_SIZEOF(unsigned short, 0)
					if test "$ac_cv_sizeof_unsigned_short" != 4
					then
						AC_ERROR(--with-int16 option is required)
					fi
					int16="unsigned short"
				fi
				int16="unsigned long"
			else
				int16="unsigned"
			fi
			])
		])
	]
)
UINT16="$int16"

AC_CACHE_CHECK([for socklen_t],
	courier_cv_hassocklen_t,
 
AC_COMPILE_IFELSE([
AC_LANG_SOURCE( [
#include <sys/types.h>
#include <sys/socket.h>
 
socklen_t sl_t;
],[
	accept(0, 0, &sl_t);
])],
	courier_cv_hassocklen_t=yes,
	courier_cv_hassocklen_t=no)
)
 
socklen_t="int"
 
if test $courier_cv_hassocklen_t = yes
then
	:
else
	AC_DEFINE_UNQUOTED(socklen_t, int, [ Default definition for socklen_t ])
fi

dnl Checks for library functions.

AC_CHECK_FUNCS(strcasecmp)

dnl Other checks

if test "$GCC" = "yes"
then
	CFLAGS="$CFLAGS -Wall"
fi

CFLAGS="$CFLAGS -I$srcdir/.. -I.."

dnl Check for IPv6 support

AC_CACHE_CHECK([for structs in6_addr, sockaddr_in6, and sockaddr_storage],
	rfc1035_cv_hasipv6structs,

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

	], rfc1035_cv_hasipv6structs=yes,
                rfc1035_cv_hasipv6structs=no )
)

changequote()

LB='['
RB=']'

changequote([,])

AC_CACHE_CHECK([for IPv6 flavor],
	rfc1035_cv_ipv6flavor,

if test "$rfc1035_cv_hasipv6structs" = no
then
	rfc1035_cv_ipv6flavor=none
else
AC_TRY_COMPILE( [
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
] , [
struct in6_addr in6a;

	in6a.s6_addr16 $LB 0 $RB =0;
	in6a.s6_addr32 $LB 0 $RB =0;
],
	rfc1035_cv_ipv6flavor="glibc (default)",

AC_TRY_COMPILE( [
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

        ], [
struct in6_addr in6a;

	in6a.__u6_addr.__u6_addr16 $LB 0 $RB =0;
	in6a.__u6_addr.__u6_addr32 $LB 0 $RB =0;
	],
		rfc1035_cv_ipv6flavor="freebsd-4.0",

AC_TRY_COMPILE( [
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

        ], [
struct in6_addr in6a;

       in6a._S6_un._S6_u8 $LB 0 $RB = 0;
       in6a._S6_un._S6_u32 $LB 0 $RB = 0;

       ],
		rfc1035_cv_ipv6flavor="solaris8",
		rfc1035_cv_ipv6flavor="unknown"
		)
	)
)
fi
)

RFC1035_FREEBSD40=0
if test "$rfc1035_cv_ipv6flavor" = "freebsd-4.0"
then
	RFC1035_FREEBSD40=1
	AC_DEFINE_UNQUOTED(RFC1035_FREEBSD40,1,
	[ Whether IPv6 support is FreeBSD-4.0 style ])
fi
AC_SUBST(RFC1035_FREEBSD40)

RFC1035_SOLARIS8=0

if test "$rfc1035_cv_ipv6flavor" = "solaris8"
then
	RFC1035_SOLARIS8=1
	AC_DEFINE_UNQUOTED(RFC1035_SOLARIS8,1,
	[ Whether IPv6 support is Solaris style ])
fi
AC_SUBST(RFC1035_SOLARIS8)

AC_CACHE_CHECK([for SIOCGIFCONF],
	rfc1035_cv_siocgifconf,

AC_TRY_COMPILE( [

#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
] , [
	struct ifreq ifreq_buf;
	struct ifconf ifc;

	ifc.ifc_len=sizeof(ifreq_buf);
	ifc.ifc_req=&ifreq_buf;

	ioctl(0, SIOCGIFCONF, &ifc);
], rfc1035_cv_siocgifconf=yes, rfc1035_cv_siocgifconf=no)
)

if test "$rfc1035_cv_siocgifconf" = "yes"
then
	AC_DEFINE_UNQUOTED(HAVE_SIOCGIFCONF,1,
	[ Whether SIOCGIFCONF ioctl is available ])
fi

AC_CACHE_CHECK([for alloca],
	rfc1035_cv_alloca,

AC_TRY_COMPILE( [
#include <stdio.h>
#include <stdlib.h>

        ], [
char *p=(char *)alloca(10);

	], rfc1035_cv_alloca=yes,
                rfc1035_cv_alloca,=no )
)

ipv6=0
if test "$rfc1035_cv_hasipv6structs$rfc1035_cv_alloca" = yesyes
then
	if test "$rfc1035_cv_ipv6flavor" != "unknown"
	then
		AC_CHECK_FUNC(inet_pton, [
			AC_CHECK_FUNC(inet_ntop, ipv6=1)
]
)
	fi
fi

AC_ARG_WITH(ipv6, [ --without-ipv6               Disable IPv6 support],
[
case $withval in
y*|Y*)
	if test "$ipv6" = 0
	then
		AC_MSG_ERROR(IPv6 support not available)
	fi
	;;
*)
	ipv6=0
	;;
esac
]
)

RFC1035_IPV6="$ipv6"

AC_DEFINE_UNQUOTED(RFC1035_UINT32, $UINT32, [ 32bit datatype ])
AC_DEFINE_UNQUOTED(RFC1035_UINT16, $UINT16, [ 16bit datatype ])
AC_DEFINE_UNQUOTED(RFC1035_IPV6, $RFC1035_IPV6,
	[ Whether IPv6 support is enabled ])

AC_OUTPUT(Makefile)
