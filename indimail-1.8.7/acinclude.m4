dnl Check for struct sockaddr_storage. Most IPv6-enabled hosts have it, but
dnl AIX 4.3 is one known exception.
AC_DEFUN([TYPE_SOCKADDR_STORAGE],
[
	AC_CHECK_TYPE([struct sockaddr_storage], AC_DEFINE(HAVE_STRUCT_SOCKADDR_STORAGE, 1,
                  [if struct sockaddr_storage is defined]), ,
	[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
	])
])
