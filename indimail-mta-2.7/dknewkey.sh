#
# $Log: dknewkey.sh,v $
# Revision 1.6  2019-02-15 17:11:18+05:30  Cprogrammer
# fixed checking of argument count
#
# Revision 1.5  2017-03-09 16:38:24+05:30  Cprogrammer
# use full path of openssl
#
# Revision 1.4  2010-05-16 19:59:48+05:30  Cprogrammer
# fix for Mac OS X
#
# Revision 1.3  2004-11-02 20:48:31+05:30  Cprogrammer
# fixed error when dknewkey was called without arguments
#
# Revision 1.2  2004-10-21 21:54:25+05:30  Cprogrammer
# create public key file
#
# Revision 1.1  2004-10-20 20:40:56+05:30  Cprogrammer
# Initial revision
#
#
if [ $# -ne 2 ] ; then
	echo "USAGE: dknewkey keyfile [bits]"
	exit 1
fi
BITS=384
if test -n "$2"; then BITS=$2; fi

/usr/bin/openssl genrsa -out $1 $BITS 2>/dev/null
/usr/bin/openssl rsa -in $1 -out /tmp/dknewkey.$$ -pubout -outform PEM 2>/dev/null
printf "%s._domainkey\tIN\tTXT\t\"k=rsa; p=%s\"\n" `basename $1` `grep -v ^-- /tmp/dknewkey.$$ | tr -d '\n'` > $1.pub
/bin/cat $1.pub
/bin/rm -f /tmp/dknewkey.$$
