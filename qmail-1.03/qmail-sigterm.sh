# $Log: qmail-sigterm.sh,v $
# Revision 1.1  2002-09-26 20:48:04+05:30  Cprogrammer
# Initial revision
#
# $Id: qmail-sigterm.sh,v 1.1 2002-09-26 20:48:04+05:30 Cprogrammer Stab mbhangui $
if [ " $SERVICEDIR" = " " ] ; then
	SERVICEDIR=/service
	if [ -d /service1 ] ; then
		SERVICEDIR=/service1
	elif [ -d /service2 ] ; then
		SERVICEDIR=/service2
	fi
fi
exec QMAIL/bin/svc -d $SERVICEDIR/qmail-send*
