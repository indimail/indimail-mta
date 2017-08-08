dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2008 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(couriertcpd, 0.11, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(tcpd.c)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
LPATH="$PATH:/usr/local/bin"
AC_CONFIG_HEADERS(config.h)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL

SPATH="$LPATH:/usr/kerberos/bin"

AC_PATH_PROG(SED, sed, sed, $SPATH)
if test "$SED" = "sed"
then
	AC_MSG_ERROR(sed not found.)
fi
AC_SUBST(SED)

AC_PATH_PROG(SETENV, env, env, $SPATH)
if test "$SETENV" = "env"
then
	AC_MSG_ERROR(env not found.)
fi
AC_SUBST(SETENV)

AC_PATH_PROG(PKGCONFIG, pkg-config, [], $SPATH)

if test x$GCC = xyes
then
	CFLAGS="-Wall $CFLAGS"
fi
if test x$GXX = xyes
then
	CXXFLAGS="-Wall $CXXFLAGS"
fi

CFLAGS="-I$srcdir/.. -I.. -I$srcdir/../.. -I../.. $CFLAGS"
CXXFLAGS="-I$srcdir/.. -I.. -I$srcdir/../.. -I../.. $CXXFLAGS"

AC_PATH_PROG(KRB5CONFIG, krb5-config, krb5-config, $SPATH)

#
#
# We may need libdl for later, not just shared check.
#

saveLIBS="$LIBS"
AC_CHECK_LIB(dl, dlopen, [ LIBDL="-ldl" ])
LIBS="$saveLIBS"

dnl Determine whether transport libraries are created static or shared

AC_SUBST(LIBDL)

. ../../dbobj.config
if test "$dblibrary" != ""
then
	dblibrary="../../$dblibrary"
fi
AC_SUBST(dblibrary)

COURIERTCPD=""
if test "$dblibrary" != ""
then
	COURIERTCPD=couriertcpd$EXEEXT
fi

AC_SUBST(COURIERTCPD)

LIBS="$saveLIBS"

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

AC_SUBST(NETLIBS)

dnl Checks for header files.
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_CHECK_HEADERS(sys/types.h sys/time.h sys/stat.h sys/wait.h sys/select.h unistd.h fcntl.h sys/ioctl.h)
AC_HEADER_TIME

dnl Checks for typedefs, structures, and compiler characteristics.

AC_PID_T
AC_TYPE_UID_T
AC_TYPE_SIGNAL
AC_SYS_LARGEFILE

AC_CACHE_CHECK([for socklen_t],
	tcpd_cv_hassocklen_t,

AC_COMPILE_IFELSE([
AC_LANG_SOURCE( [
#include <sys/types.h>
#include <sys/socket.h>

socklen_t sl_t;
],[
	accept(0, 0, &sl_t);
])],
	tcpd_cv_hassocklen_t=yes,
	tcpd_cv_hassocklen_t=no)
)

socklen_t="int"

if test $tcpd_cv_hassocklen_t = yes
then
	:
else
	AC_DEFINE_UNQUOTED(socklen_t, int, [ Default definition for socklen_t ])
fi

dnl Checks for library functions.

AC_CHECK_FUNCS(setpgrp setpgid)
AC_CHECK_FUNC(setpgrp,
	[
	AC_FUNC_SETPGRP
	]
)

AC_ARG_WITH(tcpddns, [  --without-tcpddns       Use resolver instead of DNS lookups in couriertcpd ], tcpddns="$withval", tcpddns="Y")

case "$tcpddns" in
n*|N*|0*)
	;;
*)
	AC_DEFINE_UNQUOTED(TCPDUSERFC1035,1,
	[ Whether to use librfc1035.a to resolve via DNS ])
	;;
esac

saveLIBS="$LIBS"
LIBS="$LIBS $NETLIBS"
AC_CHECK_FUNC(socketpair, have_socketpair=yes, have_socketpair=no)

AC_ARG_WITH(spipe,
[  --with-spipe=socketpair Create stream pipes as BSD socketpairs
  --with-spipe=streams    Create stream pipes as SVR3 streams
  --with-spipe=pipe       Create stream pipes as SVR4 streams],
spipe="$withval", spipe="")

