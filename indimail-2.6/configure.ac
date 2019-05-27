dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([IndiMail],2.6,[Manvendra Bhangui manvendra@indimail.org],indimail, [http://www.indimail.org])
AC_CANONICAL_TARGET
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)
AC_DEFINE_UNQUOTED(PACKAGE_BUGREPORT, "indimail-support@lists.sourceforge.net")

dnl Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

dnl Checks for header files.
AC_ISC_POSIX
AC_HEADER_STDC
AC_CHECK_HEADERS(crypt.h openssl/evp.h inttypes.h)
AC_CHECK_HEADERS(unistd.h dirent.h fnmatch.h stdarg.h fcntl.h limits.h udb.h elf.h magic.h a.out.h malloc.h sys/utsname.h sys/wait.h)
AC_EGREP_HEADER(in_addr_t, netinet/in.h, AC_DEFINE(HAVE_IN_ADDR_T, 1, "have in_addr_t data type"),)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM
AC_TYPE_GETGROUPS

dnl Checks for library functions.
AC_CHECK_FUNCS(fdatasync sigaction sigprocmask fnmatch gethostname fchown getmntent getmntinfo setenv unsetenv)

if test "$GCC" = yes ; then
	CXXFLAGS="$CXXFLAGS -Wall"
fi

case "$host" in
*-*-sunos4.1.1*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSUNOS4"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-solaris*)
	INSTALL="./install-sh -c"
	idcommand="/usr/xpg4/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DSOLARIS"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-linux*)
	AC_LIB_PREPARE_MULTILIB
	INSTALL="/usr/bin/install -c"
	if test x"$FAKED_MODE" = x
	then
		idcommand="/usr/bin/id -u"
	else
		idcommand="/usr/bin/id -u mail"
	fi
	CPPFLAGS="$CPPFLAGS -DLINUX"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC -fno-strict-aliasing"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
*-*-darwin*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CPPFLAGS="$CPPFLAGS -DDARWIN"
	CFLAGS="$CFLAGS -O4 -Wall -pie"
	MODFLAGS="-g -O4 -bundle -pie -Wall -undefined dynamic_lookup"
	STRIP_OPT=-s
	;;
*)
	INSTALL="/usr/bin/install -c"
	idcommand="/usr/bin/id -u"
	CFLAGS="$CFLAGS -O4 -Wall -fPIC"
	MODFLAGS="-O4 -fPIC -shared -Wall"
	STRIP_OPT=-s
	;;
esac
defaultprefix="/var/indimail"
if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args --prefix=$prefix"
fi
tlib='${exec_prefix}/lib'
if test x$libdir = x$tlib
then
	dnl Does not work on RHEL 5, CentOS 5 64 bit systems
	if test -d /lib64 -a "$acl_libdirstem" = "lib"
	then
		acl_libdirstem=lib64
		libdir=$prefix/$acl_libdirstem
	fi
	ilibdir=$prefix/$acl_libdirstem
else
	ilibdir=$libdir
fi
RPATH_LIB="$acl_libdirstem"
if test -f /usr/bin/lsb_release
then
DISTRIB_ID=$(lsb_release -si)
DISTRIB_VERSION=$(lsb_release -sr)
else
DISTRIB_ID=$(sh ./lsb_release -si)
DISTRIB_VERSION=$(sh ./lsb_release -sr)
fi
AC_SUBST(DISTRIB_ID)
AC_SUBST(DISTRIB_VERSION)
LIBVER=2
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(LIBVER)
AC_SUBST(STRIP_OPT)
dnl AC_SUBST(RPATH_LIB)
AC_SUBST(defaultprefix)

dnl configure only packages for whom the subdirectories which are present
subdir_list=""
for i in cdb-1.0 eps-1.2 ../flash-0.9.4 ../ripmime-1.4.0.10 ../altermime-0.3.10 \
		../mpack-1.6 ../fortune-1.1 ../pam-multi-1.1 .
do
	if test -d $i
	then
		if test " $subdir_list" = " "
		then
			subdir_list="$i"
		else
			subdir_list="$subdir_list $i"
		fi
		if test -f $i/configure.in -o -f $i/configure.ac
		then
			case $i in
				cdb-1.0)
				AC_CONFIG_SUBDIRS([cdb-1.0])
				;;
				eps-1.2)
				AC_CONFIG_SUBDIRS([eps-1.2])
				;;
				../flash-0.9.4)
				AC_CONFIG_SUBDIRS([../flash-0.9.4])
				;;
				../ripmime-1.4.0.10)
				AC_CONFIG_SUBDIRS([../ripmime-1.4.0.10])
				;;
				../altermime-0.3.10)
				AC_CONFIG_SUBDIRS([../altermime-0.3.10])
				;;
				../mpack-1.6)
				AC_CONFIG_SUBDIRS([../mpack-1.6])
				;;
				../fortune-1.1)
				AC_CONFIG_SUBDIRS([../fortune-1.1])
				;;
				../pam-multi-1.1)
				AC_CONFIG_SUBDIRS([../pam-multi-1.1])
				;;
			esac
		fi
	else
		if test " $missing_list" = " "
		then
			missing_list="$i"
		else
			missing_list="$missing_list $i"
		fi
	fi
done

qmaildir=""
for f in /var/indimail /var/qmail
do
	if test -d $f
	then
		qmaildir=$f
	fi
done
AC_ARG_ENABLE(qmaildir, [  --enable-qmaildir=dir         directory where qmail is installed.],
	qmaildir="$enableval",
	[
		if test "$qmaildir" = ""
		then
			AC_MSG_ERROR([Unable to find your qmaildir directory, specify --enable-qmaildir.])
		fi
	])
