#
# $Log: maildirsmtp.sh,v $
# Revision 1.3  2017-03-09 16:38:46+05:30  Cprogrammer
# FHS changes
#
# Revision 1.2  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
# Revision 1.1  2004-05-14 00:43:55+05:30  Cprogrammer
# Initial revision
#
#

if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd SYSCONFDIR
fi
if [ -f "$CONTROLDIR"/queuelifetime ] ; then
	LIFETIME=`cat "$CONTROLDIR"/queuelifetime`
else
	LIFETIME=1209600
fi
exec \
PREFIX/bin/maildirserial -b -t $LIFETIME -- "$1" "$2" \
PREFIX/bin/tcpclient -RHl0 -- "$3" 25 \
PREFIX/bin/serialsmtp "$2" "$4"
