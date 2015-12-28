dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.59])
AC_INIT([flash],0.9.4,[Manvendra Bhangui manvendra@indimail.org],indimail, [http://www.indimail.org])
PACKAGE=$PACKAGE_NAME
VERSION=$PACKAGE_VERSION
LT_INIT
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_CC
AC_PROG_LIBTOOL
AC_LIB_PREPARE_MULTILIB

LIBVER=0:0:0
defaultprefix="/var/indimail"

if test x$prefix = xNONE
then
	prefix="$defaultprefix"
	ac_configure_args="$ac_configure_args -prefix=$prefix"
fi

AC_ISC_POSIX

case "$host" in
*-*-darwin*)
	CFLAGS="$CFLAGS -g -I. -I.. -O4 -Wall -pie"
	MODFLAGS="-O4 -bundle -pie -Wall -undefined dynamic_lookup"
	STRIP_OPT=-s
	;;
*)
	CFLAGS="$CFLAGS -s -I. -I.. -O4 -Wall -fPIC"
	MODFLAGS="-shared -nostartfiles -s -O4 -fPIC -Wall"
	STRIP_OPT=-s
	;;
esac
AC_SUBST([LIBTOOL_DEPS])
AC_SUBST(defaultprefix)
AC_SUBST(LIBVER)
AC_SUBST(STRIP_OPT)
AC_SUBST(MODFLAGS)
if test -d /lib64 -a "$acl_libdirstem" = "lib"
then
	acl_libdirstem=lib64
fi
tlib='${exec_prefix}/lib'
if test x$libdir = x$tlib
then
	dnl Does not work on RHEL 5, CentOS 5 64 bit systems
	if test -d /lib64 -a "$acl_libdirstem" = "lib"
	then
		acl_libdirstem=lib64
	fi
	libdir=$prefix/$acl_libdirstem
fi
RPATH_LIB="$acl_libdirstem"

indiuser="indimail"
AC_ARG_ENABLE(indiuser, [  --enable-indiuser=indimail   user indimail was installed as.],
	indiuser="$enableval",
	[
		if test "$indiuser" = ""
		then
			AC_MSG_ERROR([Unable to find your indiuser user, specify --enable-indiuser.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIUSER,"$indiuser","")
AC_SUBST(indiuser)
echo $indiuser > indiusername

indigroup="indimail"
AC_ARG_ENABLE(indigroup, [  --enable-indigroup=indimail   group indimail was installed as.],
	indigroup="$enableval",
	[
		if test "$indigroup" = ""
		then
			AC_MSG_ERROR([Unable to find your indigroup group, specify --enable-indigroup.])
		fi
	])
AC_DEFINE_UNQUOTED(INDIGROUP,"$indigroup","")
AC_SUBST(indigroup)

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

	unlink("indimail.dir");
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
		f=fopen("indimail.dir","a+");
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
],
    AC_MSG_RESULT(yes),
    AC_MSG_RESULT(no)
    AC_MSG_ERROR(Could not find indimail user.),
    AC_MSG_ERROR(Could not compile and run even a trivial ANSI C program - check CC.))


indimaildir=`cat indimail.dir`
AC_SUBST(indimaildir)
AC_DEFINE_UNQUOTED(INDIMAILDIR,"$indimaildir","")
rm -f indimail.dir

flashmailuid=`cat indimail.uid`
rm -f indimail.uid

flashmailgid=`cat indimail.gid`
rm -f indimail.gid
rm -f indiusername 

AC_CHECK_HEADERS(crypt.h)
AC_CHECK_HEADERS(term.h)
AC_CHECK_HEADERS(ncurses/term.h)
AC_CHECK_HEADERS(ncurses/ncurses.h)
dnl Checks for libraries.
AC_CHECK_LIB(dl, main, [AC_SUBST([LIB_DLOPEN], ["-ldl"]) AC_DEFINE([HAVE_DLOPEN], [1],[dynamic link Library])],nolibdl=t,)
if test " $nolibdl" = " t"
then
   	AC_MSG_ERROR(Could not find libdl library.)
fi
AC_CHECK_LIB(ncurses, initscr, [AC_SUBST([LIB_CURSES], ["-lncurses"]) AC_DEFINE([HAVE_NCURSES], [1],[ncurses Library])],nocurses=t,)
if test " $nocurses" = " t"
then
	nocurses=""
	AC_CHECK_LIB(curses, initscr, [AC_SUBST([LIB_CURSES], ["-lcurses"]) AC_DEFINE([HAVE_CURSES], [1],[curses Library])],nocurses=t,)
	if test " $nocurses" = " t"
	then
    	AC_MSG_ERROR(Could not find curses/ncurses library.)
	fi
fi
AC_CHECK_LIB(crypt, crypt, [AC_SUBST([LIB_CRYPT], ["-lcrypt"]) AC_DEFINE([HAVE_CRYPT], [1],[crypt Library])],,)
AC_CHECK_FUNCS(setenv unsetenv)

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

AC_PROG_INSTALL

AC_ARG_ENABLE(
	maildir, 
	[  --enable-maildir        build in maildir support],
	AC_DEFINE_UNQUOTED(MAILDIR,"$MAILDIR","")
)   

AC_PATH_PROG(PGR,less)
if test "$PGR" = "" ; then 
	AC_PATH_PROG(PGR,more)
	if test "$PGR" = "" ; then
		echo "Cant find a pager in youre path!!!\n" 
	fi
fi

AC_DEFINE_UNQUOTED(PAGER,"$PGR","")
AC_DEFINE(DELIM,':',"")
AC_DEFINE(ESCAPE,'\\',"")
#'
AC_DEFINE(PATH_TO_GPL,"COPYING","")
AC_DEFINE(PATH_TO_HELP,"system.help","")
AC_DEFINE(DEFAULT_MENU,"system.menu","")

auth_libs="-L. -L$libdir -lflash"
echo "system lib dir = $acl_libdirstem $acl_libdirstem2 RPATH=$libdir"
AC_SUBST(auth_libs)
AC_OUTPUT(Makefile)
