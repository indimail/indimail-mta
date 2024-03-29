#
# $Log: maildircmd.sh,v $
# Revision 1.5  2024-02-22 01:04:38+05:30  Cprogrammer
# replace cat with qcat
#
# Revision 1.4  2017-03-09 16:38:36+05:30  Cprogrammer
# FHS changes
#
# Revision 1.3  2016-05-17 23:11:42+05:30  Cprogrammer
# fix for configurable control directory
#
# Revision 1.2  2005-12-29 14:30:55+05:30  Cprogrammer
# made qmail's home configurable during compile time
#
# Revision 1.1  2004-05-14 00:43:52+05:30  Cprogrammer
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
[ -f "$CONTROLDIR"/queuelifetime ] && LIFETIME=$(qcat "$CONTROLDIR"/queuelifetime) || LIFETIME=1209600
[ -z "$LIFETIME" ] && LIFETIME=1209600
exec \
PREFIX/bin/maildirserial -b -t $LIFETIME -- "$1" "$2" \
PREFIX/bin/serialcmd "$1" "$3"
