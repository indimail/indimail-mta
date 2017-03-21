#!/bin/sh
#
# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-04-19 GPL
#
# $Log: update_tmprsadh.sh,v $
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
	CHOWN=/bin/chown
else
	CHOWN=/usr/bin/chown
fi
if [ -x /bin/chmod ] ; then
	CHMOD=/bin/chmod
else
	CHMOD=/usr/bin/chmod
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
/usr/bin/openssl genrsa -out $CERTDIR/rsa512.new 512 &&
$chmod 600 $CERTDIR/rsa512.new &&
$chown indimail:indimail $CERTDIR/rsa512.new &&
mv -f $CERTDIR/rsa512.new $CERTDIR/rsa512.pem
echo rsa512.pem

/usr/bin/openssl dhparam -2 -out $CERTDIR/dh512.new 512 &&
$chmod 600 $CERTDIR/dh512.new &&
$chown indimail:indimail $CERTDIR/dh512.new &&
mv -f $CERTDIR/dh512.new $CERTDIR/dh512.pem
echo dh512.pem

/usr/bin/openssl dhparam -2 -out $CERTDIR/dh1024.new 1024 &&
$chmod 600 $CERTDIR/dh1024.new &&
$chown indimail:indimail $CERTDIR/dh1024.new &&
mv -f $CERTDIR/dh1024.new $CERTDIR/dh1024.pem
echo dh1024.pem