AC_DEFINE_UNQUOTED(QMAILDIR,"$qmaildir",qmail's home directory)
AC_SUBST(qmaildir)

servicedir="/service"
AC_ARG_ENABLE(servicedir, [  --enable-servicedir=dir         directory where supervise services are installed.],
	servicedir="$enableval",
	[
		if test "$servicedir" = ""
		then
			AC_MSG_ERROR([Unable to find your servicedir directory, specify --enable-servicedir.])
		fi
	])
AC_DEFINE_UNQUOTED(SERVICEDIR,"$servicedir",supervise service directory)
AC_SUBST(servicedir)

for f in /var/postfix
do
	if test -d $f
	then
		postfixdir=$f
	fi
done

AC_ARG_ENABLE(postfixdir, [  --enable-postfixdir=dir       directory where Postfix is installed.],
	postfixdir="$enableval",
	[
		if test "$postfixdir" != ""
		then
			AC_DEFINE_UNQUOTED(POSTFIXDIR,"$postfixdir",Directory where Postfix is installed)
		fi
	])

controldir="$sysconfdir/control"
AC_ARG_ENABLE(controldir, [  --enable-controldir=dir       directory for control files.],
	controdir="$enableval",
	[
	 	if test "$domaindir" != ""
		then
			AC_DEFINE_UNQUOTED(CONTROLDIR,"$controldir", Directory for control files)
		fi
	])
AC_DEFINE_UNQUOTED(CONTROLDIR, "$controldir", Directory for control files)

assigndir="$sysconfdir/users"
AC_ARG_ENABLE(assigndir, [  --enable-assigndir=dir       directory for qmail users/assign.],
	assigndir="$enableval",
	[
	 	if test "$assigndir" != ""
		then
			AC_DEFINE_UNQUOTED(ASSIGNDIR,"$assigndir", Directory for qmail users/assign)
		fi
	])
AC_DEFINE_UNQUOTED(ASSIGNDIR, "$assigndir",  Directory for qmail users/assign)
AC_DEFINE_UNQUOTED(LIBEXECDIR,"$libexecdir", Directory for architecture-dependent binaries)
AC_DEFINE_UNQUOTED(SYSCONFDIR,"$sysconfdir", Directory for configuration files)

shareddir=$prefix
AC_ARG_ENABLE(shareddir, [  --enable-shareddir=dir       directory for shared files.],
	shareddir="$enableval",
	[
	 	if test "$shareddir" != ""
		then
			AC_DEFINE_UNQUOTED(SHAREDDIR,"$shareddir", Directory for shared files.)
		fi
	])
AC_DEFINE_UNQUOTED(SHAREDDIR, "$shareddir", Directory for shared files)

domaindir=$prefix
AC_ARG_ENABLE(domaindir, [  --enable-domaindir=dir       directory for virtual domains.],
	domaindir="$enableval",
	[
	 	if test "$domaindir" != ""
		then
			AC_DEFINE_UNQUOTED(DOMAINDIR,"$domaindir", Directory for virtual domains)
		fi
	])
AC_DEFINE_UNQUOTED(DOMAINDIR, "$domaindir", Directory for virtual domains)

basepath="/home/mail"
AC_ARG_ENABLE(basepath,   [  --enable-basepath=dir         Base Path for User's Home Directories.],
	basepath="$enableval",
	[
		if test "$basepath" = ""
		then
				AC_MSG_ERROR([Unable to find your base path, specify --enable-basepath.])
		fi
	])
AC_DEFINE_UNQUOTED(BASE_PATH,"$basepath",Directory where User's Home Directories get created)
AC_SUBST(basepath)

logdir="/var/log/svc"
AC_ARG_ENABLE(logdir,   [  --enable-logdir=dir           Base Path for System logs.],
	logdir="$enableval",
	[
		if test "$logdir" = ""
		then
				AC_MSG_ERROR([Unable to find your logdir path, specify --enable-logdir.])
		fi
	])
AC_DEFINE_UNQUOTED(LOGDIR, "$logdir",Directory where System Logs get created)
AC_SUBST(logdir)

indiuser="indimail"
AC_ARG_ENABLE(indiuser, [  --enable-indiuser=indimail    user IndiMail was installed as.],
	indiuser="$enableval",
	[
		if test "$indiuser" = ""
		then
			AC_MSG_ERROR([Unable to find your indiuser user, specify --enable-indiuser.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIUSER,"$indiuser",IndiMail User)
AC_SUBST(indiuser)
echo $indiuser > indiusername

indigroup="indimail"
AC_ARG_ENABLE(indigroup,[  --enable-indigroup=indimail   group IndiMail was installed as.],
	indigroup="$enableval",
	[
		if test "$indigroup" = ""
		then
			AC_MSG_ERROR([Unable to find your indigroup group, specify --enable-indigroup.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIGROUP,"$indigroup",IndiMail Group)
AC_SUBST(indigroup)
AC_SUBST(indimailuid)
AC_SUBST(indimailgid)

AC_CHECK_LIB(socket, socket,echo -lsocket > socket.lib,,)
AC_CHECK_LIB(ssl, SSL_CTX_new, [AC_SUBST([LIB_SSL], ["-lssl -lcrypto"]) AC_DEFINE([HAVE_SSL], [1],[OpenSSL Library])],,-lssl -lcrypto)
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)
AC_CHECK_LIB(ncurses, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-lncurses"]) AC_DEFINE([HAVE_TERMCAP_IN_NCURSES], [1],[ncurses Library])],notermcap=t,)
if  test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(ncurses5, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-lncurses5"]) AC_DEFINE([HAVE_TERMCAP_IN_NCURSES], [1],[ncurses Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(tinfo, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-ltinfo"]) AC_DEFINE([HAVE_TERMCAP], [1],[termcap Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
	notermcap=""
	AC_CHECK_LIB(termcap, tgetstr, [AC_SUBST([LIB_TERMCAP], ["-ltermcap"]) AC_DEFINE([HAVE_TERMCAP], [1],[termcap Library])],notermcap=t,)
fi
if test " $notermcap" = " t"
then
   	AC_MSG_ERROR(Could not find termcap library.)
fi
AC_CHECK_LIB(ncurses, initscr, [AC_SUBST([LIB_CURSES], ["-lncurses"]) AC_DEFINE([HAVE_NCURSES], [1],[ncurses Library])],nocurses=t,)
if test " $nocurses" = " t"
then
	nocurses=""
	AC_CHECK_LIB(ncurses5, initscr, [AC_SUBST([LIB_CURSES], ["-lncurses5"]) AC_DEFINE([HAVE_CURSES], [1],[curses Library])],nocurses=t,)
	if test " $nocurses" = " t"
	then
		nocurses=""
		AC_CHECK_LIB(curses, initscr, [AC_SUBST([LIB_CURSES], ["-lcurses"]) AC_DEFINE([HAVE_CURSES], [1],[curses Library])],nocurses=t,)
	fi
	if test " $nocurses" = " t"
	then
    	AC_MSG_ERROR(Could not find curses/ncurses library.)
	fi
fi

AC_MSG_CHECKING(for indimail user!!!)

AC_TRY_RUN( [

#include <stdio.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main()
{
struct passwd *pw;
char tmpbuf[100];
FILE *f;

	unlink("indimail.uid");
	unlink("indimail.gid");

	if ( (f=fopen("indiusername","r+"))==NULL) {
		fprintf(stderr, "could not open indiusername file\n");
		fprintf(stdout, "could not open indiusername file\n");
		exit(-1);
	}
    fgets(tmpbuf,100,f);
    fclose(f);
    tmpbuf[strlen(tmpbuf)-1] = 0;
	if (( pw=getpwnam(tmpbuf)) != 0 ) {
		f=fopen("indimail.dir","a+");
		fprintf(f,"%s\n", pw->pw_dir);
		fclose(f);

		f=fopen("indimail.uid", "a+");
		fprintf(f,"%d\n", pw->pw_uid);
		fclose(f);

		f=fopen("indimail.gid", "a+");
		fprintf(f,"%d\n", pw->pw_gid);
		fclose(f);
	} else {
		f=fopen("indimail.dir", "a+");
		fprintf(f,"/var/indimail\n");
		fclose(f);
		f=fopen("indimail.uid", "a+");
		fprintf(f,"555\n");
		fclose(f);

		f=fopen("indimail.gid", "a+");
		fprintf(f,"555\n");
		fclose(f);
	}
	return(0);
}
], [AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

if test ! -f indimail.uid
then
	AC_MSG_ERROR(No indimail user.)
fi

if test ! -f indimail.gid
then
	AC_MSG_ERROR(No indimail group.)
fi

if test ! -f indimail.dir
then
	indimaildir=$qmaildir
else
	indimaildir=`cat indimail.dir`
fi

DATE=`date`
AC_SUBST(DATE)
AC_SUBST(indimaildir)
AC_SUBST(controldir)
AC_SUBST(domaindir)
AC_SUBST(assigndir)
AC_SUBST(shareddir)
AC_DEFINE_UNQUOTED(INDIMAILDIR,"$indimaildir",IndiMail Home Directory)
AC_DEFINE_UNQUOTED(PREFIX,"$prefix",Prefix)
rm -f indimail.dir

indimailuid=`cat indimail.uid`
rm -f indimail.uid

indimailgid=`cat indimail.gid`
rm -f indimail.gid
rm -f indiusername

defaultdomain=""
AC_ARG_ENABLE(default-domain, [  --enable-default-domain=name  Default domain name, default is null.],
	defaultdomain="$enableval",
	[])
AC_SUBST(defaultdomain)
AC_DEFINE_UNQUOTED(DEFAULT_DOMAIN,"$defaultdomain",Default Domain)

moduledir=""
for f in /usr/lib64/security /usr/lib/security /usr/lib/pam
do
	if test -d $f
	then
		moduledir=$f
		break
	fi
done
AC_ARG_ENABLE([moduledir], [  --enable-moduledir=DIR specify PAM module installation path],
moduledir="$enableval"
[
	if test "$moduledir" = ""
	then
		AC_MSG_ERROR([Unable to find your pam module dir, specify --enable-moduledir.])
	fi
])
AC_DEFINE_UNQUOTED(PAM_MOD_DIR,"$moduledir",full path to PAM Module Directory)
AC_SUBST(moduledir)

log_name="indimail"
AC_ARG_ENABLE(log-name, [  --enable-log-name=indimail    set syslog name.], log_name="$enableval", )
AC_DEFINE_UNQUOTED(LOG_NAME,"$log_name",name to use for syslog)

AC_ARG_ENABLE(roaming-users, [  --enable-roaming-users=n|y    Turn on (y, default) or off (n) open relay after POP/IMAP authentication],
	POPRELAY=$enableval,
	[
		POPRELAY=yes
	])
case $POPRELAY in
1*|y*|Y*)
	POPRELAY=1
	AC_DEFINE_UNQUOTED(POP_AUTH_OPEN_RELAY,$POPRELAY,Enable POP/IMAP before SMTP)
	for f in /usr/local/bin/tcprules /usr/bin/tcprules /var/qmail/bin/tcprules
	do
		if test -f $f
		then
			tcprules_prog=$f
		fi
	done

	relay_clear_minutes="60"
	AC_ARG_ENABLE(relay-clear-minutes, [  --enable-relay-clear-minutes=60 expire time for roaming users after POP/IMAP authentication.],
    	relay_clear_minutes="$enableval", )
	AC_DEFINE_UNQUOTED(RELAY_CLEAR_MINUTES,"$relay_clear_minutes",Interval after which Relay Permission is Reset)

	AC_ARG_ENABLE(tcprules-prog, [  --enable-tcprules-prog=/var/qmail/bin/tcprules where is your tcprules program.],
	tcprules_prog="$enableval",
	[
		if test "$tcprules_prog" = ""
		then
			AC_MSG_ERROR([Unable to find your tcprules prog, specify --enable-tcprules-prog.])
		fi
	] )
	AC_DEFINE_UNQUOTED(TCPRULES_PROG,"$tcprules_prog",full path of tcprules program)

	tcpserver_file=""
	for f in "$sysconfdir"/tcp/tcp.smtp "$sysconfdir"/tcp.smtp /etc/tcp.smtp /etc/tcprules.d/qmail-smtpd
	do
	if test -f $f
	then
		tcpserver_file=$f
		break
	fi
	done

	AC_ARG_ENABLE(tcpserver_file, [  --enable-tcpserver-file=$sysconfdir/tcp/tcp.smtp  File where tcpserver -x relay information is stored.],
	tcpserver_file="$enableval",
	[
		if test "$tcpserver_file" = ""
		then
			echo "127.:allow,RELAYCLIENT=\"\"" > "$sysconfdir"/tcp/tcp.smtp
			tcpserver_file="$sysconfdir"/tcp/tcp.smtp
			AC_MSG_WARN([Unable to find your tcpserver relay file.])
			AC_MSG_WARN([Creating tcp.smtp in "$sysconfdir"/tcp/tcp.smtp.])
		fi
	])
	AC_DEFINE_UNQUOTED(TCP_FILE,"$tcpserver_file",full path of the TCP Rules File)

	open_smtp="$sysconfdir/tcp/open-smtp"
	AC_DEFINE_UNQUOTED(OPEN_SMTP,"$open_smtp","")
	;;
*)
	;;
esac
pwdhash=md5
AC_ARG_ENABLE(pwdhash, [  --enable-passwd-hash=des|md5|sha256|sha512    Passwd Hash (des - DES, md5 - MD5, sha256 - SHA256, sha512 - SHA512)],
	pwdhash=$enableval,
	[
	AC_DEFINE_UNQUOTED(DES_HASH,0,Enable DES Encryption)
	AC_DEFINE_UNQUOTED(MD5_HASH,1,Enable DES Encryption)
	AC_DEFINE_UNQUOTED(SHA256_HASH,2,Enable DES Encryption)
	AC_DEFINE_UNQUOTED(SHA512_HASH,3,Enable DES Encryption)
	])
case $pwdhash in
	des)
	AC_DEFINE_UNQUOTED(PASSWORD_HASH,0,Enable DES Encryption)
	;;
	md5)
	AC_DEFINE_UNQUOTED(PASSWORD_HASH,1,Enable MD5 Encryption)
	;;
	sha256)
	AC_DEFINE_UNQUOTED(PASSWORD_HASH,2,Enable SHA256 Encryption)
	;;
	sha512)
	AC_DEFINE_UNQUOTED(PASSWORD_HASH,3,Enable SHA512 Encryption)
	;;
esac

AC_ARG_ENABLE(logging, [  --enable-logging=e|y|n|p|v    Turn on (y) or off (n) logging to syslog, (e) only log errors, include passwords in errors (p),
	(v) verbose errors with passwords],
	ENABLE_LOGGING=$enableval,
	[
		ENABLE_LOGGING=e
	])
case $ENABLE_LOGGING in
1*|y*|Y*)
	ENABLE_LOGGING=1
	AC_DEFINE_UNQUOTED(ENABLE_LOGGING,$ENABLE_LOGGING,Enable Log Level 1)
	;;
e*|E*)
	ENABLE_LOGGING=2
	AC_DEFINE_UNQUOTED(ENABLE_LOGGING,$ENABLE_LOGGING,Enable Log Level 2)
	;;
p*|P*)
	ENABLE_LOGGING=3
	AC_DEFINE_UNQUOTED(ENABLE_LOGGING,$ENABLE_LOGGING,Enable Log Level 3)
	;;
v*|V*)
	ENABLE_LOGGING=4
	AC_DEFINE_UNQUOTED(ENABLE_LOGGING,$ENABLE_LOGGING,Enable Log Level 4)
	;;
*)
	ENABLE_LOGGING=0
	AC_DEFINE_UNQUOTED(ENABLE_LOGGING,$ENABLE_LOGGING,Disable Logging)
	;;
esac

AC_ARG_ENABLE(mysql-logging, [  --enable-mysql-logging=y|n    Turn on (y) or off (n, default) logging detailed authentication attempts],
	ENABLE_MYSQL_LOGGING=$enableval,
	[
		ENABLE_MYSQL_LOGGING=n
	])
case $ENABLE_MYSQL_LOGGING in
1*|y*|Y*)
	ENABLE_MYSQL_LOGGING=1
	AC_DEFINE_UNQUOTED(ENABLE_MYSQL_LOGGING,$ENABLE_MYSQL_LOGGING,Enable logging in MySQL)
	;;
