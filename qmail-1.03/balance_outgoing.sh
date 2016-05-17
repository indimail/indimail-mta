#
# $Log: balance_outgoing.sh,v $
# Revision 1.3  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
# Revision 1.2  2011-07-03 16:54:23+05:30  Cprogrammer
# fixed typo and do chdir to /var/indimail
#
# Revision 1.1  2011-01-08 16:45:13+05:30  Cprogrammer
# Initial revision
#
#
# This scripts expects qmail-remote arguments on command line
# argv0          - qmail-remote
# argv1          - host   (host)
# argv2          - sender (sender)
# argv3          - qqeh   (qmail queue extra header)
# argv4          - size
# argv5 .. argvn - recipients
#
# $Id: balance_outgoing.sh,v 1.3 2016-05-17 23:11:42+05:30 Cprogrammer Exp mbhangui $
#
host=$1
sender=$2
qqeh=$3
size=$4
shift 4

cd QMAIL
if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd QMAIL
fi
FN=$CONTROLDIR/filterargs
if [ -n "$SPAMFILTER" -o -n "$FILTERARGS" -o -f $FN ] ; then
	# execute spawn-filter if you have filters defined for remote/local deliveries
	PROG="bin/spawn-filter"
else
	PROG="bin/qmail-remote"
fi
# Make an array of IP addresses in variable IP
if [ -f $CONTROLDIR/outgoingip.$host ] ; then
	IP=(`cat $CONTROLDIR/outgoingip.$host`)
elif [ -f $CONTROLDIR/outgoingip ] ; then
	IP=(`cat $CONTROLDIR/outgoingip`)
else
	exec -a qmail-remote $PROG "$host" "$sender" "$qqeh" $size $*
fi
IP_COUNT=${#IP[*]} # array size
if [ $IP_COUNT -gt 1 ] ; then
	i=`expr $RANDOM % $IP_COUNT` # chose an IP randomly
	export OUTGOINGIP=${IP[$i]}
fi
exec -a qmail-remote $PROG "$host" "$sender" "$qqeh" $size $*
