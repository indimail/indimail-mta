# $Log: qmail-sighup.sh,v $
# Revision 1.4  2013-05-07 15:52:10+05:30  Cprogrammer
# suppress error for non-indimail system
#
# Revision 1.3  2009-04-19 13:39:59+05:30  Cprogrammer
# replaced indimail/bin/echo with echo 1>&2
#
# Revision 1.2  2003-12-21 15:21:42+05:30  Cprogrammer
# send SIGHUP to inlookup during domain addition/deletion
#
# Revision 1.1  2002-09-26 20:48:02+05:30  Cprogrammer
# Initial revision
#
# $Id: qmail-sighup.sh,v 1.4 2013-05-07 15:52:10+05:30 Cprogrammer Stab mbhangui $
if [ " $SERVICEDIR" = " " ] ; then
	SERVICEDIR=`/bin/ls /service*`
	if [ " $SERVICEDIR" = " " ] ; then
		echo "No Service enabled" 1>&2
		exit 1
	else
		QMAIL/bin/svc -h /service*/qmail-send* &&
		QMAIL/bin/svc -h /service*/inlookup* 2>/dev/null
	fi
else
	QMAIL/bin/svc -h $SERVICEDIR/qmail-send* &&
	QMAIL/bin/svc -h /service*/inlookup* 2>/dev/null
fi