AC_CACHE_CHECK([for SVR3 stream pipes],tcpd_cv_svr3,
AC_TRY_COMPILE( [
#include <sys/types.h>
#include <sys/stream.h>
#include <stropts.h>
#include <fcntl.h>
],[ {
        struct strfdinsert      ins;
        queue_t                 *pointer;
	int i=I_FDINSERT;
}
], tcpd_cv_svr3=yes, tcpd_cv_svr3=no))

AC_CACHE_CHECK( [for SVR4 stream pipes],tcpd_cv_svr4,

AC_TRY_RUN(
[

changequote(<<,>>)

#include	<stdio.h>
#include	<unistd.h>

int main(int argc, char **argv)
{
int	pipefd[2];
char	c;

	c=0;
	if (pipe(pipefd) == 0 && write(pipefd[0], &c, 1) == 1 &&
		read(pipefd[1], &c, 1) == 1 &&
		write(pipefd[1], &c, 1) == 1 &&
		read(pipefd[0], &c, 1) == 1)
	{
		exit (0);
	}
	exit (1);
	return (1);
}
changequote([,])
]
,
tcpd_cv_svr4=yes,
tcpd_cv_svr4=no,
tcpd_cv_svr4="n/a")

)

LIBS="$saveLIBS"

if test "$spipe" = ""
then
	if test "$have_socketpair" = "yes"
	then
		spipe="socketpair"
	fi
fi

if test "$spipe" = ""
then
	if test "$tcpd_cv_svr3" = "yes"
	then
		spipe="streams"
	fi
fi

if test "$spipe" = ""
then
	if test "$tcpd_cv_svr4" = "yes"
	then
		spipe="pipe"
	fi
fi

case "$spipe" in
socketpair)
		AC_DEFINE_UNQUOTED(HAVE_SPIPE_SOCKETPAIR, 1,
		[ Whether to use socketpair() to create a bidirectional pipe ])
		;;
streams)
		AC_DEFINE_UNQUOTED(HAVE_SPIPE_SVR3, 1,
		[ Whether to use streams to create a vidirectional pipe ])
		;;
