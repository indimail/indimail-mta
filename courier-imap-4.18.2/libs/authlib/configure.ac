#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([courier-authlib], [1.0], [manvendra@indimail.org],indimail, [http://www.indimail.org])
AC_CONFIG_SRCDIR(authoption.c)
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT
#AM_INIT_AUTOMAKE([foreign no-define])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-support@lists.sourceforge.net")

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([../../m4])
LPATH="$PATH:/usr/local/bin"
AC_PATH_PROGS(PERL, perl5 perl, perl, $LPATH)
if test "$PERL" = "perl"
then
	AC_MSG_ERROR(Perl is required)
fi

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/stat.h sys/time.h sys/wait.h unistd.h fcntl.h crypt.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([endpwent gethostname gettimeofday putenv strchr strdup strerror strncasecmp])

if test "$GCC" = "yes"
then
	CFLAGS="$CFLAGS -Wall"
fi

CFLAGS="$CFLAGS -I.. -I${srcdir}/.."

dnl Other checks
AC_CHECK_FUNCS(setsid setlogin)
AC_CHECK_LIB(crypt, crypt, CRYPTLIBS="-lcrypt")
saveLIBS="$LIBS"
LIBS="$CRYPTLIBS $LIBS"
AC_CHECK_FUNC(crypt, AC_DEFINE_UNQUOTED(HAVE_CRYPT, 1,
		     [ Whether we have the crypt() function ]))
LIBS="$saveLIBS"
AC_CACHE_CHECK([for crypt() prototype],userdb_cv_NEED_CRYPT_PROTOTYPE,

	AC_TRY_COMPILE( [
	       #if HAVE_CRYPT_H
               #include <crypt.h>
               #endif
	       #if HAVE_UNISTD_H
	       #include <unistd.h>
	       #endif
	       int crypt(int, int);

		], [], userdb_cv_NEED_CRYPT_PROTOTYPE=1,
			userdb_cv_NEED_CRYPT_PROTOTYPE=0 )

	)

AC_DEFINE_UNQUOTED(NEED_CRYPT_PROTOTYPE, $userdb_cv_NEED_CRYPT_PROTOTYPE,
			 [ Whether we must a prototype for crypt()] )
AC_SUBST(CRYPTLIBS)

AC_ARG_ENABLE(changepass,
[ --disable-changepass               Disable webmail password changing ],
	changepass="$enableval",
	changepass="yes")

if test "$changepass" = "no"
then
	changepass="0"
else
	changepass="1"
fi

AC_DEFINE_UNQUOTED(AUTHCHANGEPASS,$changepass, [ Whether password changing should be enabled ])

AC_PATH_PROGS(EXPECT, expect, expect, $LPATH)
if test "$EXPECT" = "expect"
then
	AC_MSG_WARN(-----------------------------------------------------)
	AC_MSG_WARN(expect not found - will not be able to change passwds)
	AC_MSG_WARN(in webmail)
	AC_MSG_WARN(-----------------------------------------------------)
	sleep 5
else
	AC_DEFINE_UNQUOTED(HAVE_EXPECT, 1, [ Whether expect(1) is installed ])
fi

AC_PATH_PROGS(PASSWD, passwd, passwd, $LPATH)
AC_SUBST(PASSWD)
AC_ARG_WITH(authchangepwdir, [],
	authchangepwdir="$withval",
	AC_MSG_ERROR(--with-authchangepwdir missing))

AC_SUBST(authchangepwdir)

dnl #########################################################################
dnl Check if the MD5 library is available
dnl #########################################################################

if test -d ../md5
then
	MD5=1
	AC_DEFINE_UNQUOTED(HAVE_MD5LIB, 1,
			[ Whether we have the MD5 library available ])
	MD5LIB=../md5/libmd5.la
else
	MD5LIB=""
	MD5=0
fi
AC_SUBST(MD5LIB)
AM_CONDITIONAL(HAVE_MD5, test "$MD5" = 1)

dnl #########################################################################
dnl Check if the SHA1 library is available
dnl #########################################################################


if test -d ../sha1
then
	SHA1=1
	AC_DEFINE_UNQUOTED(HAVE_SHA1LIB, 1,
			[ Whether we have the SHA1 library available ])
	SHA1LIB=../sha1/libsha1.la
else
	SHA1LIB=""
	SHA1=0
fi
AC_SUBST(SHA1LIB)
AM_CONDITIONAL(HAVE_SHA1, test "$SHA1" = 1)

STATIC_LIST=""
LIBAUTH=""
SASL_LIST="SASL(\"PLAIN\", authsasl_plain, authsaslclient_plain) SASL(\"LOGIN\", authsasl_login, authsaslclient_login) SASL(\"CRAM-MD5\", authsasl_cram, authsaslclient_crammd5) SASL(\"CRAM-SHA1\", authsasl_cram, authsaslclient_cramsha1) SASL(\"CRAM-SHA256\", authsasl_cram, authsaslclient_cramsha256)"

dnl #########################################################################

dnl Check if the HMAC library is available

dnl #########################################################################

if test -d ../libhmac
then
	HMAC=1
	AC_DEFINE_UNQUOTED(HAVE_HMACLIB, 1,
			 [ Whether we have the HMAC library available ])
	HMACLIB=../libhmac/libhmac.la
else
	HMACLIB=""
	HMAC=0
