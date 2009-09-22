#
# $Log: maildircmd.sh,v $
# Revision 1.2  2005-12-29 14:30:55+05:30  Cprogrammer
# made qmail's home configurable during compile time
#
# Revision 1.1  2004-05-14 00:43:52+05:30  Cprogrammer
# Initial revision
#
#
if [ " $CONTROLDIR" = " " ] ; then
	CONTROLDIR="control"
fi
if [ -f QMAIL/"$CONTROLDIR"/queuelifetime ] ; then
	LIFETIME=`cat QMAIL/"$CONTROLDIR"/queuelifetime`
else
	LIFETIME=1209600
fi
exec \
QMAIL/bin/maildirserial -b -t $LIFETIME -- "$1" "$2" \
QMAIL/bin/serialcmd "$1" "$3"