*)
	;;
esac

AC_ARG_ENABLE(auth-logging, [  --enable-auth-logging=y|n     Turn on (y, default) or off (n) logging authentication attempts],
	ENABLE_AUTH_LOGGING=$enableval,
	[
		ENABLE_AUTH_LOGGING=y
	])
case $ENABLE_AUTH_LOGGING in
1*|y*|Y*)
	ENABLE_AUTH_LOGGING=1
	AC_DEFINE_UNQUOTED(ENABLE_AUTH_LOGGING,$ENABLE_AUTH_LOGGING,Enable logging of Authentication time)
	;;
*)
	;;
esac

AC_ARG_ENABLE(hardquota,    [  --enable-hardquota=y|n        Turn on (y) or off (n, default) Hard quota],
	ENABLE_HARD_QUOTA=$enableval,
	[
		ENABLE_HARD_QUOTA=0
	])
case $ENABLE_HARD_QUOTA in
0*|n*|N*)
	;;
*)
	AC_DEFINE_UNQUOTED(HARD_QUOTA,"$ENABLE_HARD_QUOTA",Enable Hard Quotas)
	;;
esac

AC_ARG_ENABLE(maildir-quota,[  --enable-maildir-quota=y|n    Turn on (y, default) or off (n) support for maildir++ quota],
	USE_MAILDIRQUOTA=$enableval,
	[
		USE_MAILDIRQUOTA=y
	])