fi
AC_SUBST(HMACLIB)
AM_CONDITIONAL(HAVE_HMACLIB, test "$HMAC" = 1)

dnl #########################################################################

dnl Prepare authpam module if libpam is available.

dnl #########################################################################

AC_ARG_WITH(authpam,
[ --without-authpam                  Do not include the authpam module ],
	doauthpam="$withval")

if test "$doauthpam" = "no"
then
	HAVE_PAM=0
	LIBDL=""
else
	AC_CHECK_HEADERS(security/pam_appl.h Pam/pam_appl.h)

	saveLIBS="$LIBS"
	LIBDL=""
	AC_CHECK_LIB(dl, dlopen, [ LIBDL="-ldl" ])
	LIBS="$saveLIBS"

	AC_CHECK_LIB(pam, pam_start,
		[ HAVE_PAM=1
		LIBS="-lpam $LIBDL $LIBS"
		AC_CHECK_FUNCS(pam_setcred)],

		HAVE_PAM=0,
		$LIBDL)
	LIBS="$saveLIBS"
fi
AC_SUBST(LIBDL)
AC_SUBST(HAVE_PAM)
AM_CONDITIONAL(HAVE_PAM, test "$HAVE_PAM" = 1)

AUTHPAM=""
if test "$HAVE_PAM" = 1
then
	AUTHPAM="authpam${EXEEXT}"
	STATIC_LIST="$STATIC_LIST authpam_info"
	LIBAUTH="$LIBAUTH libauthpam.la"
fi

AC_ARG_WITH(authpam-libraries,
[ --with-authpam-libraries="libs"    Link 'libs' with authpam, this may be
                                    required for FreeBSD 3.3],
	authpamcrypt="$withval")

AUTHPAMCRYPT="$authpamcrypt"
AC_SUBST(AUTHPAMCRYPT)

dnl #########################################################################
dnl Prepare authpwd module
dnl #########################################################################

AC_ARG_WITH(authpwd,
[ --without-authpwd                  Do not include the authpwd module ],
	doauthpwd="$withval",
	doauthpwd="yes"
	test "$HAVE_PAM" = 1 && doauthpwd="no")

HAVE_PWD=1
if test "$doauthpwd" = "no"
then
	HAVE_PWD=0
fi
AC_CHECK_FUNCS(endpwent)

AUTHPWD=""
if test "$HAVE_PWD" = 1
then
	AUTHPWD="authpwd${EXEEXT}"
	STATIC_LIST="$STATIC_LIST authpwd_info"
	LIBAUTH="$LIBAUTH libauthpwd.la"
fi

AM_CONDITIONAL(HAVE_PWD, test "$HAVE_PWD" = 1)

dnl #########################################################################

dnl Prepare authshadow module if shadow functions are available.

dnl #########################################################################

AC_ARG_WITH(authshadow,
[ --without-authshadow               Do not include the authshadow module ],
	doauthshadow="$withval",
	doauthshadow="yes"
	test "$HAVE_PAM" = 1 && doauthshadow="no")

AC_CHECK_HEADERS(shadow.h)
saveLIBS="$LIBS"
SHADOWLIBS=""
AC_CHECK_LIB(shadow, getspent,
	[ SHADOWLIBS="-lshadow" ; LIBS="-lshadow $LIBS" ])
AC_CHECK_FUNCS(endspent getspent)
LIBS="$saveLIBS"

if test "$doauthshadow" = "no"
then
	HAVE_SHADOW=0
else

	saveLIBS="$LIBS"
	AC_CHECK_LIB(shadow, getspent)
	AC_CHECK_FUNC(getspent, HAVE_SHADOW=1, HAVE_SHADOW=0)
	LIBS="$saveLIBS"
fi

AUTHSHADOW=""
if test "$HAVE_SHADOW" = 1
then
	AUTHSHADOW="authshadow${EXEEXT}"
	STATIC_LIST="$STATIC_LIST authshadow_info"
	LIBAUTH="$LIBAUTH libauthshadow.la"
fi
AC_SUBST(SHADOWLIBS)

AM_CONDITIONAL(HAVE_SHADOW, test "$HAVE_SHADOW" = 1)

dnl #########################################################################
dnl Prepare authcustom stub module.
dnl #########################################################################


AC_ARG_WITH(authcustom,
[ --without-authcustom               Do not include the authcustom module ],
	doauthcustom="$withval",
	doauthcustom=yes)

AUTHCUSTOM=""

if test "$doauthcustom" = "yes"
then
	AUTHCUSTOM="authcustom${EXEEXT}"
	STATIC_LIST="$STATIC_LIST authcustom_info"
	LIBAUTH="$LIBAUTH libauthcustom.la"
fi

AM_CONDITIONAL(HAVE_CUSTOM, test "$AUTHCUSTOM" != "")

AC_DEFINE_UNQUOTED(SASL_LIST,$SASL_LIST,
	[ This macro is used to build the list of SASL modules ])

AUTHMODULES="$AUTHCUSTOM $AUTHPAM $AUTHSHADOW $AUTHPWD"

AC_SUBST(LIBAUTH)
AC_SUBST(STATIC_LIST)
AC_SUBST(AUTHMODULES)
#AC_CONFIG_FILES([Makefile])
AC_OUTPUT(Makefile installlist modulelist)
