dnl indimail-specific Autoconf macros.
dnl Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,
dnl 2004, 2005, 2006, 2007, 2008 Free Software Foundation, Inc.

dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.

dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

dnl Additional permission under GNU GPL version 3 section 7

dnl If you modify this program, or any covered work, by linking or
dnl combining it with the OpenSSL project's OpenSSL library (or a
dnl modified version of that library), containing parts covered by the
dnl terms of the OpenSSL or SSLeay licenses, the Free Software Foundation
dnl grants you additional permission to convey the resulting work.
dnl Corresponding Source for a non-source form of such a combination
dnl shall include the source code for the parts of OpenSSL used as well
dnl as that of the covered work.

dnl ************************************************************
dnl START OF IPv6 AUTOCONFIGURATION SUPPORT MACROS
dnl ************************************************************

AC_DEFUN([TYPE_STRUCT_SOCKADDR_IN6],[
  wget_have_sockaddr_in6=
  AC_CHECK_TYPES([struct sockaddr_in6],[
    wget_have_sockaddr_in6=yes
  ],[
    wget_have_sockaddr_in6=no
  ],[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
  ])

  if test "X$wget_have_sockaddr_in6" = "Xyes"; then :
    $1
  else :
    $2
  fi
])


AC_DEFUN([MEMBER_SIN6_SCOPE_ID],[
  AC_REQUIRE([TYPE_STRUCT_SOCKADDR_IN6])
  
  wget_member_sin6_scope_id=
  if test "X$wget_have_sockaddr_in6" = "Xyes"; then
    AC_CHECK_MEMBER([struct sockaddr_in6.sin6_scope_id],[
      wget_member_sin6_scope_id=yes
    ],[
      wget_member_sin6_scope_id=no
    ],[
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
    ])
  fi

  if test "X$wget_member_sin6_scope_id" = "Xyes"; then
    AC_DEFINE([HAVE_SOCKADDR_IN6_SCOPE_ID], 1,
      [Define if struct sockaddr_in6 has the sin6_scope_id member])
    $1
  else :
    $2
  fi
])


AC_DEFUN([PROTO_INET6],[
  AC_CACHE_CHECK([for INET6 protocol support], [wget_cv_proto_inet6],[
    AC_TRY_CPP([
#include <sys/types.h>
#include <sys/socket.h>

#ifndef PF_INET6
#error Missing PF_INET6
#endif
#ifndef AF_INET6
#error Mlssing AF_INET6
#endif
    ],[
      wget_cv_proto_inet6=yes
    ],[
      wget_cv_proto_inet6=no
    ])
  ])

  if test "X$wget_cv_proto_inet6" = "Xyes"; then :
    $1
  else :
    $2
  fi
])


AC_DEFUN([INDIMAIL_STRUCT_SOCKADDR_STORAGE],[
  AC_CHECK_TYPES([struct sockaddr_storage],[], [], [
#include <sys/types.h>
#include <sys/socket.h>
  ])
])

dnl ************************************************************
dnl END OF IPv6 AUTOCONFIGURATION SUPPORT MACROS
dnl ************************************************************
