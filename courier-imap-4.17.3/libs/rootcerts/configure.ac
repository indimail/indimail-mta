dnl
dnl Copyright 2001-2008 Double Precision, Inc.  See COPYING for
dnl distribution information.

AC_INIT(rootcerts, 0.50, [courier-users@lists.sourceforge.net])

>confdefs.h  # Kill PACKAGE_ macros

AC_CONFIG_SRCDIR(rootcertsdir.cnf.in)
AC_CONFIG_AUX_DIR(../..)
AM_INIT_AUTOMAKE([foreign no-define])
LPATH="$PATH:/usr/local/bin"

AC_PROG_INSTALL
AC_PROG_LN_S

test "x$prefix" = xNONE && prefix=$ac_default_prefix
test "x$exec_prefix" = xNONE && exec_prefix='${prefix}'
eval "datarootdir=$datarootdir"
eval "datadir=$datadir"

certdb=""

AC_ARG_WITH(certdb, [  --with-certdb=/pathname     Use external root certificate database in pathname], [
		case $withval in
		/*)
			certdb="$withval"
			;;
		*)
			AC_MSG_ERROR(--with-certdb requires an absolute pathname)
			;;
		esac
], [

for f in /etc/ssl/certs /etc/ssl/cert.pem /etc/pki/tls/cert.pem
do
	if test -f $f -o -d $f
	then
		certdb=$f
	fi
done

if test "$certdb" = ""
then
	certdb="/etc/ssl/cert.pem"

	AC_MSG_WARN([Cannot find known locations of CA certs, using $certdb])
fi
 ])

AC_SUBST(certdb)

AC_OUTPUT(Makefile rootcertsdir.cnf)
