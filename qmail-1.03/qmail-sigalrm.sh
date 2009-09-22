# $Log: qmail-sigalrm.sh,v $
# Revision 1.1  2002-09-26 20:48:00+05:30  Cprogrammer
# Initial revision
#
# $Id: qmail-sigalrm.sh,v 1.1 2002-09-26 20:48:00+05:30 Cprogrammer Stab mbhangui $
if [ " $SERVICEDIR" = " " ] ; then
	SERVICEDIR=/service
	if [ -d /service1 ] ; then
		SERVICEDIR=/service1
	elif [ -d /service2 ] ; then
		SERVICEDIR=/service2
	fi
fi
exec QMAIL/bin/svc -a $SERVICEDIR/qmail-send*