pipe)
		AC_DEFINE_UNQUOTED(HAVE_SPIPE_SVR4, 1,
		[ Whether pipe() is SvR4-style that's a bidirectional pipe ])
		;;
*)
	AC_MSG_ERROR(Cannot determine stream pipe support.  If cross-compiling use --with-spipe=pipe to get SVR4 stream pipes.)
		;;
esac


AC_MSG_CHECKING([for stream pipes])
AC_MSG_RESULT($spipe)

AC_CHECK_HEADER(openssl/ssl.h,have_ssl_h=yes,have_ssl_h=no)
AC_CHECK_LIB(ssl, SSL_load_error_strings, have_ssl=yes, have_ssl=no,
	-lcrypto)
AC_CHECK_LIB(ssl, SSL_library_init, have_ssl=yes)
AC_CHECK_LIB(ssl, OPENSSL_init_ssl, have_ssl=yes)

if test "$have_ssl" = "yes"
then
	if test "$have_ssl_h" = "no"
	then
		AC_MSG_WARN(OpenSSL runtime libraries installed but the header files are missing)
		AC_MSG_WARN(disabling OpenSSL support.)
		have_ssl="no"
	fi
fi

KRBFLAGS=""
if test "$have_ssl" = "yes"
then
	LIBCOURIERTLSOPENSSL=libcouriertlsopenssl.la

	if test "$KRB5CONFIG" != "krb5-config"
	then
		AC_MSG_CHECKING(whether OpenSSL requires Kerberos)
		AC_TRY_COMPILE( [
#include <openssl/ssl.h>
], [ int x=1; ], :,
		 [
			KRBFLAGS=`$KRB5CONFIG --cflags`
			CFLAGS="$CFLAGS $KRBFLAGS"


			AC_TRY_COMPILE( [
#include <openssl/ssl.h>
], [ int x=1; ], [ KRBLIBS="`$KRB5CONFIG --libs`" ],
   AC_MSG_ERROR(OpenSSL test build failed) )
			]
)

		if test "$KRBLIBS" = ""
		then
			AC_MSG_RESULT(no)
		else
			AC_MSG_RESULT(yes)
		fi
	fi
	openssldep="-lssl -lcrypto $KRBLIBS $NETLIBS ../random128/librandom128.la"
fi

# GnuTLS

have_gnutls=no

AC_MSG_CHECKING([for GnuTLS])

if test "$PKGCONFIG" != ""
then
	if $PKGCONFIG --modversion gnutls >/dev/null 2>&1
	then
		save_CPPFLAGS="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS `$PKGCONFIG --cflags gnutls`"

		AC_TRY_COMPILE( [
#include <gnutls/gnutls.h>
],
	[
	],
		[ have_gnutls=yes
		],

		[
			have_gnutls="no: \#include <gnutls/gnutls.h> failed"
		])

		CPPFLAGS="$save_CPPFLAGS"
	else
		have_gnutls="no: pkgconfig --modversion gnutls failed"
	fi
else
	have_gnutls="no: pkg-config not found"
fi

AC_MSG_RESULT($have_gnutls)

if test "$have_gnutls" = "yes"
then
	LIBCOURIERTLSGNUTLS="libcouriertlsgnutls.la"
	CPPFLAGS="$CPPFLAGS `$PKGCONFIG --cflags gnutls`"
	have_gnutls3=no
	PKG_CHECK_EXISTS([gnutls >= 3.0], [have_gnutls3=yes])
	if test "x$have_gnutls3" = "xno"
	then
		gnutlsdep="`$PKGCONFIG --libs gnutls` -lgnutls-extra"
	else
		gnutlsdep="`$PKGCONFIG --libs gnutls`"
		AC_DEFINE([HAVE_GNUTLS3], [1], [Use GnuTLS3])
	fi
fi

AC_CHECK_SIZEOF(gnutls_transport_ptr_t,0, [
AC_INCLUDES_DEFAULT
#include <gnutls/gnutls.h>
])

AC_CHECK_SIZEOF(long,0)

cast_to_ptr_t=""

if test "$gnutls_transport_ptr_t_SIZEOF" = "$long_SIZEOF"
then
	if test "$gnutls_transport_ptr_t_SIZEOF" != 0
	then
		cast_to_ptr_t="(long)"
	fi
fi

AC_DEFINE_UNQUOTED(GNUTLS_CAST_PTR_T, $cast_to_ptr_t,[How to cast a file descriptor to gnutls_transport_ptr_t])

echo "cppflags='$KRBFLAGS'" >couriertls.config

AC_SUBST(openssldep)
AC_SUBST(gnutlsdep)

soxdep="`cat ../soxwrap/soxlibs.dep`"
AC_SUBST(soxdep)


# Choose GnuTLS or OpenSSL

AC_ARG_WITH(gnutls, [ --with-gnutls     Use GnuTLS even if OpenSSL is available],
		    [

if test "$withval" = "yes"
then
	if test "$have_gnutls" = "yes"
	then
		have_ssl="no"
	else
		AC_MSG_ERROR(Cannot find GnuTLS)
	fi
else
	have_gnutls="no"
fi
],
	[

	if test "$have_ssl" = "yes"
	then
		have_gnutls="no"
	fi
])

if test "$have_gnutls" = "yes"
then
	TLSLIBRARY="$LIBCOURIERTLSGNUTLS"
	STARTTLS=couriertls$EXEEXT
	BUILDLIBCOURIERTLS=libcouriertls.la

	CRYPTLIBS=""

	AC_MSG_CHECKING([for libgcrypt])

	save_LIBS="$LIBS"
	LIBS="$LIBS -lgcrypt"
	AC_TRY_LINK( [
#include <gcrypt.h>

], [
	gcry_cipher_open(NULL, 0, 0, 0);
], [
	AC_MSG_RESULT(yes)
	AC_DEFINE_UNQUOTED(HAVE_GCRYPT, 1,
				[ Whether libgcrypt is installed ])
	CRYPTLIBS="$CRYPTLIBS -lgcrypt"
], [
AC_MSG_RESULT(no)
])

	AC_MSG_CHECKING([for libgpg-error])
	LIBS="$LIBS -lgpg-error"
	AC_TRY_LINK( [
#include <gcrypt.h>

], [
	gpg_err_code_from_errno(0);
], [
	AC_MSG_RESULT(yes)
	CRYPTLIBS="$CRYPTLIBS -lgpg-error"
], [
AC_MSG_RESULT(no)
])

	LIBS="$save_LIBS"

	echo "ssllib=gnutls" >>couriertls.config
else
	if test "$have_ssl" = "yes"
	then

		save_LIBS="$LIBS"
		LIBS="-lcrypto $KRBLIBS $LIBS"

		AC_MSG_CHECKING(for OpenSSL 0.9.7)
		AC_TRY_LINK( [
#include <openssl/evp.h>
#include <openssl/rand.h>
],
[
EVP_CIPHER_CTX ctx;
char dummy[1];
unsigned char a[1], b[1];

EVP_CIPHER_CTX_init(&ctx);
EVP_EncryptInit_ex(&ctx, EVP_des_cbc(), NULL, a, b);
RAND_pseudo_bytes(dummy, 1);

], [
   CRYPTLIBS="-lcrypto $KRBLIBS"
   AC_MSG_RESULT(yes)
   AC_DEFINE_UNQUOTED(HAVE_OPENSSL097, 1,
				       [ Whether OpenSSL 0.9.7 is installed ])
], [
   AC_MSG_RESULT(no)



		AC_MSG_CHECKING(for OpenSSL 1.1.0)
		AC_TRY_LINK( [
#include <openssl/evp.h>
#include <openssl/rand.h>
],
[
char dummy[1];
unsigned char a[1], b[1];

EVP_CIPHER_CTX *ctx=EVP_CIPHER_CTX_new();
EVP_EncryptInit_ex(ctx, EVP_des_cbc(), NULL, a, b);
RAND_bytes(dummy, 1);

], [
   CRYPTLIBS="-lcrypto $KRBLIBS"
   AC_MSG_RESULT(yes)
   AC_DEFINE_UNQUOTED(HAVE_OPENSSL110, 1,
				       [ Whether OpenSSL 1.1.0 is installed ])
], [
   AC_MSG_RESULT(no)
])

]
)
		LIBS="-lssl $LIBS"
		AC_CHECK_FUNCS(TLSv1_1_method TLSv1_2_method)
		LIBS="$save_LIBS"

		AC_TRY_COMPILE( [
#include <openssl/ssl.h>
],
[
SSL_get_servername((SSL *)0, TLSEXT_NAMETYPE_host_name);
], [
                AC_DEFINE_UNQUOTED(HAVE_OPENSSL_SNI,1,[ When OpenSSL supports SNI ])
		])

		TLSLIBRARY="$LIBCOURIERTLSOPENSSL"
		STARTTLS=couriertls$EXEEXT
		BUILDLIBCOURIERTLS=libcouriertls.la
		echo "ssllib=openssl" >>couriertls.config
	fi
fi
echo "couriertls=$STARTTLS" >>couriertls.config
echo "cryptlibs=\"`echo $CRYPTLIBS`\"" >>couriertls.config

# Do not built the other SSL library.  ... Except if I'm the maintainer

if test ! -d "$srcdir/CVS"
then
	if test "$have_ssl" = "yes"
	then
		LIBCOURIERTLSGNUTLS=""
		fi

	if test "$have_gnutls" = "yes"
	then
		LIBCOURIERTLSOPENSSL=""
	fi
fi

AC_SUBST(LIBCOURIERTLSOPENSSL)
AC_SUBST(LIBCOURIERTLSGNUTLS)

AC_SUBST(CRYPTLIBS)
AC_SUBST(STARTTLS)
AC_SUBST(BUILDLIBCOURIERTLS)
AC_SUBST(TLSLIBRARY)

AM_CONDITIONAL(HAVE_SGML, test -d ${srcdir}/../docbook)
AC_OUTPUT(Makefile)
