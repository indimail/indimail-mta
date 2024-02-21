#
# $Log: maildirsmtp.sh,v $
# Revision 1.5  2024-02-22 01:04:45+05:30  Cprogrammer
# replace cat with qmail-cat
#
# Revision 1.4  2023-12-08 12:56:22+05:30  Cprogrammer
# use PORT env variable if set as the SMTP port
#
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

[ -z "$CONTROLDIR" ] && CONTROLDIR=/etc/indimail/control
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
	cd SYSCONFDIR
	if [ $? -ne 0 ] ; then
		exit 111
	fi
fi
[ -f "$CONTROLDIR"/queuelifetime ] && LIFETIME=$(qmail-cat "$CONTROLDIR"/queuelifetime) || LIFETIME=1209600
[ -z "$LIFETIME" ] && LIFETIME=1209600
[ -z "$PORT" ] && PORT=25
exec \
PREFIX/bin/maildirserial -b -t $LIFETIME -- "$1" "$2" \
PREFIX/bin/tcpclient -RHl0 -- "$3" $PORT \
PREFIX/bin/serialsmtp "$2" "$4"