case $USE_MAILDIRQUOTA in
1*|y*|Y*)
	USE_MAILDIRQUOTA=1
	AC_DEFINE_UNQUOTED(USE_MAILDIRQUOTA,$USE_MAILDIRQUOTA,Enable Maildir Quota)
	;;
*)
esac

AC_ARG_ENABLE(large-site,   [  --enable-large-site=n|y       Turn on (y) or off (n, default) tune for large numbers of users per domain],
	LARGE_SITE=$enableval,
	[
		LARGE_SITE=no
	])
case $LARGE_SITE in
1*|y*|Y*)
	SITE_SIZE=1
	AC_DEFINE_UNQUOTED(SITE_SIZE,$SITE_SIZE,Enable Large Site)
	;;
*)
	SITE_SIZE=0
	AC_DEFINE_UNQUOTED(SITE_SIZE,$SITE_SIZE,Enable Small Site)
	;;
esac

AC_ARG_ENABLE(user-cluster, [  --enable-user-cluster=y|n     Turn on (y, default) or off (n) user cluster architecture],
	CLUSTERED_SITE=$enableval,
	[
	CLUSTERED_SITE=y
	])
case $CLUSTERED_SITE in
1*|y*|Y*)
    if test $SITE_SIZE -eq 1
	then
		AC_MSG_ERROR([--enable-large-site=y is incompatible with --enable-user-cluster=y.])
	else
	CLUSTERED_SITE=1
	AC_DEFINE_UNQUOTED(CLUSTERED_SITE,$CLUSTERED_SITE,Enable Clustering)
	fi
	;;
*)
	;;
esac

AC_ARG_ENABLE(qmail-ext,    [  --enable-qmail-ext=y|n        Turn on (y, default) or off (n) qmail email address extensions],
	QMAIL_EXT=$enableval,
	[
		QMAIL_EXT=y
	])
case $QMAIL_EXT in
1*|y*|Y*)
	QMAIL_EXT=1
	AC_DEFINE_UNQUOTED(QMAIL_EXT,$QMAIL_EXT,Enable qmail extension addresses)
	;;
*)
	;;
esac

AC_ARG_ENABLE(ip-alias-domains, [  --enable-ip-alias-domains=y|n Turn on (y) or off (n, default) virutal domain lookup via reverse ip address lookup for virtual domains],
	IPALIAS=$enableval,
	[
	IPALIAS=no
	])
case $IPALIAS in
1*|y*|Y*)
	IPALIAS=1
	AC_DEFINE_UNQUOTED(IP_ALIAS_DOMAINS,$IPALIAS,Enable IP Alias Domains)
	;;
*)
	;;
esac

AC_ARG_ENABLE(valias,           [  --enable-valias=n|y           Turn on (y, default) or off (n) enable mysql alias processing],
	VALIAS=$enableval,
	[
		VALIAS=y
	])
case $VALIAS in
1*|y*|Y*)
	VALIAS=1
	AC_DEFINE_UNQUOTED(VALIAS,$VALIAS,Enable Valias Mechanism)
	;;
*)
	;;
esac

AC_ARG_ENABLE(vfilter,          [  --enable-vfilter=n|y          Turn on (y) or off (n, default) enable mysql vfilter processing],
	VFILTER=$enableval,
	[
		VFILTER=n
	])
case $VFILTER in
1*|y*|Y*)
	VFILTER=1
	AC_DEFINE_UNQUOTED(VFILTER,$VFILTER,Enable Vfilters)
	;;
*)
	;;
esac

AC_ARG_ENABLE(file-locking, [  --enable-file-locking=n|y|s|l|f|c  Turn on (y, default) or off (n),
	(s) use semaphores, (l) use link, (f) use flock, (c) use fcntl default],
	FILE_LOCKING=$enableval,
	[
		FILE_LOCKING=y
		USE_FLOCK=0
		USE_FCNTLLOCK=1
		USE_LINKLOCK=0
		USE_SEMAPHORES=0
	])
case $FILE_LOCKING in
1*|y*|Y*|c*|C*)
	FILE_LOCKING=c
	USE_FLOCK=0
	USE_FCNTLLOCK=1
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_FCNTLLOCK,$USE_FCNTLLOCK,Use fcntl for file locking)
	;;
f*|F*)
	FILE_LOCKING=f
	USE_FLOCK=1
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_FLOCK,$USE_FLOCK,Use flock for file locking)
	;;
l*|L*)
	FILE_LOCKING=l
	USE_FLOCK=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=1
	USE_SEMAPHORES=0
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_LINKLOCK,$USE_LINKLOCK,Use link for file locking)
	;;
s*|S*)
	FILE_LOCKING=s
	USE_FLOCK=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=1
	AC_DEFINE_UNQUOTED(FILE_LOCKING,$FILE_LOCKING,Enable File Locking Code)
	AC_DEFINE_UNQUOTED(USE_SEMAPHORES,$USE_SEMAPHORES,Use semaphores for file locking)
	;;
*)
	FILE_LOCKING=0
	USE_FLOCK=0
	USE_FCNTLLOCK=0
	USE_LINKLOCK=0
	USE_SEMAPHORES=0
	;;
esac

AC_ARG_ENABLE(file-sync,        [  --enable-file-sync=y|n        Turn on (y, default) or off (n) file sync in mail delivery],
	FILE_SYNC=$enableval,
	[
		FILE_SYNC=y
	])
case $FILE_SYNC in
1*|y*|Y*)
	FILE_SYNC=1
	AC_DEFINE_UNQUOTED(FILE_SYNC,$FILE_SYNC,Enable fsync when writing mail to disk)
	;;
*)
	;;
esac

AC_ARG_ENABLE(make-seekable,    [  --enable-make-seekable=n|y    Turn on (y, default) or off (n) Make input to vdelivermail seekable by lseek/fseek],
	MAKE_SEEKABLE=$enableval,
	[
		MAKE_SEEKABLE=y
	])
