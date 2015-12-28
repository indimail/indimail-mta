#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([procmail], [3.22], [manvendra@indimail.org])
LT_INIT
AC_CONFIG_SRCDIR([patchlevel.h])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_DEFINE_UNQUOTED(PM_VERSION, $PACKAGE_VERSION, "Procmail Version")

# Checks for programs.
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -fPIC"
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -fPIC"
	;;
*-*-linux*)
	INSTALL="/usr/bin/install -c"
	if test x"$FAKED_MODE" = x
	then
		idcommand="/usr/bin/id -u"
	else
		idcommand="/usr/bin/id -u mail"
	fi
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -fPIC -fno-strict-aliasing"
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -pie"
	;;
*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -fPIC"
	;;
esac
AC_SUBST(idcommand)

AC_DEFINE_UNQUOTED(MAILSPOOLDIR, "/var/spool/mail/", mail spool dir)
AC_DEFINE_UNQUOTED(defPATH, "PATH=$HOME/bin:/bin:/usr/bin:/var/indimail/bin:/usr/local/bin", default path)
AC_DEFINE_UNQUOTED(defSPATH,"PATH=/bin:/usr/bin:/var/indimail/bin:/usr/local/bin", default system path)
if test -f /etc/shadow
then
AC_DEFINE_UNQUOTED(SHADOW_PASSWD,1,if shadow password is present)
fi

sendmail_prog=""
for f in $prefix/bin /usr/bin /usr/sbin
do
	if test -f $f/sendmail
	then
		sendmail_prog=$f/sendmail
		break
	fi
done
AC_ARG_ENABLE(sendmail-prog, [  --enable-sendmail-prog=/var/indimail/bin/sendmail where is your sendmail program.],
sendmail_prog="$enableval",
[
	if test "$sendmail_prog" = ""
	then
		AC_MSG_ERROR([Unable to find your sendmail prog, specify --enable-sendmail-prog.])
	fi
] )
AC_DEFINE_UNQUOTED(SENDMAIL,"$sendmail_prog",full path of tcprules program)

udp_protocol_no=17
AC_ARG_ENABLE(udp-protocol-no, [ --enable-udp-protocol-no=17 Your UDP protocol Number], udp_protocol_no=$enableval, [])
AC_DEFINE_UNQUOTED(UDP_protocolno,$udp_protocol_no,UDP Protocol No)

AC_ARG_ENABLE(file-locking, [  --enable-file-locking=f|c|l  (f) use flock, (c) use fcntl default, (l) use lockf],
	FILE_LOCKING=$enableval,
	[
		USE_FLOCK=0
		USE_LOCKF=0
		USE_FCNTLLOCK=1
	])
case $FILE_LOCKING in
f*|F*)
	USE_FLOCK=1
	USE_LOCKF=0
	USE_FCNTLLOCK=0
	AC_DEFINE_UNQUOTED(USE_FLOCK,$USE_FLOCK,Use flock for file locking)
	;;
l*|L*)
	USE_FLOCK=0
	USE_LOCKF=1
	USE_FCNTLLOCK=0
	AC_DEFINE_UNQUOTED(USE_LOCKF,$USE_LOCKF,Use lockf for file locking)
	;;
*)
	USE_FLOCK=0
	USE_LOCKF=0
	USE_FCNTLLOCK=1
	AC_DEFINE_UNQUOTED(USE_FCNTLLOCK,$USE_FCNTLLOCK,Use fcntl for file locking)
	;;
esac

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h shadow.h stddef.h stdlib.h string.h strings.h sys/file.h sys/socket.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm endgrent endpwent gethostbyname gethostname getspnam memchr strerror memmove mkdir munmap pow socket strchr strcspn strncasecmp strpbrk strstr strtol uname setruid setrgid setreuid setregid strlcat strpbrk memset bzero bcopy strtol getspnam strcspn strncasecmp uname fstat strtod fdlock initgroups pow ftruncate fsync waitpid])
AC_CHECK_LIB(m, pow)
AC_CHECK_LIB(crypt, crypt)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
