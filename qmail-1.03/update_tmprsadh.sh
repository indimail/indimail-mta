#!/bin/sh
#
# Update temporary RSA and DH keys
# Frederik Vermeulen 2004-04-19 GPL
#
# $Log: update_tmprsadh.sh,v $
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

export PATH="$PATH:/usr/local/bin/ssl:/usr/sbin"
while test $# -gt 0; do
    case "$1" in
    -*=*) optarg=`echo "$1" | sed 's/[-_a-zA-Z0-9]*=//'` ;;
    *) optarg= ;;
    esac

    case "$1" in
    --controldir=*)
	CONTROLDIR=$optarg
	;;

    *)
	echo "invalid option [$1]"
	read key
	usage 1
	;;
    esac
    shift
done

if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd @sysconfdir@
fi
openssl genrsa -out $CONTROLDIR/rsa512.new 512 &&
chmod 600 $CONTROLDIR/rsa512.new &&
chown indimail:indimail $CONTROLDIR/rsa512.new &&
mv -f $CONTROLDIR/rsa512.new $CONTROLDIR/rsa512.pem
echo

openssl dhparam -2 -out $CONTROLDIR/dh512.new 512 &&
chmod 600 $CONTROLDIR/dh512.new &&
chown indimail:indimail $CONTROLDIR/dh512.new &&
mv -f $CONTROLDIR/dh512.new $CONTROLDIR/dh512.pem
echo

openssl dhparam -2 -out $CONTROLDIR/dh1024.new 1024 &&
chmod 600 $CONTROLDIR/dh1024.new &&
chown indimail:indimail $CONTROLDIR/dh1024.new &&
mv -f $CONTROLDIR/dh1024.new $CONTROLDIR/dh1024.pem