case $MAKE_SEEKABLE in
1*|y*|Y*)
	MAKE_SEEKABLE=1
	AC_DEFINE_UNQUOTED(MAKE_SEEKABLE,$MAKE_SEEKABLE,Enable making seekable pipes)
	;;
*)
	;;
esac

AC_ARG_ENABLE(random-balancing, [  --enable-random-balancing=y|n Turn on (y) or off (n, default) Random Balancing of inquery queries],
	RANDOM_BALANCING=$enableval,
	[
		RANDOM_BALANCING=n
	])
case $RANDOM_BALANCING in
1*|y*|Y*)
	RANDOM_BALANCING=1
	AC_DEFINE_UNQUOTED(RANDOM_BALANCING,$RANDOM_BALANCING,Enable Random Loadbalancing of inquery)
	;;
*)
	;;
esac

AC_ARG_ENABLE(query-cache,     [  --enable-query-cache=y|n     Turn on (y) or off (n, default) Query Caching for authentication],
	QUERY_CACHE=$enableval,
	[
		QUERY_CACHE=n
	])
case $QUERY_CACHE in
1*|y*|Y*)
	QUERY_CACHE=1
	AC_DEFINE_UNQUOTED(QUERY_CACHE,$QUERY_CACHE,Enable Query Caching for authentication)
	;;
*)
	;;
esac

AC_ARG_ENABLE(domain-limits,    [  --enable-domain-limits=y|n    Turn on (y, default) or off (n) handling of Domain Limits],
	ENABLE_DOMAIN_LIMITS=$enableval,
	[
		ENABLE_DOMAIN_LIMITS=y
	])
case $ENABLE_DOMAIN_LIMITS in
1*|y*|Y*)
	ENABLE_DOMAIN_LIMITS=1
	AC_DEFINE_UNQUOTED(ENABLE_DOMAIN_LIMITS,$ENABLE_DOMAIN_LIMITS,Enable domain level limits)
	;;
*)
	;;
esac

AC_DEFINE_UNQUOTED(USE_MYSQL,$USE_MYSQL,Use MySQL for primary authentication)

AC_ARG_ENABLE(mysqlprefix, [  --enable-mysqlprefix=""       MySQL Prefix Directory.],[mysql_prefix=$enableval],[mysql_prefix=""])

if test "$mysql_prefix" = ""
then
mysql_incdir=""
for f in /usr/local/mysql/include /usr/include/mysql
do
	if test -d $f
	then
		mysql_incdir=$f
		break
	fi
done

AC_ARG_ENABLE(mysqlincdir, [  --enable-mysqlincdir=""       directory where auth include files are.],
mysql_incdir="$enableval",
[
	if test "$mysql_incdir" = ""
	then
		AC_MSG_ERROR([Unable to find your inc dir, specify --enable-mysqlincdir.])
	fi
])


mysql_libdir=""
for f in /usr/local/mysql/lib64/mysql /usr/local/mysql/lib64 /usr/local/mysql/lib/mysql /usr/local/mysql/lib /usr/lib/mysql
do
	if test -d $f
	then
		mysql_libdir=$f
		break
	fi
done
AC_ARG_ENABLE(mysqllibdir, [  --enable-mysqllibdir=path-to-auth-libs directory where auth libs are.],
mysql_libdir="$enableval",
[
	if test "$mysql_libdir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql libraries dir, specify --enable-mysqllibdir.])
	fi
])

mysqlbindir=""
for f in /usr/local/mysql/bin/mysql /usr/bin/mysql
do
if test -f $f
then
	mysqlbindir="`dirname $f`"
	break
	fi
done

AC_ARG_ENABLE(mysqlbindir, [  --enable-mysqlbindir=""       directory where mysql binary are.],
mysqlbindir="$enableval",
[
	if test "$mysqlbindir" = ""
	then
		AC_MSG_ERROR([Unable to find your sql bin dir, specify --enable-mysqlbindir.])
	fi
])
mysql_prefix=`dirname $mysqlbindir`
else
	for f in $mysql_prefix/include/mysql.h $mysql_prefix/include/mysql/mysql.h
	do
		if test -f $f
		then
			mysql_incdir=`dirname $f`
			break
		fi
	done

	if test -d $mysql_prefix/lib64/mysql
	then
		mysql_libdir=$mysql_prefix/lib64/mysql
	elif test -d $mysql_prefix/lib64
	then
		mysql_libdir=$mysql_prefix/lib64
	elif test -d $mysql_prefix/lib/mysql
	then
		mysql_libdir=$mysql_prefix/lib/mysql
	elif test -d $mysql_prefix/lib
	then
		mysql_libdir=$mysql_prefix/lib
	else
		mysql_libdir=$mysql_prefix/lib/mysql
	fi
	mysqlbindir=$mysql_prefix/bin
fi
egrep MYSQL_OPT_LOCAL_INFILE $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_LOCAL_INFILE, 1, "local infile support")
fi
egrep MYSQL_OPT_SSL_VERIFY_SERVER_CERT $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_VERIFY_SERVER_CERT, 1, "verify server cert")
fi
egrep MYSQL_OPT_SSL_KEY $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_KEY, 1, "ssl key")
fi
egrep MYSQL_OPT_SSL_CERT $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CERT, 1, "ssl cert")
fi
egrep MYSQL_OPT_SSL_CA $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CA, 1, "ssl ca")
fi
egrep MYSQL_OPT_SSL_CAPATH $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CAPATH, 1, "ssl capath")
fi
egrep MYSQL_OPT_SSL_CIPHER $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CIPHER, 1, "ssl cipher")
fi
egrep MYSQL_OPT_SSL_CRL $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CRL, 1, "ssl cert revocation list")
fi
egrep MYSQL_OPT_SSL_CRLPATH $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_CRLPATH, 1, "ssl cert revocation list path")
fi
egrep MYSQL_OPT_SSL_ENFORCE $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_ENFORCE, 1, "enforce ssl/tls")
fi
egrep MYSQL_OPT_TLS_VERSION $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_TLS_VERSION, 1, "tls version")
fi
egrep MYSQL_OPT_SSL_MODE $mysql_incdir/mysql.h > /dev/null 2>&1
if test $? = 0 ; then
	AC_DEFINE(HAVE_MYSQL_OPT_SSL_MODE, 1, "ssl mode")
fi

saveLIBS="$LIBS"
AC_CHECK_LIB(mysqlclient -L${mysql_libdir}, mysql_real_connect,libs="$libs -lmysqlclient",
	AC_MSG_ERROR([could not find libmysqlclient which is needed for MySQL support]),)
AC_CHECK_FUNC(mysql_optionsv, have_mysqloptionsv=1)
LIBS=$saveLIBS

AC_DEFINE_UNQUOTED(MYSQLBINDIR,"$mysqlbindir",full path to MySQL binaries)

auth_inc="-I$indimaildir/include -I$mysql_incdir -I$mysql_incdir/mysql"
case "$host" in
	*-*-darwin*)
	auth_libs="-L. -L$prefix/lib -lindimail -L$mysql_libdir -lmysqlclient $LIB_SSL $LIB_CRYPT"
	;;
	*)
	auth_libs="-L. -L$ilibdir -lindimail -L$mysql_libdir -lmysqlclient $LIB_SSL $LIB_CRYPT"
	;;
esac

