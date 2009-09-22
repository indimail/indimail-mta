#
# $Log: maildirsmtp.sh,v $
# Revision 1.1  2004-05-14 00:43:55+05:30  Cprogrammer
# Initial revision
#
#

if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR="control"
fi
if [ -f HOME/"$CONTROLDIR"/queuelifetime ] ; then
	LIFETIME=`cat HOME/"$CONTROLDIR"/queuelifetime`
else
	LIFETIME=1209600
fi
exec \
HOME/bin/maildirserial -b -t $LIFETIME -- "$1" "$2" \
HOME/bin/tcpclient -RHl0 -- "$3" 25 \
HOME/bin/serialsmtp "$2" "$4"
