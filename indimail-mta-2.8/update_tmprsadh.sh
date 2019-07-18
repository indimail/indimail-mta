#!/bin/sh
#
# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-04-19 GPL
#
# $Log: update_tmprsadh.sh,v $
# Revision 1.9  2018-04-01 15:15:10+05:30  Cprogrammer
# fixed wrong return value
#
# Revision 1.8  2018-03-25 15:13:42+05:30  Cprogrammer
# fix for system where ln doesn't have -r option
#
# Revision 1.7  2017-05-10 16:06:11+05:30  Cprogrammer
# create link to dh2048.pem for dhparams.pem
#
# Revision 1.6  2017-04-14 00:17:22+05:30  Cprogrammer
# added permissions for roundcube to accces certs, spamignore
#
# Revision 1.5  2017-03-29 19:11:10+05:30  Cprogrammer
# added rsa2048.pem, dh2048.pem creation
#
# Revision 1.4  2017-03-21 10:57:36+05:30  Cprogrammer
# updated certificate directory to sysconfdir/certs
#
# Revision 1.3  2017-03-09 14:37:37+05:30  Cprogrammer
# fixed controldir path
#
# Revision 1.2  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
# Revision 1.1  2010-05-18 17:42:03+05:30  Cprogrammer
# Initial revision
#
#

umask 0077 || exit 0
if [ -x /bin/chown ] ; then
	chown=/bin/chown
else
	chown=/usr/bin/chown
fi
if [ -x /bin/chmod ] ; then
	chmod=/bin/chmod
else
	chmod=/usr/bin/chmod
fi
if [ -x /bin/ln ] ; then
	ln=/bin/ln
else
	ln=/usr/bin/ln
fi

export PATH="$PATH:/usr/local/bin/ssl:/usr/sbin"
while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case "$1" in
    --certdir=*)
	CERTDIR=$optarg
	;;

    *)
	echo "invalid option [$1]"
	read key
	usage 1
	;;
    esac
    shift
done

if [ " $CERTDIR" = " " ] ; then
	CERTDIR=@sysconfdir@/certs
fi
slash=`echo $CERTDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd @sysconfdir@
fi
for i in 512 1024 2048
do
	/usr/bin/openssl genrsa -out $CERTDIR/rsa"$i".new $i &&
	$chmod 600 $CERTDIR/rsa"$i".new &&
	$chown indimail:qmail $CERTDIR/rsa"$i".new &&
	mv -f $CERTDIR/rsa"$i".new $CERTDIR/rsa"$i".pem
	echo rsa"$i".pem

	/usr/bin/openssl dhparam -2 -out $CERTDIR/dh"$i".new $i &&
	$chmod 600 $CERTDIR/dh"$i".new &&
	$chown indimail:qmail $CERTDIR/dh"$i".new &&
	mv -f $CERTDIR/dh"$i".new $CERTDIR/dh"$i".pem
	echo dh"$i".pem
done
if [ ! -f $CERTDIR/dhparams.pem ] ; then
	$ln -sr $CERTDIR/dh2048.pem $CERTDIR/dhparams.pem > /dev/null 2>&1
	if [ $? -ne 0 ] ; then
		cd $CERTDIR
		$ln -s dh2048.pem dhparams.pem
	fi
fi
