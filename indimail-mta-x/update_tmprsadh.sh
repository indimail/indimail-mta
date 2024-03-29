#!/bin/sh
#
# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-04-19 GPL
#
# $Log: update_tmprsadh.sh,v $
# Revision 1.13  2023-02-14 09:27:09+05:30  Cprogrammer
# create rsa/dh parameter files with root:qcerts owner:group
#
# Revision 1.12  2022-05-18 13:30:52+05:30  Cprogrammer
# added --maxbits argument to specify maximum bits
#
# Revision 1.11  2021-09-11 19:03:47+05:30  Cprogrammer
# added read permissions for qmail group
#
# Revision 1.10  2020-09-17 11:16:08+05:30  Cprogrammer
# FreeBSD fixes
#
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
chown=$(which chown)
chmod=$(which chmod)
ln=$(which ln)
mv=$(which mv)

export PATH="$PATH:/usr/local/bin/ssl:/usr/sbin"
maxbits=2048
while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case "$1" in
    --certdir=*)
	CERTDIR=$optarg
	;;
	--maxbits=*)
	maxbits=$optarg
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
i=$maxbits
while true
do
	/usr/bin/openssl genrsa -out $CERTDIR/rsa"$i".new $i &&
	$chmod 640 $CERTDIR/rsa"$i".new &&
	$chown root:qcerts $CERTDIR/rsa"$i".new &&
	$mv -f $CERTDIR/rsa"$i".new $CERTDIR/rsa"$i".pem
	echo rsa"$i".pem

	/usr/bin/openssl dhparam -2 -out $CERTDIR/dh"$i".new $i &&
	$chmod 640 $CERTDIR/dh"$i".new &&
	$chown root:qcerts $CERTDIR/dh"$i".new &&
	$mv -f $CERTDIR/dh"$i".new $CERTDIR/dh"$i".pem
	echo dh"$i".pem
	i=$(expr $i / 2)
	if [ $i -lt 512 ] ; then
		break;
	fi
done
if [ ! -f $CERTDIR/dhparams.pem ] ; then
	echo $ln -sr $CERTDIR/dh2048.pem $CERTDIR/dhparams.pem
	$ln -sr $CERTDIR/dh2048.pem $CERTDIR/dhparams.pem 2>/dev/null
	if [ $? -ne 0 ] ; then
		cd $CERTDIR
		$ln -s dh2048.pem dhparams.pem
	fi
fi