AC_ARG_ENABLE(HAVE_MD5_CRYPT,    [  --enable-md5-crypt            if your system crypt does not support md5],
	[with_md5crypt=$enableval],
	[with_md5crypt=no])
test "$with_md5crypt" = "no" && AC_DEFINE(HAVE_MD5_CRYPT,1,Define if you want md5 crypt support compiled in)
AM_CONDITIONAL(HAVE_MD5_CRYPT, test "$with_md5crypt" = yes)

AC_ARG_ENABLE(HAVE_SHA256_CRYPT, [  --enable-sha256-crypt         if your system does not support sha256],
	[with_sha256crypt=$enableval],
	[with_sha256crypt=no])
test "$with_sha256crypt" = "no" && AC_DEFINE(HAVE_SHA256_CRYPT,1,Define if you want sha256 crypt support compiled in)
AM_CONDITIONAL(HAVE_SHA256_CRYPT, test "$with_sha256crypt" = yes)

AC_ARG_ENABLE(HAVE_SHA512_CRYPT, [  --enable-sha512-crypt         if your system does not support sha512],
	[with_sha512crypt=$enableval],
	[with_sha512crypt=no])
test "$with_sha512crypt" = "no" && AC_DEFINE(HAVE_SHA512_CRYPT,1,Define if you want sha512 crypt support compiled in)
AM_CONDITIONAL(HAVE_SHA512_CRYPT, test "$with_sha512crypt" = yes)

HOST=$host
AC_SUBST(subdir_list)
AC_SUBST(mysql_prefix)
AC_SUBST(mysql_incdir)
AC_SUBST(mysql_libdir)
AC_SUBST(mysqlbindir)
AC_SUBST(auth_inc)
AC_SUBST(CFLAGS)
AC_SUBST(auth_libs)
AC_SUBST(idcommand)
AC_SUBST(MODFLAGS)
AC_SUBST(HOST)

AC_CHECK_LIB(m,floor)
AC_CHECK_LIB(z,compress)

AC_ARG_ENABLE(passwd,   [  --enable-passwd=y|n           Turn on (y, default) or off (n) /etc/passwd (or shadow) authentication],
	ENABLE_PASSWD=$enableval,
	[
		ENABLE_PASSWD=y
	])
case $ENABLE_PASSWD in
1*|y*|Y*)
	ENABLE_PASSWD=1
	AC_CHECK_HEADERS(shadow.h)
	HAS_SHADOW_H=0
	AC_CHECK_HEADER(shadow.h,
	[
		HAS_SHADOW_H=1;
	])
	if test $HAS_SHADOW_H = 1
	then
		AC_CHECK_LIB(shadow, setspent)
	fi
	AC_DEFINE_UNQUOTED(ENABLE_PASSWD,$ENABLE_PASSWD,enable system passwds)
	;;
*)
	ENABLE_PASSWD=0
	;;
esac

if test -f /etc/shadow
then
	HAS_SHADOW=1
	AC_DEFINE_UNQUOTED(HAS_SHADOW,$HAS_SHADOW,/etc/shadow present)
fi

#
# pgconfig
#

AC_MSG_CHECKING(for pgconfig)

pgconfig=""

for f in /usr/local/pgsql/bin/pg_config /usr/local/bin/pg_config /usr/bin/pg_config
do
   if test -x $f; then
	  pgconfig=$f
	  AC_MSG_RESULT($pgconfig)
	  break
   fi
done

if test "$pgconfig" != ""; then
	AC_DEFINE_UNQUOTED(HAVE_PGSQL,$HAVE_PGSQL,Use PGSQL for authentication)
	HAVE_PGSQL=1
	auth_pg_libs="-L`$pgconfig --libdir` -lpq "
	AC_MSG_RESULT($auth_pg_libs)
	AC_SUBST(auth_pg_libs)
	auth_pg_inc="-I`$pgconfig --includedir`"
	AC_MSG_RESULT($auth_pg_inc)
	AC_SUBST(auth_pg_inc)
fi

###	use option --enable-mysql-escape to include the mysql-escape-string support
AC_ARG_ENABLE(mysql_escape,   [  --enable-mysql-escape=yes|no Turn on (y, default) or off (n) mysql escape string],
	mysql_escape=$enableval,
	[
		mysql_escape=y
	])
case $mysql_escape in
1*|y*|Y*)
	mysql_escape="yes"
	;;
*)
	mysql_escape="no"
	;;
esac
test "$mysql_escape" = "yes" && AC_DEFINE_UNQUOTED(mysql_query(x,y),_mysql_Query(x,y),mysql_real_escape_string)
case $have_mysqloptionsv in
	1*|y*|Y*)
	AC_DEFINE_UNQUOTED(HAVE_MYSQL_OPTIONSV,$have_mysqloptionsv, "Enable mysql_optionsv()")
	;;
esac

### turn on enterprise support to enable support from IndiMail Support <manvendra@indimail.org>
AC_ARG_ENABLE(enterprise-support, [  --enable-enterprise-support=y|n     Turn on (y) or (n, default) Turn on Enterprise Messaging for Support],
	ENABLE_ENTERPRISE=$enableval,
	[
		ENABLE_ENTERPRISE=n
	])
case $ENABLE_ENTERPRISE in
1*|y*|Y*)
	ENABLE_ENTERPRISE=1
	AC_DEFINE_UNQUOTED(ENABLE_ENTERPRISE,$ENABLE_ENTERPRISE,Enable Enterprise Messaging for Support)
	;;
*)
	;;
esac

dnl **********************************************************************
dnl Checks for IPv6
dnl **********************************************************************
dnl
dnl We test for IPv6 by checking, in turn, for availability of
dnl getaddrinfo, presence of the INET6 address/protocol family, and
dnl the existence of struct sockaddr_in6.  If any of them is missing,
dnl IPv6 is disabled, and the code reverts to old-style gethostbyname.
dnl
dnl If --enable-ipv6 is explicitly specified on the configure command
dnl line, we check for IPv6 and abort if not found.  If --disable-ipv6
dnl is specified, we disable IPv6 and don't check for it.  The default
dnl is to autodetect IPv6 and use it where available.
dnl

