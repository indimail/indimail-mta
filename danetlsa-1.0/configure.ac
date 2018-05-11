AC_PREREQ([2.69])
AC_INIT([danetlsa], [1.0], [Manvendra Bhangui manvendra@indimail.org], danetlsa, [http://www.indimail.org])
AC_CONFIG_SRCDIR([danetlsa.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
LT_INIT
AC_CONFIG_MACRO_DIR([m4])
AC_DISABLE_STATIC

# Checks for programs.
AC_PROG_CC
if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi
LIBVER=0:0:0
AC_SUBST(LIBVER)

# Checks for libraries.
AC_CHECK_LIB(ssl, SSL_CTX_new, [AC_SUBST([LIB_SSL], ["-lssl -lcrypto"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl -lcrypto)
AC_CHECK_LIB(getdns, getdns_context_create,,AC_MSG_ERROR([could not find getdns library needed for dns RR lookups]),)
AC_CHECK_LIB(getdns_ext_event, getdns_extension_set_libevent_base,,AC_MSG_ERROR([could not find getdns_ext_event library needed for async lookups]),)
AC_CHECK_LIB(event, event_base_new,,AC_MSG_ERROR([could not find libevent library for event handling]),)
# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h inttypes.h netdb.h netinet/in.h stdint.h stdlib.h string.h sys/socket.h unistd.h])
AC_CHECK_HEADERS([stdarg.h utils2/event.h event.h])
AC_CHECK_HEADERS([getdns/getdns.h getdns/getdns_extra.h getdns/getdns_ext_libevent.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

AC_DEFINE_UNQUOTED(LIBEXECDIR,"$libexecdir", Directory for architecture-dependent binaries)

# Checks for library functions.
AC_FUNC_MALLOC
AC_OUTPUT(Makefile)
