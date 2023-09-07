#
# $Log: surblqueue.sh,v $
# Revision 1.7  2023-09-07 19:58:19+05:30  Cprogrammer
# Redirect debug messages from surbfilter to SURBL_DEBUG_FN if set
#
# Revision 1.6  2017-04-16 13:08:01+05:30  Cprogrammer
# fixed qmail-multi path
#
# Revision 1.5  2017-03-09 16:39:29+05:30  Cprogrammer
# FHS changes
#
# Revision 1.4  2013-08-18 15:53:37+05:30  Cprogrammer
# use SURBQUEUE to execute qmail-queue program
#
# Revision 1.3  2013-08-12 18:56:37+05:30  Cprogrammer
# added error message if mktemp fails
#
# Revision 1.2  2013-08-12 15:00:54+05:30  Cprogrammer
# figure out mktemp path
#
# Revision 1.1  2011-07-15 11:48:51+05:30  Cprogrammer
# Initial revision
#
#
# $Id: surblqueue.sh,v 1.7 2023-09-07 19:58:19+05:30 Cprogrammer Exp mbhangui $
#
# I should be called by qmail-smtpd or anything that calls qmail-queue
#
if [ -f /bin/mktemp ] ; then
	MKTEMP=/bin/mktemp
elif [ -f /usr/bin/mktemp ] ; then
	MKTEMP=/usr/bin/mktemp
else
	MKTEMP=mktemp
fi
out=`$MKTEMP -t surblXXXXXXXXXX`
if [ $? -ne 0 ] ; then
	echo "mktemp: unable to create temp files" 1>&2
	exit 111
fi
#
# run surblfilter and feed output to qmail-multi
#
if [ -n "$ERROR_FD" ] ; then
	if [ "$ERROR_FD" != "2" ] ; then
		exec 2>&$ERROR_FD
	fi
fi
if [ -n "$SURBL_DEBUG_FN" ] ; then
	PREFIX/sbin/surblfilter $SURBLOPTS > $out 5>>$SURBL_DEBUG_FN
else
	PREFIX/sbin/surblfilter $SURBLOPTS > $out 5>/dev/null
fi
status=$?
if [ $status -eq 0 ] ; then
	exec 0<$out
	/bin/rm -f $out
	# use SURBLQUEUE to execute queue program (thanks Roberto Puzzanghera)
	if [ -n "$SURBLQUEUE" -a -x "$SURBLQUEUE" ]; then
		exec $SURBLQUEUE
	else
		exec PREFIX/sbin/qmail-multi
	fi
else
	/bin/rm -f $out
	exit $status
fi
