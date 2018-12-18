dnl Process this file with autoconf to produce a configure script.
dnl
dnl Copyright 1998 - 2018 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_PREREQ(2.59)
AC_INIT(courier-imap, 5.0.5, [courier-imap@lists.sourceforge.net])
AC_CONFIG_SRCDIR(libs/imap/imapd.c)
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE
>confdefs.h # Kill PACKAGE_ macros

AC_PROG_CC
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_LIBTOOL_DLOPEN
AC_CONFIG_MACRO_DIR([m4])
AM_PROG_LIBTOOL

AC_PATH_PROGS(SETENV, env)

defaultprefix="/usr/lib/courier-imap"
AC_SUBST(defaultprefix)

CXXFLAGS="-std=c++0x $CXXFLAGS"

AC_ARG_ENABLE(root-check, [  --disable-root-check    Allow build as root (make check will fail)],
	rootcheck="$enableval",
	rootcheck="yes")

if test "$rootcheck" = "yes"
then
if test -w /etc
then
	AC_MSG_WARN(=== Do not compile Courier-IMAP as root.  Compile)
	AC_MSG_WARN(=== Courier-IMAP as a non-root user then su to)
	AC_MSG_WARN(=== root before running make install.  You must now)
	AC_MSG_WARN(=== remove this entire directory and then extract the)
	AC_MSG_WARN(=== source code from the tarball as a non-root user)
	AC_MSG_WARN(=== and rerun the configure script.  If you have read)
	AC_MSG_WARN(=== the INSTALL file you should have known this.  So)
	AC_MSG_WARN(=== you better read INSTALL again.)
	AC_MSG_ERROR(aborted.)
fi
fi

PKG_CHECK_MODULES(LIBIDN, libidn >= 0.0.0, [libidn=yes], [libidn=no])
if test "$libidn" != "yes"
then
	AC_MSG_ERROR([libidn not found])
fi

AC_ARG_WITH(redhat, [], [ redhat=$withval ], [redhat=no])