AC_ARG_ENABLE(ipv6,
  AC_HELP_STRING([--disable-ipv6],[disable IPv6 support]),
  [case "${enable_ipv6}" in
    n|no)
      AC_MSG_RESULT([disabling IPv6 at user request])
      dnl Disable IPv6 checking
      ipv6=no
      ;;
    y|yes)
      dnl IPv6 explicitly enabled: force its use (abort if unavailable).
      ipv6=yes
      force_ipv6=yes
      ;;
    auto)
      dnl Auto-detect IPv6, i.e. check for IPv6, but don't force it.
      ipv6=yes
      ;;
    *)
      AC_MSG_ERROR([Invalid --enable-ipv6 argument \`$enable_ipv6'])
      ;;
    esac
  ], [
    dnl If nothing is specified, assume auto-detection.
    ipv6=yes
  ]
)

if test "X$ipv6" = "Xyes"; then
  AC_CHECK_FUNCS(getaddrinfo, [], [
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support getaddrinfo(3)])
    ipv6=no
  ])
fi

if test "X$ipv6" = "Xyes"; then
  PROTO_INET6([], [
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support the PF_INET6 protocol family])
    ipv6=no
  ])
fi

if test "X$ipv6" = "Xyes"; then
  TYPE_STRUCT_SOCKADDR_IN6([],[
    AC_MSG_RESULT([Disabling IPv6 support: your system does not support \`struct sockaddr_in6'])
    ipv6=no
  ])
  if test "X$ipv6" = "Xyes"; then
    INDIMAIL_STRUCT_SOCKADDR_STORAGE
    MEMBER_SIN6_SCOPE_ID
  fi
fi

if test "X$ipv6" = "Xyes"; then
  AC_DEFINE([ENABLE_IPV6], 1, [Define if IPv6 support is enabled.])
  AC_MSG_RESULT([Enabling support for IPv6.])
elif test "x$force_ipv6" = "xyes"; then
  AC_MSG_ERROR([IPv6 support requested but not found; aborting])
fi

case "$host" in
*-*-sunos4.1.1*)
PS_COMMAND="ps -aux"
;;
*-*-solaris*)
PS_COMMAND="ps -ef"
;;
*-*-sunos*)
PS_COMMAND="ps -ef"
;;
*-sgi-5*)
PS_COMMAND="ps -ef"
;;
*-ibm-aix*)
PS_COMMAND="ps -ef"
;;
mips-dec-mach3*)
PS_COMMAND="ps -ef"
;;
*-dec-ultrix*)
PS_COMMAND="ps -ef"
;;
*-*-hpux*)
PS_COMMAND="ps -eaf"
;;
alpha-dec-osf*)
PS_COMMAND="ps -ef"
;;
*-*-nextstep*)
PS_COMMAND="ps -ef"
;;
i*86-*-bsdi*)
PS_COMMAND="ps ax"
;;
i*86-unknown-bsd*)
PS_COMMAND="ps -ef"
;;
m68k-sony-newsos*)
PS_COMMAND="ps -ef"
;;
mips-sony-bsd*)
PS_COMMAND="ps -ef"
;;
m68k-apple-aux*)
PS_COMMAND="ps -ef"
;;
m88k-dg-dgux*)
PS_COMMAND="ps -ef"
;;
t3e-*-*)
PS_COMMAND="ps -ef"
;;
*-cray-unicos*)
PS_COMMAND="ps -ef"
;;
*-*-sysv4*)
PS_COMMAND="ps -ef"
;;
*-*-machten*)
PS_COMMAND="ps -ef"
;;
i*86-unknown-sco*)
PS_COMMAND="ps -ef"
;;
*-convex-bsd*)
PS_COMMAND="ps -ef"
;;
*-sequent-ptx*)
PS_COMMAND="ps -ef"
;;
*-sequent-dynix*|*-sequent-ptx*)
PS_COMMAND="ps -ef"
;;
*-*-freebsd*)
PS_COMMAND="ps aux"
;;
*-*-netbsd*)
PS_COMMAND="ps aux"
;;
*-*-openbsd*)
PS_COMMAND="ps -aux"
;;
*-*-IRIX*)
PS_COMMAND="ps -ef"
;;
*)
PS_COMMAND="ps -ef"
;;
esac
AC_DEFINE_UNQUOTED(PS_COMMAND,"$PS_COMMAND",ps command)

# Output files
# Rename the template .pc file to include the API version on configure
AC_SUBST(libdir)
AC_CONFIG_FILES([libindimail.pc:libindimail.pc.in],[], [])
AC_CONFIG_FILES([indimail-release:indimail-release.in],[], [])

AC_OUTPUT(Makefile)

(
echo "---------- $PACKAGE $VERSION $target ----------"
echo "Date                 = $DATE"
echo "Target CPU           = $target_cpu"
echo "Target Vendor        = $target_vendor"
echo "Target OS            = $target_os"
echo "Host                 = $host"
echo "DISTRIB_ID           = $DISTRIB_ID"
echo "DISTRIB_VERSION      = $DISTRIB_VERSION"
echo "ID command           = $idcommand"
echo "CPPFLAGS             = $CPPFLAGS"
echo "CXXFLAGS             = $CXXFLAGS"
echo "CFLAGS               = $CFLAGS"
echo "LDFLAGS              = $LDFLAGS"
echo "MODFLAGS             = $MODFLAGS"
echo "            Current settings"
echo "-------------------------------------------------------------"
echo "   qmail directory = $qmaildir --enable-qmaildir=$qmaildir"
echo " control directory = $controldir --enable-controldir=$controldir"
echo " domains directory = $domaindir --enable-domaindir=$domaindir"
echo "  shared directory = $shareddir --enable-shareddir=$shareddir"
echo "        sysconfdir = $q_sysconfdir"
echo "        servicedir = $servicedir"
if test "$postfixdir" != ""
then
	echo " Postfix directory = $postfixdir --enable-postfixdir=$postfixdir"
fi
echo "    Mail Base Path = $basepath  --enable-basepath=$basepath"
echo "Supervise Log Path = $logdir    --enable-logdir=$logdir"
echo "    indimail  user = $indiuser  --enable-indiuser=$indiuser"
echo "    indimail group = $indigroup --enable-indigroup=$indigroup"
echo "indimail directory = $indimaildir"
echo "    indimail   uid = $indimailuid"
echo "    indimail   gid = $indimailgid"
echo "    default domain = $defaultdomain --enable-default-domain=$defaultdomain"
echo "   syslog log name = $log_name --enable-log-name=$log_name"
if test "$mysql_prefix" != ""
then
echo "      MySQL Prefix = $mysql_prefix"
fi
echo "MySQL bin directory= $mysqlbindir --enable-mysqlbindir=$mysqlbindir"
echo "       include dir = $mysql_incdir --enable-mysqlincdir=$mysql_incdir"
echo "     MySQL lib dir = $mysql_libdir --enable-mysqllibdir=$mysql_libdir"
echo "         libraries = $libs"
echo "          auth inc = $auth_inc"
echo "          auth lib = $auth_libs"
echo "    acl_libdirstem = $acl_libdirstem"
case $mysql_escape in
	1*|y*|Y*)
	echo "      mysql escape = enabled"
	;;
	2|n*|N*)
	echo "      mysql escape = disabled"
	;;
esac
case $have_mysqloptionsv in
	1*|y*|Y*)
	echo "  mysql_optionsv() = enabled"
	;;
esac
case $POPRELAY in
1*|y*|Y*)
	echo "     roaming users = ON --enable-roaming-users=y default"
	echo "  relay clear mins = $relay_clear_minutes --enable-relay-clear-minutes=$relay_clear_minutes"
	echo "    tcprules  prog = $tcprules_prog --enable-tcprules-prog=$tcprules_prog"
	echo "    tcpserver file = $tcpserver_file --enable-tcpserver-file=$tcpserver_file"
	echo "    open_smtp file = $open_smtp"
	;;
*)
	echo "     roaming users = OFF --enable-roaming-users=n"
	;;
esac

case $ENABLE_LOGGING in
1*|y*|Y*)
	echo "            syslog = show successful and failure login attempts --enable-logging=y"
	;;
2*|e*|E*)
	echo "            syslog = show only failure attempts --enable-logging=e default"
	;;
3*|p*|P*)
	echo "            syslog = show failure attempts with clear text password --enable-logging=p"
	;;
4*|v*|V*)
	echo "            syslog = log everything, including passwords in errors --enable-logging=v"
	;;
*)
	echo "            syslog = OFF"
	;;
esac

case $ENABLE_MYSQL_LOGGING in
1*|y*|Y*)
	echo "     mysql logging = ON  --enable-mysql-logging=y"
	;;
*)
	echo "     mysql logging = OFF --enable-mysql-logging=n default"
	;;
esac

case $ENABLE_AUTH_LOGGING in
1*|y*|Y*)
	echo "      auth logging = ON  --enable-auth-logging=y default"
	;;
*)
	echo "      auth logging = OFF --enable-auth-logging=n"
	;;
esac

case $ENABLE_HARD_QUOTA in
0*|n*|N*)
	echo "        user quota = OFF --enable-hardquota=n default"
	;;
*)
	echo "        user quota = $ENABLE_HARD_QUOTA --enable-hardquota=$ENABLE_HARD_QUOTA"
	;;
esac

case $USE_MAILDIRQUOTA in
1*|y*|Y*)
	echo "   maildir++ quota = ON  --enable-maildir-quota=y default"
	;;
*)
	echo "   maildir++ quota = OFF --enable-maildir-quota=n"
	;;
esac

case $LARGE_SITE in
1*|y*|Y*)
	echo "        Large Site = ON  --enable-large-site=y (multiple tables)"
	;;
*)
	echo "        Large Site = OFF --enable-large-site=n (indimail table) default"
	;;
esac

case $CLUSTERED_SITE in
1*|y*|Y*)
	echo "  Clustered   Arch = ON  --enable-user-cluster=y default"
	;;
*)
	echo "  Clustered   Arch = OFF --enable-user-cluster=n"
	;;
esac

case $QMAIL_EXT in
1*|y*|Y*)
	echo "address extentions = ON  --enable-qmail-ext=y default"
	;;
*)
	echo "address extentions = OFF --enable-qmail-ext=n"
	;;
esac

case $IPALIAS in
1*|y*|Y*)
	echo "          ip alias = ON  --enable-ip-alias-domains=y"
	;;
*)
	echo "          ip alias = OFF --enable-ip-alias-domains=n (default)"
	;;
esac

case $VALIAS in
1*|y*|Y*)
echo " valias processing = ON  --enable-valias=y default"
      ;;
*)
echo " valias processing = OFF --enable-valias=n"
	  ;;
esac

case $VFILTER in
1*|y*|Y*)
echo "vfilter processing = ON  --enable-vfilter=y"
      ;;
*)
echo "vfilter processing = OFF --enable-vfilter=n default"
	  ;;
esac

case $FILE_LOCKING in
1*|y*|Y*|c*|C*)
	echo "     fcntl locking = ON  --enable-file-locking=y default (use fcntl)"
	;;
f*|F*)
	echo "     flock locking = ON  --enable-file-locking=f (use flock)"
	;;
l*|L*)
	echo "      link locking = ON  --enable-file-locking=l (use link)"
	;;
s*|S*)
	echo " semaphore locking = ON  --enable-file-locking=s (use semaphore)"
	;;
*)
	echo "      file locking = OFF --enable-file-locking=n"
	;;
esac

case $FILE_SYNC in
1*|y*|Y*)
	echo "         file sync = ON  --enable-file-sync=y default"
	;;
*)
	echo "         file sync = OFF --enable-file-sync=n disable vdelivermail fsync"
	;;
esac

case $MAKE_SEEKABLE in
1*|y*|Y*)
	echo "make input seekable= ON  --enable-make-seekable=y default"
	;;
*)
	echo "make input seekable= OFF --enable-make-seekable=n"
	;;
esac

case $RANDOM_BALANCING in
1*|y*|Y*)
	echo "random     balance = ON  --enable-random-balancing=y"
	;;
*)
	echo "time slice balance = ON  --enable-random-balancing=n default"
	;;
esac

case $QUERY_CACHE in
1*|y*|Y*)
	echo "       Query cache = ON  --enable-query-cache=y"
	;;
*)
	echo "       Query cache = OFF --enable-query-cache=n default"
	;;
esac

case $ENABLE_PASSWD in
1*|y*|Y*)
echo "  system passwords = ON  --enable-passwd=y default"
      ;;
*)
echo "  system passwords = OFF --enable-passwd=n"
	  ;;
esac

case $ENABLE_DOMAIN_LIMITS in
1*|y*|Y*)
echo "     domain limits = ON  --enable-domain-limits=y default"
      ;;
*)
echo "     domain limits = OFF --enable-domain-limits=n"
	  ;;
esac
case $pwdhash in
	des)
echo "     Password Hash = DES --enable-password-hash=des"
	;;
	md5)
echo "     Password Hash = MD5 --enable-password-hash=md5"
	;;
	sha256)
echo "     Password Hash = SHA256 --enable-password-hash=sha256"
	;;
	sha512)
echo "     Password Hash = SHA512 --enable-password-hash=sha512"
	;;
esac

case $ipv6 in
1*|y*|Y*)
echo "      IPV6 Support = ON  --enable-ipv6=y default"
;;
*)
echo "      IPV6 Support = OFF --enable-ipv6=n"
AC_CHECK_LIB(nsl, gethostbyname,echo -lnsl >> socket.lib,,)
;;
esac

case $with_md5crypt in
1*|y*|Y*)
echo "         MD5 Crypt = ON  --enable-md5-crypt=y"
;;
*)
echo "         MD5 Crypt = OFF --enable-md5-crypt=n default"
;;
esac

case $with_sha256crypt in
1*|y*|Y*)
echo "      SHA256 Crypt = ON  --enable-sha256-crypt=y"
;;
*)
echo "      SHA256 Crypt = OFF --enable-sha256-crypt=n default"
;;
esac

case $with_sha512crypt in
1*|y*|Y*)
echo "      SHA512 Crypt = ON  --enable-sha512-crypt=y"
;;
*)
echo "      SHA512 Crypt = OFF --enable-sha512-crypt=n default"
;;
esac
case $ENABLE_ENTERPRISE in
1*|y*|Y*)
echo "      Enterprise   = ON  --enable-enterprise-support=y"
;;
*)
echo "      Enterprise   = OFF --enable-enterprise-support=n default"
;;
esac
echo "-------------------------------------------------------------"
echo "Additonal Components = $subdir_list"
if test "$missing_list" != ""
then
	clear
	echo
	echo
	echo "============ WARNING WARNING WARNING WARNING ==============="
	echo "Missing   Components = $missing_list"
	echo
	echo "Download and extract the following additional component(s)"
	echo
	if ! test -d ../flash-0.9.4
	then
		echo "http://downloads.sourceforge.net/indimail/flash-0.9.4.tar.gz"
	fi
	if ! test -d ../altermime-0.3.10
	then
		echo "http://downloads.sourceforge.net/indimail/altermime-0.3.10.tar.gz"
	fi
	if ! test -d ../ripmime-1.4.0.10
	then
		echo "http://downloads.sourceforge.net/indimail/ripmime-1.4.0.10.tar.gz"
	fi
	if ! test -d ../mpack-1.6
	then
		echo "http://downloads.sourceforge.net/indimail/mpack-1.6.tar.gz"
	fi
	if ! test -d ../fortune-1.1
	then
		echo "http://downloads.sourceforge.net/indimail/fortune-1.1.tar.gz"
	fi
	if ! test -d ../pam-multi-1.1
	then
		echo "http://downloads.sourceforge.net/indimail/pam-multi-1.1.tar.gz"
	fi
	echo
	echo "in the directory `dirname $PWD`"
	echo
	sleep 20
else
	echo "CONGRATULATIONS!! All IndiMail components found"
fi
) | tee indimail.settings