if test "$srcdir" = "."
then
	case `./config.guess` in
	*-redhat-*)
		if test "$redhat" = "no"
		then
			hash='#'
			AC_MSG_WARN(=== I think you are trying to run this configure script)
			AC_MSG_WARN([=== on Red Hat/Fedora.  You're doing too much work!])
			AC_MSG_WARN([=== It's much faster to create installable binary RPMs])
			AC_MSG_WARN([=== like this: http://www.courier-mta.org/FAQ.html${hash}rpm])
			AC_MSG_WARN(=== When you do this you may find that RPM will tell you)
			AC_MSG_WARN([=== to install some other software first, before trying to])
			AC_MSG_WARN([=== build this one, and even tell you the name of RPMs you])
			AC_MSG_WARN([=== need to install from the distribution CD.  That's much])
			AC_MSG_WARN([=== easier than trying to figure out the same from some])
			AC_MSG_WARN([=== cryptic error message.])
			AC_MSG_WARN([])
			AC_MSG_WARN([=== Even if you don't intend to use everything you need to])
			AC_MSG_WARN([=== have in order to build via RPM, you should still do as])
			AC_MSG_WARN([=== you're told.  All the extra stuff (LDAP, SQL, etc...)])
			AC_MSG_WARN([=== goes into RPM sub-packages, which do not need to be])
			AC_MSG_WARN([=== installed.])
			AC_MSG_WARN([=== But, if you insist, you can simply add '--with-redhat'])
			AC_MSG_WARN(=== parameter to this configure script and not see this)
			AC_MSG_WARN(=== error message.  You should also do this when upgrading)
			AC_MSG_WARN([=== and you didn't use RPM with the older version.])
			AC_MSG_ERROR(... in either case you better know what you're doing!)
		fi
		;;
	*)
		;;
	esac
fi

AC_ARG_ENABLE(unicode,
	[],
	[
	unicode="$enableval"

	if test "$unicode" != "yes"
	then
		AC_MSG_ERROR(This option is no longer supported)
	fi
	],
	[
	ac_configure_args="$ac_configure_args --enable-unicode"
	unicode="yes"
	]
)

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

if test x$exec_prefix = xNONE
then
	exec_prefix="$prefix"
fi

eval "datadir=$datadir"

AC_ARG_WITH(certsdir, [ --with-certsdir Directory where certs are created ],
certsdir="$withval", certsdir="$datadir")

AC_SUBST(certsdir)

dnl Check whether we should use gdbm, or db.

needs_withdb=0

AC_ARG_WITH(db, [  --with-db=gdbm          Use the GDBM library.
  --with-db=db            Use the libdb.a library.],
	db="$withval", db=""; needs_withdb=1)

case "$db" in
gdbm)
	;;
db)
	;;
"")
	;;
*)
	AC_MSG_ERROR(Invalid --with-db option.)
	;;
esac

saveLIBS="$LIBS"

if test "$db" != "db"
then
	AC_CHECK_LIB(gdbm, gdbm_open, [ LIBGDBM=-lgdbm ; LIBS="-lgdbm $LIBS" ])
	AC_CHECK_FUNC(gdbm_open, [ HAVE_GDBM=y ])
fi

LIBS="$saveLIBS"

if test "$db" != "gdbm"
then

	AC_CHECK_LIB(db, dbopen, [ LIBDB=-ldb ; LIBS="-ldb $LIBS" ],
		AC_CHECK_LIB(db, db_open, [ LIBDB=-ldb ; LIBS="-ldb $LIBS"],
		AC_CHECK_LIB(db, db_env_create,
				[ LIBDB=-ldb; LIBS="-ldb $LIBS"])
	))

	AC_CHECK_FUNC(dbopen, HAVE_BDB=1)
	AC_CHECK_FUNC(db_open, HAVE_BDB=1)
	AC_CHECK_FUNC(db_env_create, HAVE_BDB=1)
fi

LIBS="$saveLIBS"

if test "$HAVE_GDBM$HAVE_BDB" = ""
then
	AC_MSG_ERROR(Cannot find either the gdbm or the db library.)
fi

USE_GDBM=0
USE_DB=0
if test "$HAVE_GDBM" = "y"
then
	if test "$needs_withdb" = 1
	then
		ac_configure_args="$ac_configure_args --with-db=gdbm"
	fi
	dblibrary=libs/gdbmobj/libgdbmobj.la
	USE_GDBM=1
else
	if test "$needs_withdb" = 1
	then
		ac_configure_args="$ac_configure_args --with-db=db"
	fi
	dblibrary=libs/bdbobj/libbdbobj.la
	USE_DB=1
fi

AC_SUBST(USE_GDBM)
AC_SUBST(USE_DB)
AC_SUBST(dblibrary)

AC_ARG_WITH(userdb, [], userdb="$withval", userdb=/etc/userdb
	ac_configure_args="$ac_configure_args --with-userdb=$userdb")
AC_SUBST(userdb)
AC_ARG_WITH(makedatprog, [], ,
	ac_configure_args="$ac_configure_args --with-makedatprog='${libexecdir}/makedatprog'")

AC_ARG_WITH(mailuser, [], , ac_configure_args="$ac_configure_args --with-mailuser=$authmailuser")
AC_ARG_WITH(mailgroup, [], , ac_configure_args="$ac_configure_args --with-mailgroup=$authmailgroup")
AC_SUBST(authmailuser)
AC_SUBST(authmailgroup)

AC_CONFIG_SUBDIRS(libs/numlib)
AC_CONFIG_SUBDIRS(libs/unicode)
AC_CONFIG_SUBDIRS(libs/bdbobj)
AC_CONFIG_SUBDIRS(libs/gdbmobj)
AC_CONFIG_SUBDIRS(libs/soxwrap)
AC_CONFIG_SUBDIRS(libs/rfc822)
AC_CONFIG_SUBDIRS(libs/rfc1035)
AC_CONFIG_SUBDIRS(libs/rfc2045)
AC_CONFIG_SUBDIRS(libs/md5)
AC_CONFIG_SUBDIRS(libs/sha1)
AC_CONFIG_SUBDIRS(libs/random128)
AC_CONFIG_SUBDIRS(libs/liblock)
AC_CONFIG_SUBDIRS(libs/maildir)
AC_CONFIG_SUBDIRS(libs/makedat)
AC_CONFIG_SUBDIRS(libs/libhmac)
AC_CONFIG_SUBDIRS(libs/waitlib)
AC_CONFIG_SUBDIRS(libs/tcpd)
AC_CONFIG_SUBDIRS(libs/rootcerts)
AC_CONFIG_SUBDIRS(libs/authlib)
AC_CONFIG_SUBDIRS(libs/imap)

mkdir -p m4

# Courier-IMAP standalone does not use SOCKSv5

AC_ARG_WITH(socks, [], , ac_configure_args="$ac_configure_args --without-socks")

# Neither does it use the change password feature

AC_ARG_WITH(authchangepwdir, [], ,
	ac_configure_args="$ac_configure_args --with-authchangepwdir=$libexecdir/imapmodules")

INSTALL_DATA='${INSTALL} -m 0600'
INSTALL_SCRIPT='${INSTALL}'

if ulimit -v 2>/dev/null
then
	ULIMIT='ulimit -v'
else
	ULIMIT='ulimit -d'
fi
AC_SUBST(ULIMIT)

#
# autoconf bug
#

ac_given_INSTALL="$INSTALL"

AC_ARG_WITH(package, [], package="$withval",
	[
		package="$PACKAGE"
		ac_configure_args="$ac_configure_args --with-package=$PACKAGE --with-version=$VERSION"
	])

AM_CONDITIONAL(HAVE_HTML, test -f ${srcdir}/html/main.html)

AC_ARG_WITH(certsdir, [ --with-certsdir Directory where certs are created ],
		      certsdir="$withval", certsdir=$datadir)

AC_CONFIG_FILES(Makefile dbobj.h dbobj.config imapd.rc imapd-ssl.rc
		pop3d.rc pop3d-ssl.rc makeimapaccess
		courier-imap.spec
		INSTALL.html
		packaging/suse/courier-imap.init
		packaging/systemd/courier-imap.sysvinit
		packaging/systemd/courier-imap.service)
AC_OUTPUT
